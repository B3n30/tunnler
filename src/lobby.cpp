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

#include "RakSleep.h"
#include "Gets.h"

//FIXME: Maybe seperate this into an interface and have LocalNetworkLobby and MasterServerLobby ?
class Lobby {

// Pointers to the interfaces of our server and client.
// Note we can easily have both in the same program
RakNet::RakPeerInterface *client = RakNet::RakPeerInterface::GetInstance();
bool b;
char str[256];
RakNet::TimeMS quitTime;
// Holds packets
RakNet::Packet* p;  

public:

struct RoomInformation {
  std::string name;
  //FIXME: Can we somehow extract a game title id from UDS or something?
  //FIXME: Can we somehow check the NWM cryptokey for validity?
  unsigned int users;
  unsigned int maxUsers;
};

struct FoundRoom {
  bool local; // Wether this was retrieved from a master list or LAN scan
  std::string server;
  unsigned int serverPort;
    //FIXME: Store some timestamp when this was last alive, if it's too old it should probably be removed
  RoomInformation information;
  unsigned int ping;
};

private:

std::vector<FoundRoom> foundRooms;

public:

~Lobby() {
    // We're done with the network
  if (client) {
    RakNet::RakPeerInterface::DestroyInstance(client);
  }
}

static void processLocalNetworkData() {
  // Loop for input
//  while (RakNet::GetTimeMS() < quitTime) {
  p = client->Receive();

  if (p == nullptr) {
    return;
  }

  switch(p->data[0]) {
  case ID_UNCONNECTED_PONG:
    RakNet::TimeMS time;
    RakNet::BitStream bsIn(p->data,p->length,false);
    bsIn.IgnoreBytes(1);
    bsIn.Read(time);
    printf("Got pong from %s with time %i\n", p->systemAddress.ToString(), RakNet::GetTimeMS() - time);
    break;
  case ID_UNCONNECTED_PING:
    printf("ID_UNCONNECTED_PING from %s\n",p->guid.ToString());
    break;
  case ID_UNCONNECTED_PING_OPEN_CONNECTIONS) {
    printf("ID_UNCONNECTED_PING_OPEN_CONNECTIONS from %s\n",p->guid.ToString());
    break;
  default:
    printf("Oops!\n");
    break;
  }
  client->DeallocatePacket(p);
}

// This might take some time to process
void sendPing() {
  //FIXME: Send a ping to all rooms
}

void updateFoundRooms(unsigned int expirationTime = 5000) {
  processLocalNetworkData();
  //FIXME: Remove dead servers = those who did not respond to ping in the last X milliseconds

  

}

//FIXME: Make the parameter some chrono:: crap
std::vector<FoundRoom> retrieveFoundRooms() {
  return foundRooms;
}

void scan(unsigned int serverPort = 1234, unsigned int clientPort = 0) {

  RakNet::SocketDescriptor socketDescriptor(clientPort, 0);
  socketDescriptor.socketFamily = AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
  client->Startup(1, &socketDescriptor, 1);

  // Connecting the client is very simple.  0 means we don't care about
  // a connectionValidationInteger, and false for low priority threads
  // All 255's mean broadcast
  client->Ping("255.255.255.255", serverPort, false);
  printf("Ping-ed\n");
}

};
