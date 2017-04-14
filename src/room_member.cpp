// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <mutex>
#include <thread>

#include "tunnler/tunnler.h"
#include "tunnler/room.h"
#include "tunnler/room_member.h"
#include "tunnler/room_message_types.h"

#include "tunnler/assert.h"

#include "BitStream.h"
#include "RakNetTypes.h"
#include "RakSleep.h"

RoomMember::RoomMember() {
    peer = RakNet::RakPeerInterface::GetInstance();
}

RoomMember::~RoomMember() {
    ASSERT_MSG(!IsConnected(), "RoomMember is being destroyed while connected");
    if (receive_thread) {
        receive_thread->join();
    }
    peer->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer);
}

void RoomMember::HandleChatPacket(const RakNet::Packet* packet) {
    RakNet::BitStream stream(packet->data, packet->length, false);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    ChatEntry chat_entry{};
    RakNet::RakString nickname;
    stream.Read(nickname);
    chat_entry.nickname.assign(nickname.C_String(), nickname.GetLength());
    RakNet::RakString message;
    stream.Read(message);
    chat_entry.message.assign(message.C_String(),message.GetLength());

    {
        std::lock_guard<std::mutex> lock(chat_mutex);
        chat_queue.emplace_back(std::move(chat_entry));
    }
    Invoke(EventType::OnMessagesReceived);
}

void RoomMember::HandleWifiPackets(const RakNet::Packet* packet) {
    WifiPacket wifi_packet{};
    
    auto EmplaceBackAndCheckSize = [&](std::deque<WifiPacket>& queue, size_t max_size) {
        queue.emplace_back(std::move(wifi_packet));
        // If we have more than the max number of buffered packets, discard the oldest one
        if (queue.size() > max_size) {
            queue.pop_front();
        }
    };

    RakNet::BitStream stream(packet->data, packet->length, false);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    // Parse the WifiPacket from the BitStream
    uint8_t frame_type;
    stream.Read(frame_type);
    WifiPacket::PacketType type = static_cast<WifiPacket::PacketType>(frame_type);

    wifi_packet.type = type;
    stream.Read(wifi_packet.channel);
    stream.Read(wifi_packet.transmitter_address);
    stream.Read(wifi_packet.destination_address);

    uint32_t data_length;
    stream.Read(data_length);

    std::vector<uint8_t> data(data_length);
    stream.Read(reinterpret_cast<char*>(data.data()), data_length);

    wifi_packet.data = std::move(data);

    switch (type) {
    case WifiPacket::PacketType::Beacon: {
        std::lock_guard<std::mutex> lock(beacon_mutex);
        EmplaceBackAndCheckSize(beacon_queue, MaxBeaconQueueSize);
        }
        break;
    case WifiPacket::PacketType::Data: {
        std::lock_guard<std::mutex> lock(data_mutex);
        EmplaceBackAndCheckSize(data_queue, MaxBeaconQueueSize);
        }
        break;
    case WifiPacket::PacketType::Management: {
        std::lock_guard<std::mutex> lock(management_mutex);
        EmplaceBackAndCheckSize(management_queue, MaxBeaconQueueSize);
        }
        break;
    }
    Invoke(EventType::OnFramesReceived);
}

void RoomMember::HandleRoomInformationPacket(const RakNet::Packet* packet) {
    RakNet::BitStream stream(packet->data, packet->length, false);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    RakNet::RakString room_name;
    stream.Read(room_name);
    room_information.name.assign(room_name.C_String(), room_name.GetLength());
    stream.Read(room_information.member_slots);

    uint32_t num_members;
    stream.Read(num_members);
    member_information.resize(num_members);
    
    for (auto& member : member_information) {
        RakNet::RakString nickname;
        stream.Read(nickname);
        member.nickname.assign(nickname.C_String(), nickname.GetLength());

        stream.Read(member.mac_address);

        RakNet::RakString game_name;
        stream.Read(game_name);
        member.game_name.assign(game_name.C_String(), game_name.GetLength());
    }
    Invoke(EventType::OnRoomChanged);
}

void RoomMember::HandleJoinPacket(const RakNet::Packet* packet) {
    RakNet::BitStream stream(packet->data, packet->length, false);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    //Parse the MAC Address from the BitStream
    stream.Read(mac_address);
}

std::deque<RoomMember::ChatEntry> RoomMember::PopChatEntries() {
    std::lock_guard<std::mutex> lock(chat_mutex);
    std::deque<ChatEntry> result_queue(std::move(chat_queue));
    chat_queue.clear();
    return result_queue;
}

