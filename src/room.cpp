// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.


#include <algorithm>

#include "tunnler/packet.h"
#include "tunnler/assert.h"
#include "tunnler/room.h"
#include "tunnler/room_message_types.h"

/// Maximum number of concurrent connections allowed to this room.
static const uint32_t MaxConcurrentConnections = 10;

void Room::Create(const std::string& name, const std::string& server_address, uint16_t server_port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    enet_address_set_host(&address, server_address.c_str());
    address.port = server_port;

    server = enet_host_create(&address, MaxConcurrentConnections, 3, 0, 0);
    // TODO(Subv): Allow specifying the maximum number of concurrent connections.
    // TODO(B3N30): Specify the number of Channels
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
        enet_host_destroy(server);
    }
    room_information = {};
    server = nullptr;
}

void Room::HandleJoinRequest(const ENetEvent* event) {
    Packet packet;
    packet.append(event->packet->data, event->packet->dataLength);
    packet.IgnoreBytes(sizeof(MessageID));
    std::string nickname;
    packet >> nickname;

    MacAddress preferred_mac;
    packet >> preferred_mac;

    if (!IsValidNickname(nickname)) {
        SendNameCollision(event->peer);
        return;
    }

    if (preferred_mac != NoPreferredMac) {
        // Verify if the preferred mac is available
        if (!IsValidMacAddress(preferred_mac)) {
            SendMacCollision(event->peer);
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
    member.network_address = event->peer->address;

    members.push_back(member);

    // Notify everyone that the room information has changed.
    BroadcastRoomInformation();

    SendJoinSuccess(event->peer);
}

void Room::HandleChatPacket(const ENetEvent* event) {
    Packet inPacket;
    inPacket.append(event->packet->data, event->packet->dataLength);

    inPacket.IgnoreBytes(sizeof(MessageID));
    std::string message;
    inPacket >> message;
    auto CompareNetworkAddress = [&](const Member member) -> bool {
        return member.network_address.host == event->peer->address.host;
    };
    const auto sending_member = std::find_if(members.begin(), members.end(), CompareNetworkAddress);
    ASSERT_MSG(sending_member != members.end(), "Received a chat message from a unknown sender");

    Packet outPacket;
    outPacket << static_cast<MessageID>(ID_ROOM_CHAT);
    outPacket << sending_member->nickname;
    outPacket << message;

    ENetPacket* enetPacket = enet_packet_create(outPacket.getData(), outPacket.getDataSize(),  ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, enetPacket);
    enet_host_flush(server);
}

void Room::ServerLoop() {
    while (state != State::Closed) {
        ENetEvent event;
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                switch (event.packet->data[0]) {
                case ID_ROOM_WIFI_PACKET:
                    // Received a wifi packet, broadcast it to everyone else except the sender.
                    // TODO(Subv): Maybe change this to a loop over `members`, since we only want to
                    // send this data to the people who have actually joined the room.
                        enet_host_broadcast(server, 0, event.packet);
                        enet_host_flush(server);
                    break;
                case ID_ROOM_CHAT:
                    HandleChatPacket(&event);
                    break;
                case ID_ROOM_JOIN_REQUEST:
                    HandleJoinRequest(&event);
                    break;
                }
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                HandleClientDisconnection(event.peer);
                break;
            }
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

void Room::SendNameCollision(ENetPeer* client) {
    Packet packet;
    packet << static_cast<MessageID>(ID_ROOM_NAME_COLLISION);

    ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(client, 0, enetPacket);
    enet_host_flush(server);
    enet_peer_disconnect(client, 0);
}

void Room::SendMacCollision(ENetPeer* client) {
    Packet packet;
    packet << static_cast<MessageID>(ID_ROOM_MAC_COLLISION);

    ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(client, 0, enetPacket);
    enet_host_flush(server);
    ASSERT_MSG(false,"MAC");
    enet_peer_disconnect(client, 0);
}

void Room::HandleClientDisconnection(ENetPeer*  client) {
    // Remove the client from the members list.
    members.erase(std::remove_if(members.begin(), members.end(), [&](const Member& member) {
        return member.network_address.host == client->address.host;
    }), members.end());

    // Announce the change to all other clients.
    BroadcastRoomInformation();
}

void Room::SendJoinSuccess(ENetPeer* client) {
    Packet packet;
    packet << static_cast<MessageID>(ID_ROOM_JOIN_SUCCESS);
    packet << client->address.host;
    ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(),  ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(client, 0, enetPacket);
    enet_host_flush(server);
}

void Room::BroadcastRoomInformation() {
    Packet packet;
    packet << static_cast<MessageID>(ID_ROOM_INFORMATION);

    packet << room_information.name;
    packet << room_information.member_slots;

    packet << static_cast<uint32_t>(members.size());
    for (const auto& member: members) {
        packet << member.nickname;
        packet << member.mac_address;
        packet << member.game_name;
    }

    ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(),  ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, enetPacket);
    enet_host_flush(server);
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
