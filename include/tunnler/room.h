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
#include <vector>

#include "tunnler/tunnler.h"

#include "RakPeerInterface.h"

const uint16_t DefaultRoomPort = 1234;

// This MAC address is used to generate a 'Nintendo' like Mac address.
const MacAddress NintendoOUI = { 0x00, 0x1F, 0x32, 0x00, 0x00, 0x00 };

// This is what a server [person creating a server] would use.
class Room final {
public:
    enum class State {
        Open,   // The room is open and ready to accept connections.
        Closed, // The room is not opened and can not accept connections.
    };

    struct Member {
        std::string nickname; ///< The nickname of the member.
        std::string game_name; //< The current game of the member
        MacAddress mac_address; ///< The assigned mac address of the member.
        RakNet::AddressOrGUID network_address; ///< The network address of the remote peer.
    };

    using MemberList = std::vector<Member>;

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
    MemberList members; ///< Information about the members of this room.
    std::unique_ptr<std::thread> room_thread; ///< Thread that receives and dispatches network packets

    RakNet::RakPeerInterface* server = nullptr; ///< RakNet network interface.

    /// Thread function that will receive and dispatch messages until the room is destroyed.
    void ServerLoop();

    /**
     * Parses and answers a room join request from a client.
     * Validates the uniqueness of the username and assigns the MAC address
     * that the client will use for the remainder of the connection.
     */
    void HandleJoinRequest(const RakNet::Packet* packet);

    /**
     * Sends the information about the room, along with the list of members
     * to every connected client in the room.
     * The packet has the structure:
     * <MessageID>ID_ROOM_INFORMATION
     * <RakString> room_name
     * <uint32_t> member_slots: The max number of clients allowed in this room
     * <uint32_t> num_members: the number of currently joined clients
     * This is followed by the following three values for each member:
     * <RakString> nickname of that member
     * <MacAddress> mac_address of that member
     * <RakString> game_name of that member
     */
    void BroadcastRoomInformation();

    /**
     * Returns whether the nickname is valid, ie. isn't already taken by someone else in the room.
     */
    bool IsValidNickname(const std::string& nickname);

    /**
     * Returns whether the MAC address is valid, ie. isn't already taken by someone else in the room.
     */
    bool IsValidMacAddress(const MacAddress& address);

    /**
     * Generates a free MAC address to assign to a new client.
     * The first 3 bytes are the NintendoOUI 0x00, 0x1F, 0x32
     */
    MacAddress GenerateMacAddress();

    /**
     * Sends a ID_ROOM_NAME_COLLISION message telling the client that the name is invalid.
     */
    void SendNameCollision(RakNet::AddressOrGUID client);

    /**
     * Sends a ID_ROOM_MAC_COLLISION message telling the client that the MAC is invalid.
     */
    void SendMacCollision(RakNet::AddressOrGUID client);

    /**
     * Removes the client from the members list if it was in it and announces the change
     * to all other clients.
     */
    void HandleClientDisconnection(RakNet::AddressOrGUID client);

    /**
     * Notifies the member that its connection attempt was successful,
     * and it is now part of the room.
     */
    void SendJoinSuccess(const Member& member);
};