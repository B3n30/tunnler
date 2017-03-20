// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "tunnler/tunnler.h"

#include "RakPeerInterface.h"

const uint16_t DefaultRoomPort = 1234;

// This is what a server [person creating a server] would use.
class Room final {
public:
    enum class State {
        Open,   // The room is open and ready to accept connections.
        Closed, // The room is not opened and can not accept connections.
    };

    Room() { }
    ~Room() { }

    /**
     * Gets the current state of the room.
     */
    State GetState() const { return state; };

    /**
     * Gets the room information of the room.
     */
    RoomInformation GetRoomInformation() const { return room_information; };

    /**
     * Creates the socket for this room. Returns true on success. Will bind to default address if server is empty string.
     */
    void Create(const std::string& name, const std::string& server = "", uint16_t server_port = DefaultRoomPort);

    /**
     * Destroys the socket
     */
    void Destroy();
private:
    std::atomic<State> state; ///< Current state of the room.
    RoomInformation room_information; ///< Information about this room.
    std::unique_ptr<std::thread> room_thread; ///< Thread that receives and dispatches network packets

    RakNet::RakPeerInterface* server = nullptr; ///< RakNet network interface.

    std::mutex server_mutex; ///< Mutex that controls access to the `server` variable.

    /// Thread function that will receive and dispatch messages until the room is destroyed.
    void ServerLoop();
};