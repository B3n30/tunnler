// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <mutex>
#include <thread>

#include "tunnler/assert.h"
#include "tunnler/bytestream.h"
#include "tunnler/tunnler.h"
#include "tunnler/room.h"
#include "tunnler/room_member.h"
#include "tunnler/room_message_types.h"


RoomMember::RoomMember() {
    client = enet_host_create(NULL,1,1,0,0);
}

RoomMember::~RoomMember() {
    ASSERT_MSG(!IsConnected(), "RoomMember is being destroyed while connected");
    if (receive_thread) {
        receive_thread->join();
    }
    enet_host_destroy(client);
}

void RoomMember::HandleChatPacket(const ENetEvent* event) {
    ByteStream stream(event->packet->data, event->packet->dataLength);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(MessageID));

    ChatEntry chat_entry{};
    std::string nickname;
    stream.Read(nickname);
    chat_entry.nickname.assign(nickname.c_str(), nickname.length());
    std::string message;
    stream.Read(message);
    chat_entry.message.assign(message.c_str(),message.length());

    {
        std::lock_guard<std::mutex> lock(chat_mutex);
        chat_queue.emplace_back(std::move(chat_entry));
    }
}

void RoomMember::HandleWifiPackets(const ENetEvent* event) {
    Invoke(EventType::OnMessagesReceived);

    WifiPacket wifi_packet{};
    
    auto EmplaceBackAndCheckSize = [&](std::deque<WifiPacket>& queue, size_t max_size) {
        queue.emplace_back(std::move(wifi_packet));
        // If we have more than the max number of buffered packets, discard the oldest one
        if (queue.size() > max_size) {
            queue.pop_front();
        }
    };

    ByteStream stream(event->packet->data, event->packet->dataLength);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(MessageID));

    // Parse the WifiPacket from the BitStream
    uint8_t frame_type;
    stream.Read(frame_type);
    WifiPacket::PacketType type = static_cast<WifiPacket::PacketType>(frame_type);

    wifi_packet.type = type;
    stream.Read(wifi_packet.channel);
    stream.Read(static_cast<unsigned char*>(wifi_packet.transmitter_address.data()), sizeof(MacAddress));
    stream.Read(static_cast<unsigned char*>(wifi_packet.destination_address.data()), sizeof(MacAddress));

    uint32_t data_length;
    stream.Read(data_length);

    std::vector<uint8_t> data(data_length);
    stream.Read(reinterpret_cast<unsigned char*>(data.data()), data_length);

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
        EmplaceBackAndCheckSize(management_queue, MaxManagementQueueSize);
        }
        break;
    }
    Invoke(EventType::OnFramesReceived);
}

void RoomMember::HandleRoomInformationPacket(const ENetEvent* event) {
    ByteStream stream(event->packet->data, event->packet->dataLength);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(MessageID));

    std::string room_name;
    stream.Read(room_name);
    room_information.name.assign(room_name.c_str(), room_name.length());
    stream.Read(room_information.member_slots);

    uint32_t num_members;
    stream.Read(num_members);
    member_information.resize(num_members);
    
    for (auto& member : member_information) {
        std::string nickname;
        stream.Read(nickname);
        member.nickname.assign(nickname.c_str(), nickname.length());

        stream.Read(member.mac_address.data(), sizeof(MacAddress));

        std::string game_name;
        stream.Read(game_name);
        member.game_name.assign(game_name.c_str(), game_name.length());
    }
    Invoke(EventType::OnRoomChanged);
}

void RoomMember::HandleJoinPacket(const ENetEvent* event) {
    ByteStream stream(event->packet->data, event->packet->dataLength);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(MessageID));

    //Parse the MAC Address from the BitStream
    stream.Read(mac_address.data(), sizeof(MacAddress));
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
    ByteStream stream;

    stream.Write(static_cast<MessageID>(ID_ROOM_CHAT));
    stream.Write(message);

    ENetPacket * packet = enet_packet_create(stream.GetData(), stream.Size(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
}

void RoomMember::SendWifiPacket(const WifiPacket& wifi_packet) {
    ByteStream stream;

    stream.Write(static_cast<MessageID>(ID_ROOM_WIFI_PACKET));
    stream.Write(static_cast<uint8_t>(wifi_packet.type));
    stream.Write(wifi_packet.channel);
    stream.Write(wifi_packet.transmitter_address.data(), sizeof(MacAddress));
    stream.Write(wifi_packet.destination_address.data(), sizeof(MacAddress));
    stream.Write(static_cast<uint32_t>(wifi_packet.data.size()));
    stream.Write((unsigned char*)wifi_packet.data.data(), wifi_packet.data.size());

    ENetPacket * packet = enet_packet_create(stream.GetData(), stream.Size(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
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
void RoomMember::SendJoinRequest(const std::string& nickname, const MacAddress& preferred_mac) {
    ByteStream stream;

    stream.Write(static_cast<MessageID>(ID_ROOM_JOIN_REQUEST));
    stream.Write(nickname);
    stream.Write(preferred_mac.data(), sizeof(MacAddress));

    ENetPacket * packet = enet_packet_create(stream.GetData(), stream.Size(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
}

void RoomMember::ReceiveLoop() {
    // Receive packets while the connection is open
    while (IsConnected()) {
        std::this_thread::sleep_for(sleep_time);
        std::lock_guard<std::mutex> lock(network_mutex);

        ENetEvent* event = nullptr;
        while (enet_host_service(client, event, 1000) > 0) {
            switch (event->type) {
            case ENET_EVENT_TYPE_CONNECT:
                SendJoinRequest(nickname);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                switch (event->packet->data[0]) {
                case ID_ROOM_CHAT:
                    HandleChatPacket(event);
                    break;
                case ID_ROOM_WIFI_PACKET:
                    HandleWifiPackets(event);
                    break;
                case ID_ROOM_INFORMATION:
                    HandleRoomInformationPacket(event);
                    break;
                case ID_ROOM_NAME_COLLISION:
                    state = State::NameCollision;
                    break;
                case ID_ROOM_MAC_COLLISION:
                    state = State::MacCollision;
                    break;
                case ID_ROOM_JOIN_SUCCESS:
                    // The join request was successful, we are now in the room.
                    // If we joined successfully, there must be at least one client in the room: us.
                    ASSERT_MSG(GetMemberInformation().size() > 0, "We have not yet received member information.");
                    HandleJoinPacket(event);    // Get the MAC Address for the client
                    state = State::Joined;
                    break;
                default:
                    break;
                }
                enet_packet_destroy (event->packet);
            case ENET_EVENT_TYPE_DISCONNECT:
                if(IsConnected())
                    state = State::LostConnection;
                return;
            }
        }
    }
};

void RoomMember::Join(const std::string& nickname, const std::string& server, uint16_t server_port, uint16_t client_port) {

    ENetAddress address;
    enet_address_set_host(&address, server.c_str());
    address.port = server_port;

    this->server = enet_host_connect(client, &address, 1, 0);

    if (this->server == nullptr) {
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
        enet_peer_disconnect(server, 0);
        state = State::Idle;
        Invoke(EventType::OnStateChanged);
    }

    receive_thread->join();
    receive_thread.reset();
    enet_peer_reset(server);
}
