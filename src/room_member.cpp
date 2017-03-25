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

void RoomMember::HandleWifiPackets(const RakNet::Packet* packet) {
    RakNet::BitStream stream(packet->data, packet->length, false);

    // Ignore the first byte, which is the message id.
    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    // Parse the WifiPacket from the BitStream
    uint8_t frame_type;
    stream.Read(frame_type);
    WifiPacket::PacketType type = static_cast<WifiPacket::PacketType>(frame_type);

    WifiPacket wifi_packet{};
    wifi_packet.type = type;
    stream.Read(wifi_packet.channel);
    stream.Read(wifi_packet.transmitter_address);
    stream.Read(wifi_packet.destination_address);

    uint32_t data_length;
    stream.Read(data_length);

    std::vector<uint8_t> data(data_length);
    stream.Read(reinterpret_cast<char*>(data.data()), data_length);

    wifi_packet.data = std::move(data);

    if (type == WifiPacket::PacketType::Beacon) {
        std::lock_guard<std::mutex> lock(beacon_mutex);
        beacon_queue.emplace_back(std::move(wifi_packet));

        // If we have more than the max number of buffered beacons, discard the oldest one
        if (beacon_queue.size() > MaxBeaconQueueSize) {
            beacon_queue.pop_front();
        }
    } else {
        // TODO(Subv): Add to ringbuffer for data / non-beacons
    }
}

std::deque<WifiPacket> RoomMember::PopWifiPackets(WifiPacket::PacketType type, const MacAddress& mac_address) {
    if (type == WifiPacket::PacketType::Beacon) {
        std::lock_guard<std::mutex> lock(beacon_mutex);
        // Clear the current beacon buffer and return the data.
        // TODO(Subv): Filter by sending mac address.
        return std::move(beacon_queue);
    } else {
        return {};
    }
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
        std::lock_guard<std::mutex> lock(network_mutex);

        RakNet::Packet* packet = nullptr;
        while (packet = peer->Receive()) {
            switch (packet->data[0]) {
            case ID_ROOM_CHAT:
                //FIXME
                break;
            case ID_ROOM_WIFI_PACKET:
                HandleWifiPackets(packet);
                break;
            case ID_ROOM_INFORMATION:
                //FIXME: Update playerlist from packet
                break;
            case ID_ROOM_NAME_COLLISION:
                state = State::NameCollision;
                peer->Shutdown(300);
                break;
            case ID_ROOM_MAC_COLLISION:
                state = State::MacCollision;
                peer->Shutdown(300);
                break;
            case ID_ROOM_JOIN_SUCCESS:
                // The join request was successful, we are now in the room.
                // If we joined successfully, there must be at least one client in the room: us.
                ASSERT_MSG(GetMemberInformation().size() > 0, "We have not yet received member information.");
                state = State::Joined;
                break;
            case ID_DISCONNECTION_NOTIFICATION:
                // Connection lost normally
                state = State::Idle;
                peer->Shutdown(300);
                break;
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                state = State::WrongVersion;
                peer->Shutdown(300);
                break;
            case ID_CONNECTION_ATTEMPT_FAILED:
                state = State::Error;
                peer->Shutdown(300);
                break;
            case ID_NO_FREE_INCOMING_CONNECTIONS:
                state = State::RoomFull;
                peer->Shutdown(300);
                break;
            case ID_CONNECTION_LOST:
                // Couldn't deliver a reliable packet, the other system was abnormally terminated
                state = State::LostConnection;
                peer->Shutdown(300);
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
    RakNet::SocketDescriptor socket(client_port,"");
    peer->Startup(1, &socket, 1);

    RakNet::ConnectionAttemptResult result = peer->Connect(server.c_str(), server_port, nullptr, 0);
    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        state = State::Error;
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
    }

    receive_thread->join();
    receive_thread.reset();
}
