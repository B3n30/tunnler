// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "MessageIdentifiers.h"

#include "RakPeerInterface.h"
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "GetTime.h"
#include "BitStream.h"

#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

//FIXME: Maybe seperate this into an interface and have LocalNetworkLobby and MasterServerLobby ?
class Lobby {

// Pointers to the interfaces of our server and client.
// Note we can easily have both in the same program
RakNet::RakPeerInterface *client = RakNet::RakPeerInterface::GetInstance();
bool b;
char str[256];
RakNet::TimeMS quit_time;
// Holds packets
RakNet::Packet* p;  

public:

struct RoomInformation {
  std::string name;
  //FIXME: Can we somehow extract a game title id from UDS or something?
  //FIXME: Can we somehow check the NWM cryptokey for validity?
  unsigned int users;
  unsigned int max_users;
};

struct FoundRoom {
  bool local; // Wether this was retrieved from a master list or LAN scan
  std::string server;
  unsigned int server_port;
    //FIXME: Store some timestamp when this was last alive, if it's too old it should probably be removed
  std::shared_ptr<RoomInformation> information;
  unsigned int ping;
};

private:

std::vector<FoundRoom> foundRooms;

public:

~Lobby();

std::string MasterServer = "http://127.0.0.1:8000/"; //FIXME: Make controllable
const std::string Path = "/rooms.php";

std::vector<FoundRoom> GetRooms();
// Returns a token
std::string CreateRoom(std::string server, uint16_t server_port);
bool UpdateRoom(std::string token);

bool DestroyRoom(std::string token);

// This might take some time to process
void sendPing();

void updateFoundRooms(unsigned int expiration_time = 5000);

RoomInformation GetRoomInformation(std::string server, uint16_t server_port, uint16_t client_port = 0);

//FIXME: Make the parameter some chrono:: crap
std::vector<FoundRoom> retrieveFoundRooms();

//FIXME: Use default server port here
void StartScan(unsigned int server_port = 1234, unsigned int client_port = 0);
void StopScan();

private:

//static void processInternetData(std::string masterServer);
void processLocalNetworkData();

};
