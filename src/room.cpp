// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.


#include <algorithm>

#include "tunnler/assert.h"
#include "tunnler/room.h"
#include "tunnler/room_message_types.h"

#include "BitStream.h"
#include "RakSleep.h"

/// Maximum number of concurrent connections allowed to this room.
static const uint32_t MaxConcurrentConnections = 10;

void Room::Create(const std::string& name, const std::string& server_address, uint16_t server_port) {
    RakNet::SocketDescriptor socket(server_port, server_address.c_str());

    server = RakNet::RakPeerInterface::GetInstance();
    // TODO(Subv): Allow specifying the maximum number of concurrent connections.
    server->Startup(MaxConcurrentConnections, &socket, 1);
    server->SetMaximumIncomingConnections(MaxConcurrentConnections);

    state = State::Open;

    room_information.name = name;
    room_information.member_slots = MaxConcurrentConnections;

    // Start a network thread to Receive packets in a loop.
    room_thread = std::make_unique<std::thread>(&Room::ServerLoop, this);
}

void Room::Destroy() {
    state = State::Closed;
    room_thread->join();

    if (server) {
        server->Shutdown(300);
        RakNet::RakPeerInterface::DestroyInstance(server);
    }
    room_information = {};
    server = nullptr;
}

void Room::HandleJoinRequest(const RakNet::Packet* packet) {
    RakNet::BitStream stream(packet->data, packet->length, false);

    stream.IgnoreBytes(sizeof(RakNet::MessageID));

    RakNet::RakString nick;
    stream.Read(nick);

    MacAddress preferred_mac;
    stream.Read(preferred_mac);

    // Verify if the nick is already taken.
    const std::string nickname(nick.C_String(), nick.GetLength());
    if (!IsValidNickname(nickname)) {
        SendNameCollision(packet->systemAddress);
        return;
    }

    if (preferred_mac != NoPreferredMac) {
        // Verify if the preferred mac is available
        if (!IsValidMacAddress(preferred_mac)) {
            SendMacCollision(packet->systemAddress);
            return;
        }
    } else {
        // Assign a MAC address of this client automatically
        preferred_mac = GenerateMacAddress();
    }

    // At this point the client is ready to be added to the room.
    Member member{};
    member.mac_address = preferred_mac;
    member.nickname = nickname;
    member.network_address = packet->systemAddress;

    members.push_back(member);

    // Notify everyone that the room information has changed.
    BroadcastRoomInformation();

    SendJoinSuccess(member);
}

void Room::HandleChatPacket(const RakNet::Packet* packet) {
    RakNet::BitStream in_stream(packet->data, packet->length, false);

    in_stream.IgnoreBytes(sizeof(RakNet::MessageID));
    RakNet::RakString message;
    in_stream.Read(message);
    auto CompareNetworkAddress = [&](const Member member) -> bool {
        return member.network_address == packet->systemAddress;
    };
    const auto sending_member = std::find_if(members.begin(), members.end(), CompareNetworkAddress);

    ASSERT_MSG(sending_member == members.end(),"Received a chat message from a unknown sender");

    RakNet::RakString nickname = sending_member->nickname.c_str();
    RakNet::BitStream out_stream;
    
    out_stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_CHAT));
    out_stream.Write(nickname);
    out_stream.Write(message);
    server->Send(&out_stream, LOW_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Room::ServerLoop() {
    while (state != State::Closed) {
        std::this_thread::sleep_for(sleep_time);
        RakNet::Packet* packet = nullptr;
        while (packet = server->Receive()) {
            switch (packet->data[0]) {
            case ID_ROOM_JOIN_REQUEST:
                // Someone is trying to join the room.
                HandleJoinRequest(packet);
                break;
            case ID_DISCONNECTION_NOTIFICATION:
            case ID_CONNECTION_LOST:
                // A client has disconnected, remove them from the members list if they had joined the room.
                HandleClientDisconnection(packet->systemAddress);
                break;
            case ID_ROOM_WIFI_PACKET:
                // Received a wifi packet, broadcast it to everyone else except the sender.
                // TODO(Subv): Maybe change this to a loop over `members`, since we only want to
                // send this data to the people who have actually joined the room.
                server->Send(reinterpret_cast<char*>(packet->data), packet->length,
                             HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
                break;
            case ID_ROOM_CHAT:
                HandleChatPacket(packet);
                break;
            default:
                break;
            }

            server->DeallocatePacket(packet);
        }
    }
}

bool Room::IsValidNickname(const std::string& nickname) {
    // A nickname is valid if it is not already taken by anybody else in the room.
    // TODO(Subv): Check for spaces in the name.

    for (const Member& member : members) {
        if (member.nickname == nickname) {
            return false;
        }
    }

    return true;
}

bool Room::IsValidMacAddress(const MacAddress& address) {
    // A MAC address is valid if it is not already taken by anybody else in the room.

    for (const Member& member : members) {
        if (member.mac_address == address) {
            return false;
        }
    }

    return true;
}

void Room::SendNameCollision(RakNet::AddressOrGUID client) {
    RakNet::BitStream stream;
    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_NAME_COLLISION));

    server->Send(&stream, HIGH_PRIORITY, RELIABLE, 0, client, false);
}

void Room::SendMacCollision(RakNet::AddressOrGUID client) {
    RakNet::BitStream stream;
    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_MAC_COLLISION));

    server->Send(&stream, HIGH_PRIORITY, RELIABLE, 0, client, false);
}

void Room::HandleClientDisconnection(RakNet::AddressOrGUID client) {
    // Remove the client from the members list.
    members.erase(std::remove_if(members.begin(), members.end(), [&](const Member& member) {
        return member.network_address == client;
    }), members.end());

    // Announce the change to all other clients.
    BroadcastRoomInformation();
}

void Room::SendJoinSuccess(const Member& member) {
    RakNet::BitStream stream;
    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_JOIN_SUCCESS));
    stream.Write(member.mac_address);

    server->Send(&stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, member.network_address, false);
}

void Room::BroadcastRoomInformation() {
    RakNet::BitStream stream;
    stream.Write(static_cast<RakNet::MessageID>(ID_ROOM_INFORMATION));

    RakNet::RakString room_name = room_information.name.c_str();
    stream.Write(room_name);
    stream.Write(room_information.member_slots);

    stream.Write(static_cast<uint32_t>(members.size()));
    for (const auto& member: members) {
        RakNet::RakString nickname = member.nickname.c_str();
        RakNet::RakString game_name = member.game_name.c_str();
        stream.Write(nickname);
        stream.Write(member.mac_address);
        stream.Write(game_name);
    }

    server->Send(&stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

MacAddress Room::GenerateMacAddress() {
    MacAddress result_mac = NintendoOUI;
    std::uniform_int_distribution<> dis(0x00, 0xFF); //Random byte between 0 and 0xFF
    do {
        for (int i = 3; i < result_mac.size(); ++i) {
            result_mac[i] = dis(random_gen);
        }
    } while (!IsValidMacAddress(result_mac));
    return result_mac;
}
