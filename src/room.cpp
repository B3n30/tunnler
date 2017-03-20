// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "tunnler/room.h"

#include "tunnler/room_message_types.h"

/// Maximum number of concurrent connections allowed to this room.
static const uint32_t MaxConcurrentConnections = 10;

void Room::Create(const std::string& name, const std::string& server_address, uint16_t server_port) {
    RakNet::SocketDescriptor socket(server_port, server_address.c_str());

    server = RakNet::RakPeerInterface::GetInstance();
    // TODO(Subv): Allow specifying the maximum number of concurrent connections.
    server->Startup(MaxConcurrentConnections, &socket, 1);
    server->SetMaximumIncomingConnections(MaxConcurrentConnections);

    state = State::Open;

    // Start a network thread to Receive packets in a loop.
    room_thread = std::make_unique<std::thread>(&Room::ServerLoop, this);
}

void Room::Destroy() {
    state = State::Closed;

    {
        std::lock_guard<std::mutex> lock(server_mutex);

        if (server)
            RakNet::RakPeerInterface::DestroyInstance(server);
        server = nullptr;
    }

    room_thread->join();
}

void Room::ServerLoop() {
    while (state != State::Closed) {
        std::lock_guard<std::mutex> lock(server_mutex);

        RakNet::Packet* packet = server->Receive();
        if (!packet)
            continue;

        switch (packet->data[0]) {
        case ID_ROOM_WIFI_PACKET:
            // Received a wifi packet, broadcast it to everyone else except the sender.
            server->Send(reinterpret_cast<char*>(packet->data), packet->length,
                         HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
            break;
        }

        server->DeallocatePacket(packet);
    }
}

