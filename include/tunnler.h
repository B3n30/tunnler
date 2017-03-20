// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <array>
#include <vector>

using MacAddress = std::array<uint8_t, 6>;

/// Information about the received WiFi packets.
/// Acts as our own 802.11 header.
struct WifiPacket {
    enum class PacketType {
        Beacon,
        Data
    };
    PacketType type;                    ///< The type of 802.11 frame, Beacon / Data.
    std::vector<uint8_t> data;          ///< Raw 802.11 frame data, starting at the management frame header for management frames.
    MacAddress transmitter_address;     ///< Mac address of the transmitter.
    MacAddress destination_address;     ///< Mac address of the receiver.
    uint8_t channel;                    ///< WiFi channel where this frame was transmitted.
};

struct RoomInformation {
    std::string name;          // Name of the server
    unsigned int member_slots; // Maximum number of members in this room
};