std::deque<WifiPacket> RoomMember::PopWifiPackets(WifiPacket::PacketType type, const MacAddress& mac_address) {
    auto FilterAndPopPackets = [&](std::deque<WifiPacket>& source_queue) {
        std::deque<WifiPacket> result_queue;
        if (mac_address == NoPreferredMac) {         // Don't apply an address filter
            result_queue = std::move(source_queue);
            source_queue.clear();
        } else {                                    // Apply the address filter
            for (auto it = source_queue.begin(); it != source_queue.end();) {
                if (it->transmitter_address == mac_address) {
                    result_queue.emplace_back(std::move(*it));
                    it = source_queue.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return result_queue;
    };

    switch (type) {
    case WifiPacket::PacketType::Beacon: {
        std::lock_guard<std::mutex> lock(beacon_mutex);
        return FilterAndPopPackets(beacon_queue);
        }
    case WifiPacket::PacketType::Data: {
        std::lock_guard<std::mutex> lock(data_mutex);
        return FilterAndPopPackets(data_queue);
        }
    case WifiPacket::PacketType::Management: {
        std::lock_guard<std::mutex> lock(management_mutex);
        return FilterAndPopPackets(management_queue);
        }
    }
}

void RoomMember::SendChatMessage(const std::string& message) {
    RakNet::BitStream stream;

    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_CHAT));
    RakNet::RakString sendable_message = message.c_str();
    stream.Write(sendable_message);

    peer->Send(&stream, LOW_PRIORITY, RELIABLE_ORDERED, 0, server_address, false);
}

void RoomMember::SendWifiPacket(const WifiPacket& wifi_packet) {
    RakNet::BitStream stream;

    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_WIFI_PACKET));
    stream.Write(static_cast<uint8_t>(wifi_packet.type));
    stream.Write(wifi_packet.channel);
    stream.Write(wifi_packet.transmitter_address);
    stream.Write(wifi_packet.destination_address);
    stream.Write(static_cast<uint32_t>(wifi_packet.data.size()));
    stream.Write((char*)wifi_packet.data.data(), wifi_packet.data.size());

    peer->Send(&stream, HIGH_PRIORITY, RELIABLE, 0, server_address, false);
}

RoomMember::Connection RoomMember::Connect(std::function<void()> callback, EventType event_type) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    RoomMember::Connection connection;
    connection.event_type = event_type;
    connection.callback =  std::make_shared<std::function<void()> >(callback);
    callback_map[event_type].insert(connection.callback);
    return connection;
}

void RoomMember::Disconnect(Connection connection) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    callback_map[connection.event_type].erase(connection.callback);
}

void RoomMember::Invoke(EventType event_type)
{
    CallbackSet callbacks;
    {
        std::lock_guard<std::mutex> lock(callback_mutex);
        callbacks = callback_map[event_type];
        
    }
    for(auto const& callback: callbacks)
        (*callback)();
}

/**
 * Sends a request to the server, asking for permission to join a room with the specified nickname and preferred mac.
 * @params nickname The desired nickname.
 * @params preferred_mac The preferred MAC address to use in the room, an all-zero MAC address tells the server to assign one for us.
 */
static void SendJoinRequest(RakNet::RakPeerInterface* peer, const std::string& nickname, const MacAddress& preferred_mac = NoPreferredMac) {
    RakNet::BitStream stream;

    RakNet::RakString nick = nickname.c_str();
    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_JOIN_REQUEST));
    stream.Write(nick);
    stream.Write(preferred_mac);

    peer->Send(&stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void RoomMember::ReceiveLoop() {
    // Receive packets while the connection is open
    while (IsConnected()) {
        std::this_thread::sleep_for(sleep_time);
        std::lock_guard<std::mutex> lock(network_mutex);

        RakNet::Packet* packet = nullptr;
        while (packet = peer->Receive()) {
            switch (packet->data[0]) {
            case ID_ROOM_CHAT:
                HandleChatPacket(packet);
                break;
            case ID_ROOM_WIFI_PACKET:
                HandleWifiPackets(packet);
                break;
            case ID_ROOM_INFORMATION:
                HandleRoomInformationPacket(packet);
                break;
            case ID_ROOM_NAME_COLLISION:
                state = State::NameCollision;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_ROOM_MAC_COLLISION:
                state = State::MacCollision;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_ROOM_JOIN_SUCCESS:
                // The join request was successful, we are now in the room.
                // If we joined successfully, there must be at least one client in the room: us.
                ASSERT_MSG(GetMemberInformation().size() > 0, "We have not yet received member information.");
                HandleJoinPacket(packet);    // Get the MAC Address for the client
                state = State::Joined;
                Invoke(EventType::OnStateChanged);
                break;
            case ID_DISCONNECTION_NOTIFICATION:
                // Connection lost normally
                state = State::Idle;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                state = State::WrongVersion;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_CONNECTION_ATTEMPT_FAILED:
                state = State::Error;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_NO_FREE_INCOMING_CONNECTIONS:
                state = State::RoomFull;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_CONNECTION_LOST:
                // Couldn't deliver a reliable packet, the other system was abnormally terminated
                state = State::LostConnection;
                peer->Shutdown(300);
                Invoke(EventType::OnStateChanged);
                break;
            case ID_CONNECTION_REQUEST_ACCEPTED:
                // Update the server address with the address of the sender of this packet.
                server_address = packet->systemAddress;
                SendJoinRequest(peer, nickname);
                break;
            default:
                break;
            }

            peer->DeallocatePacket(packet);
        }
    }
};

void RoomMember::Join(const std::string& nickname, const std::string& server, uint16_t server_port, uint16_t client_port) {
    RakNet::SocketDescriptor socket(client_port, 0);
    peer->Startup(1, &socket, 1);

    RakNet::ConnectionAttemptResult result = peer->Connect(server.c_str(), server_port, nullptr, 0);
    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        state = State::Error;
        Invoke(EventType::OnStateChanged);
        return;
    }

    this->nickname = nickname;
    state = State::Joining;

    // Start a network thread to Receive packets in a loop.
    receive_thread = std::make_unique<std::thread>(&RoomMember::ReceiveLoop, this);
}

bool RoomMember::IsConnected() const {
    return state == State::Joining || state == State::Joined;
}

void RoomMember::Leave() {
    ASSERT_MSG(receive_thread != nullptr, "Must be in a room to leave it.");

    {
        std::lock_guard<std::mutex> lock(network_mutex);
        peer->Shutdown(300);
        state = State::Idle;
        Invoke(EventType::OnStateChanged);
    }

    receive_thread->join();
    receive_thread.reset();
}
