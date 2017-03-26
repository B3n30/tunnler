// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "tunnler/room.h"
#include "tunnler/tunnler.h"

#include "RakNetStatistics.h"
#include "RakPeerInterface.h"

// This is what a client [person joining a server] would use.
// It also has to be used if you host a game yourself (You'd create both, a Room and a RoomMembership for yourself)
class RoomMember final {
public:
    struct MemberInformation {
        std::string nickname;      // Nickname of the member.
        std::string game_name;     // Name of the game they're currently playing, or empty if they're not playing anything.
        MacAddress mac_address;    // MAC address associated with this member.
    };

    enum class State {
        Idle,     // Default state
        Error,    // Some error [permissions to network device missing or something]
        Joining,  // The client is attempting to join a room.
        Joined,   // The client is connected to the room and is ready to send/receive packets.

        // Reasons for connection loss
        LostConnection,

        // Reasons why connection was rejected
        RoomFull,        // The room is full and is not accepting any more connections.
        RoomDestroyed,   // Unknown reason, server not reachable or not responding for w/e reason
        NameCollision,   // Somebody is already using this name
        MacCollision,    // Somebody is already using that mac-address
        WrongVersion,    // The RakNet version does not match.
    };

    /// Represents a chat message.
    struct ChatEntry {
        std::string nickname;    ///< Nickname of the client who sent this message.
        std::string message;     ///< Body of the message.
    };

    using MemberList = std::vector<MemberInformation>;

    RoomMember();
    ~RoomMember();

    /**
     * Returns the status of our connection to the room.
     */
    State GetState() const { return state; };

    /**
     * Returns whether we're connected to a server or not.
     */
    bool IsConnected() const;

    /**
     * Returns information about the members in the room we're currently connected to.
     */
    const MemberList& GetMemberInformation() const { return member_information; }

    /**
     * Returns information about the room we're currently connected to.
     */
    RoomInformation GetRoomInformation() const { return room_information; };

    /**
     * Returns a list of received chat messages since the last call.
     */
    std::deque<ChatEntry> PopChatEntries();

    /**
     * Returns a list of received 802.11 frames from the specified sender
     * matching the type since the last call.
     */
    std::deque<WifiPacket> PopWifiPackets(WifiPacket::PacketType type, const MacAddress& sender = NoPreferredMac);

    /**
     * Sends a chat message to the room.
     * @param message The contents of the message.
     */
    void SendChatMessage(const std::string& message);

	/**
     * Sends a WiFi packet to the room.
     * @param packet The WiFi packet to send.
     */
    void SendWifiPacket(const WifiPacket& packet);

    /**
     * Returns a string with informations about the connection
     */
    std::string GetStatistics() const;

    /**
     * Returns the latest ping to the room
	 */
    int GetPing() const;

    /**
     * Attempts to join a room at the specified address and port, using the specified nickname.
     * This may fail if the username is already taken.
     */
    void Join(const std::string& nickname, const std::string& server = "127.0.0.1",
              const uint16_t serverPort = DefaultRoomPort, const uint16_t clientPort = 0);

    /**
     * Leaves the current room.
     */
    void Leave();

private:
    std::atomic<State> state; ///< Current state of the RoomMember.
    MemberList member_information; ///< Information about the clients connected to the same room as us.
    RoomInformation room_information; ///< Information about the room we're connected to.

    RakNet::RakPeerInterface* peer; ///< RakNet network interface.

    std::string nickname; ///< The nickname of this member.
    MacAddress mac_address; ///< The mac_address of this member.

    /**
     * Extracts a WifiPacket from a received RakNet packet and adds it to the proper queue.
     * @param packet The RakNet packet that was received.
     */
    void HandleWifiPackets(const RakNet::Packet* packet);

    /**
     * Extracts RoomInformation and MemberInformation from a received RakNet packet.
     * @param packet The RakNet packet that was received.
     */
    void HandleRoomInformationPacket(const RakNet::Packet* packet);

    /**
     * Extracts a MAC Address from a received RakNet packet.
     * @param packet The RakNet packet that was received.
     */
    void HandleJoinPacket(const RakNet::Packet* packet);

    /**
     * Closes the connection and sets the variables to a clean state for a new connection.
     * Must be called, when connection to server is closed.
     */
    void CloseConnection();

    std::unique_ptr<std::thread> receive_thread; ///< Thread that receives and dispatches network packets

    /// Max size of the data queue before the oldest entry is expunged.
    /// TODO(Ben): Find a good value for this
    static const size_t MaxDataQueueSize = 100;

    /// Max size of the beacon queue before the oldest entry is expunged.
    /// This number was empirically calculated, keeping too many frames will
    // cause an overflow in UDS::RecvBeaconBroadcastData.
    static const size_t MaxBeaconQueueSize = 25;

    std::deque<ChatEntry> chat_queue;    ///< List of all chat messages received since last PopChatEntries was called
    std::mutex data_mutex;             ///< Mutex to protect access to the data queue.
    std::deque<WifiPacket> data_queue;   ///< List of all received 802.11 frames with type `Data`
    std::mutex beacon_mutex;             ///< Mutex to protect access to the beacons queue.
    std::deque<WifiPacket> beacon_queue; ///< List of all received 802.11 frames with type `Beacon`

    RakNet::SystemAddress server_address; ///< Address of the server we're connected to.

    std::mutex network_mutex; ///< Mutex that controls access to the `peer` variable.

    /// Thread function that will receive and dispatch messages until connection is lost.
    void ReceiveLoop();
};
