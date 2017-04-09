// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <vector>
#include <string>

using MacAddress = std::array<uint8_t, 6>;

/// A special MAC address that tells the room we're joining to assign us a MAC address automatically.
const MacAddress NoPreferredMac = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/// This is the time the loops in Room::ServerLoop and RoomMember::ReceiveLoop sleep to keep the cpu usage low.
///  It should be as low as possible, to keep the response time low.
const std::chrono::milliseconds sleep_time(1);

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
    uint32_t member_slots; // Maximum number of members in this room
};
