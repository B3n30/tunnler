// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "tunnler/room.h"
#include "tunnler/room_member.h"

/**
 * This will test the functions of the tunnler lib. 
 * It will create a room listening on 127.0.0.1:1234 and two members connecting to that room.
 * The output should look like:
 *
 * Created a test room
 *
 * Joining Member A
 * Successfully joined Member A
 * Member A 00:1f:32:7d:3a:3b:
 *
 * Joining Member B
 * Successfully joined Member B
 * Member A 00:1f:32:7d:3a:3b:
 * Member B 00:1f:32:18:21:27:
 * Test room(2/10)          *For now this will look like (2/0) since room_information.name and room_information.number_slots is never set*
 *
 * Testing beacon tunneling
 *
 * Member B received beacon:
 * Data: 1234567
 * There shouldn't be any more beacons now
 * Member A has 0 beacons
 * Member B has 0 beacons
 *
 * Testing data tunneling
 *
 * Member A received data:
 * Data: 1234567
 * There shouldn't be any more data frames now
 * Member A has 0 data frames
 * Member B has 0 data frames
 */

int main(int argc, char** argv) {
    //Test: creating room
    std::unique_ptr<Room> room = std::make_unique<Room>();
    room->Create("Test room");
    std::cout << "Created a test room" << std::endl << std::endl;

    //Test: Joining
    std::unique_ptr<RoomMember> member_a = std::make_unique<RoomMember>();
    std::cout << "Joining Member A" << std::endl;
    member_a->Join("Member A");

    RoomMember::State current_state;
    while ((current_state = member_a->GetState()) != RoomMember::State::Joined) {
        switch (current_state) {
        case RoomMember::State::Error:
            std::cout <<  "Unkown error while connecting Member A"<< std::endl;
            return 1;
        case RoomMember::State::LostConnection:
            std::cout <<  "Lost connection while connecting Member A"<< std::endl;
            return 1;
        case RoomMember::State::MacCollision:
            std::cout <<  "MAC collision of Member A"<< std::endl;
            return 1;
        case RoomMember::State::NameCollision:
            std::cout <<  "Name Collision of Member A"<< std::endl;
            return 1;
        case RoomMember::State::RoomFull:
            std::cout <<  "Room is to crowded"<< std::endl;
            return 1;
        case RoomMember::State::RoomDestroyed:
            std::cout <<  "Room is dead"<< std::endl;
            return 1;
        case RoomMember::State::WrongVersion:
            std::cout <<  "You got the wrong version"<< std::endl;
            return 1;
        case RoomMember::State::Idle:
            std::cout <<  "This should not happen!"<< std::endl;
            return 1;
        default:
            sleep(1);
            break;
        }
    }
    std::cout << "Successfully joined Member A" << std::endl;
    RoomMember::MemberList member_list = member_a->GetMemberInformation();
    for (const auto member:member_list) {
        std::ostringstream mac_address;
        for (const uint8_t& value: member.mac_address) {
            mac_address << std::hex << std::setw(2) << std::setfill('0') << +value << ":";
        }
        std::cout << member.nickname << " " << mac_address.str() << " " << member.game_name << std::endl;
    }


    std::unique_ptr<RoomMember> member_b = std::make_unique<RoomMember>();
    std::cout << std::endl << "Joining Member B" << std::endl;
    member_b->Join("Member B");             // Change the ncikname to "Member A" to test name collision

    while ((current_state = member_b->GetState()) != RoomMember::State::Joined) {
        switch (current_state) {
        case RoomMember::State::Error:
            std::cout <<  "Unkown error while connecting Member B"<< std::endl;
            return 1;
        case RoomMember::State::LostConnection:
            std::cout <<  "Lost connection while connecting Member B"<< std::endl;
            return 1;
        case RoomMember::State::MacCollision:
            std::cout <<  "MAC collision of Member B"<< std::endl;
            return 1;
        case RoomMember::State::NameCollision:
            std::cout <<  "Name collision of Member B"<< std::endl;
            return 1;
        case RoomMember::State::RoomFull:
            std::cout <<  "Room is to crowded"<< std::endl;
            return 1;
        case RoomMember::State::RoomDestroyed:
            std::cout <<  "Room is dead"<< std::endl;
            return 1;
        case RoomMember::State::WrongVersion:
            std::cout <<  "You got the wrong version"<< std::endl;
            return 1;
        case RoomMember::State::Idle:
            std::cout <<  "This should not happen!"<< std::endl;
            return 1;
        default:
            sleep(1);
            break;
        }
    }
    std::cout << "Successfully joined Member B" << std::endl;

    member_list = member_a->GetMemberInformation();
    for (const auto member:member_list) {
        std::ostringstream mac_address;
        for (const uint8_t& value: member.mac_address) {
            mac_address << std::hex << std::setw(2) << std::setfill('0') << +value << ":";
        }
        std::cout << member.nickname << " " << mac_address.str() << " " << member.game_name << std::endl;
    }

    RoomInformation room_information = member_a->GetRoomInformation();
    std::cout << room_information.name << "(" << member_list.size() << "/" << room_information.member_slots << ")" << std::endl;

/*   Chat isn't in master yet
    // Test: Chat
    std::cout << std::endl;
    std::cout << "Testing the chat" << std::endl << std::endl;

    member_a->SendChatMessage("This is a test message");
    std::deque<RoomMember::ChatEntry> chat_entries;

    std::cout << "This is what Member B receives:" << std::endl;
    while(chat_entries.size() == 0) {
        chat_entries = member_b->PopChatEntries();
        sleep(10);
    }
    for (const auto& entry: chat_entries) {
    std::cout << entry.nickname << ": " << entry.message << std::endl; ;
    }

    std::cout << "Member A should receive it, too:" << std::endl;
    while(chat_entries.size() == 0) {
        chat_entries = member_a->PopChatEntries();
        sleep(10);
    }
    for (const auto& entry: chat_entries) {
    std::cout << entry.nickname << ": " << entry.message << std::endl;
    }
*/

    //Test: Beacons
    std::cout << std::endl;
    std::cout << "Testing beacon tunneling" << std::endl << std::endl;
    
    WifiPacket beacon_packet{};
    beacon_packet.channel=11;
    beacon_packet.type=WifiPacket::PacketType::Beacon;
    beacon_packet.destination_address = NoPreferredMac;
    // packet.transmitter_address can't be set, because member->GetMacAddress is missing
    beacon_packet.data = std::vector<uint8_t> { 1, 2, 3, 4, 5, 6, 7};
    member_a->SendWifiPacket(beacon_packet);

    std::deque<WifiPacket> packets;
    while(packets.size() == 0)
        packets = member_b->PopWifiPackets(WifiPacket::PacketType::Beacon);
    for (const auto& received_packet: packets) {
        std::cout << "Member B received beacon:" <<std::endl << "Data: ";
        for (const auto& value: received_packet.data) {
            std::cout << +value;
        }
        std::cout << std::endl;
    }
    std::cout << "There shouldn't be any more beacons now" << std::endl;
    std::cout << "Member A has " << member_a->PopWifiPackets(WifiPacket::PacketType::Beacon).size() << " beacons" << std::endl;
    std::cout << "Member B has " << member_b->PopWifiPackets(WifiPacket::PacketType::Beacon).size() << " beacons" << std::endl;

    //Test: Data
    std::cout << std::endl;
    std::cout << "Testing data tunneling" << std::endl << std::endl;

    packets.clear();
    WifiPacket data_packet{};
    data_packet.channel=11;
    data_packet.type=WifiPacket::PacketType::Data;
    data_packet.destination_address = NoPreferredMac;
    // packet.transmitter_address can't be set, because member->GetMacAddress is missing
    data_packet.data = std::vector<uint8_t> { 1, 2, 3, 4, 5, 6, 7};
    member_b->SendWifiPacket(data_packet);

    while(packets.size() == 0)
        packets = member_a->PopWifiPackets(WifiPacket::PacketType::Data);
    for (const auto& received_packet: packets) {
        std::cout << "Member A received data:" <<std::endl << "Data: ";
        for (const auto& value: received_packet.data) {
            std::cout << +value;
        }
        std::cout << std::endl;
    }
    std::cout << "There shouldn't be any more data frames now" << std::endl;
    std::cout << "Member A has " << member_a->PopWifiPackets(WifiPacket::PacketType::Data).size() << " data frames" << std::endl;
    std::cout << "Member B has " << member_b->PopWifiPackets(WifiPacket::PacketType::Data).size() << " data frames" << std::endl;

    member_b->Leave();
    member_a->Leave();
    room->Destroy();
}