#include "tunnler/lobby.h"

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

#include <cpr/cpr.h>

#if 0
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
//using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // As

//#include "restclient-cpp/connection.h"
//#include "restclient-cpp/restclient.h"

#endif

#include "json.hpp"

using json = nlohmann::json;

Lobby::~Lobby() {
    // We're done with the network
    if (client) {
        RakNet::RakPeerInterface::DestroyInstance(client);
    }
}

static void processInternetData(std::string masterServer) {
  //FIXME: do some rest requests
#if 0
  // initialize RestClient
  RestClient::init();

  // get a connection object
  RestClient::Connection* conn = new RestClient::Connection(std::string("http://") + masterServer);

  // configure basic auth
  //conn->SetBasicAuth("WarMachine68", "WARMACHINEROX");

  // set connection timeout to 5s
  conn->SetTimeout(5);

  // set custom user agent
  // (this will result in the UA "foo/cool restclient-cpp/VERSION")
  //conn->SetUserAgent("foo/cool");

  // enable following of redirects (default is off)
  conn->FollowRedirects(true);

  // set headers
  RestClient::HeaderFields headers;
  headers["Accept"] = "application/json";
  conn->SetHeaders(headers);

  // append additional headers
//  conn->AppendHeader("X-MY-HEADER", "foo")

  // if using a non-standard Certificate Authority (CA) trust file
//  conn->SetCAInfoFilePath("/etc/custom-ca.crt")
#endif

}

/*

So we have four endpoints. 
/room/
GET - Gets a list of available rooms. (What are you expected to be returned?)
POST - Creates a room. 
DELETE - Destroys a room.
PUT - A keep alive packet that keeps the room from automatically being destroyed.

*/

std::vector<Lobby::FoundRoom> Lobby::GetRooms() {
    auto url = cpr::Url{MasterServer + Path};
    auto response = cpr::Get(url);
#if 0
    std::cout << "Trying to parse results from request with status " << response.status_code << std::endl;
#endif

    if (response.status_code != 200) {
        return {};
    }

    std::vector<Lobby::FoundRoom> found_rooms;

    auto room_addresses_json = json::parse(response.text);
    for(auto room_address_json : room_addresses_json) {

        auto GetMandatoryKey = [&](const std::string& key, auto& value, json::value_t type) -> bool {
            auto element = room_address_json.find(key);
            if (element == room_address_json.end()) {
                return false;
            }
            if (element->type() != type) {
                return false;
            }
            value = room_address_json[key];
            return true;
        };

        Lobby::FoundRoom found_room;

        if (!GetMandatoryKey("server", found_room.server, json::value_t::string)) {
            continue;
        }
        if (!GetMandatoryKey("serverPort", found_room.server_port, json::value_t::number_unsigned)) {
            continue;
        }

        found_rooms.push_back(found_room);
    }
    return found_rooms;
}

// Returns a token
std::string Lobby::CreateRoom(std::string server, uint16_t server_port) {
    auto url = cpr::Url{MasterServer + Path};
    json body;
    body["server"] = server;
    body["serverPort"] = server_port;
    auto response = cpr::Post(url, cpr::Body(body.dump(4)));
    return response.text;
}

bool Lobby::UpdateRoom(std::string token) {
    auto url = cpr::Url{MasterServer + Path};
    json body;
    body["token"] = token;
    auto response = cpr::Put(url, cpr::Body(body.dump(4)));
    return response.status_code == 200;
}

bool Lobby::DestroyRoom(std::string token) {
    auto url = cpr::Url{MasterServer + Path};
    json body;
    body["token"] = token;
    auto response = cpr::Delete(url, cpr::Body(body.dump(4)));
    return response.status_code == 200;
}

void Lobby::processLocalNetworkData() {
  // Loop for input
//  while (RakNet::GetTimeMS() < quit_time) {

#if 1

  p = client->Receive();

  if (p == nullptr) {
    return;
  }

  switch(p->data[0]) {
  case ID_UNCONNECTED_PONG: {
    RakNet::TimeMS time;
    RakNet::BitStream bsIn(p->data,p->length,false);
    bsIn.IgnoreBytes(1);
    bsIn.Read(time);
    printf("Got pong from %s with time %i\n", p->systemAddress.ToString(), RakNet::GetTimeMS() - time);
    break;
  }
  case ID_UNCONNECTED_PING:
    printf("ID_UNCONNECTED_PING from %s\n",p->guid.ToString());
    break;
  case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
    printf("ID_UNCONNECTED_PING_OPEN_CONNECTIONS from %s\n",p->guid.ToString());
    break;
  default:
    printf("Oops!\n");
    break;
  }
  client->DeallocatePacket(p);

#endif
}

// This might take some time to process
Lobby::RoomInformation Lobby::GetRoomInformation(std::string server, uint16_t server_port, uint16_t client_port) {
    RakNet::SocketDescriptor socketDescriptor(client_port, 0);
    socketDescriptor.socketFamily = AF_INET; // Only IPV4 supports broadcast on 255.255.255.255

    //FIXME:Create a temporary socket or grab one from a pool?!

    client->Startup(1, &socketDescriptor, 1);
    //FIXME: Send a ping to room
    client->Ping(server.c_str(), server_port, false);

    // Wait for a response
    while(true) {
      //FIXME!!!!
      break;
    }

    return {};
}

void Lobby::updateFoundRooms(unsigned int expiration_time) {
  processLocalNetworkData();
  //FIXME: Remove dead servers = those who did not respond to ping in the last X milliseconds

  

}

//FIXME: Make the parameter some chrono:: crap
std::vector<Lobby::FoundRoom> Lobby::retrieveFoundRooms() {
  return foundRooms;
}

void Lobby::StartScan(unsigned int server_port, unsigned int client_port) {
  RakNet::SocketDescriptor socketDescriptor(client_port, 0);
  socketDescriptor.socketFamily = AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
  client->Startup(1, &socketDescriptor, 1);

  // Connecting the client is very simple.  0 means we don't care about
  // a connectionValidationInteger, and false for low priority threads
  // All 255's mean broadcast
  client->Ping("255.255.255.255", server_port, false);
  printf("Ping-ed\n");
}

void Lobby::StopScan() {
  printf("FIXME! Kill socket!\n");
}
