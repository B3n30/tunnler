// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <string>
#include <thread>

#include "tunnler.h"

const uint16_t DefaultRoomPort = 1234;

// This is what a server [person creating a server] would use.
class Room {
public:
    enum class Status {
        Open,   // The room is open and ready to accept connections.
        Closed, // The room is not opened and can not accept connections.
    };

    Room();
    ~Room();

    /** 
     * Gets the current status of the room.
     */
    Status GetStatus() const { return status; };

    /** 
     * Gets the room information of the room.
     */
    RoomInformation GetRoomInformation() const { return room_information; };

    /** 
     * Creates the socket for this room. Returns true on success. Will bind to default address if server is empty string.
     */
    bool Create(const std::string& name, const std::string& server = "", uint16_t server_port = DefaultRoomPort);

    /**
     * Destroys the socket
     */
    void Destroy();
private:
    Status status;
    RoomInformation room_information;
    std::thread room_thread;

    void ServerLoop(); // Gets called as a seperate thread during create, and will loop until room is destroyed
};