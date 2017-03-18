/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  Copyright (c) 2017, Jannik Vogel
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the RakNet
 *  LICENSE file in the root directory of the RakNet source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// Based on:

// ----------------------------------------------------------------------
// RakNet version 1.0
// Filename ChatExample.cpp
// Very basic chat engine example
// ----------------------------------------------------------------------

#include "MessageIdentifiers.h"

#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "RakNetTypes.h"
#ifdef _WIN32
#include "Kbhit.h"
#include "WindowsIncludes.h" // Sleep
#else
#include "Kbhit.h"
#include <unistd.h> // usleep
#endif
#include "Gets.h"

//FIXME: RakNet AUR package seems to suck, this is a workaround
#undef LIBCAT_SECURITY
#define LIBCAT_SECURITY 0

#if LIBCAT_SECURITY==1
#include "SecureHandshake.h" // Include header for secure handshake
#endif

#include <iostream>
#include <vector>

enum RoomMessages
{
	ID_ROOM_CHAT = ID_USER_PACKET_ENUM + 1,
	ID_ROOM_DATA
};

class RoomMembership {

private:

RakNet::RakNetStatistics *rss;
// Pointers to the interfaces of our server and client.
// Note we can easily have both in the same program
RakNet::RakPeerInterface *client=RakNet::RakPeerInterface::GetInstance();
//	client->InitializeSecurity(0,0,0,0);
//RakNet::PacketLogger packetLogger;
//client->AttachPlugin(&packetLogger);


// Holds packets
RakNet::Packet* p;

// GetPacketIdentifier returns this
unsigned char packetIdentifier;

// Record the first client that connects to us so we can pass it to the ping function
RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;

// Crude interface

// Copied from RakNet Multiplayer.cpp
// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
static unsigned char GetPacketIdentifier(const RakNet::Packet *p) {
  if (p == 0) {
	  return 255;
  }

  if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
	  RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
	  return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
  } else {
	  return (unsigned char) p->data[0];
  }
}



public:

~RoomMembership() {
  //FIXME: Is this blocking?!
	// Be nice and let the server know we quit.
	client->Shutdown(300);

	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(client);
}

void join(std::string server = "127.0.0.1", unsigned int serverPort = 1234, unsigned int clientPort = 0) {
  
	client->AllowConnectionResponseIPMigration(false);

	// Connecting the client is very simple.  0 means we don't care about
	// a connectionValidationInteger, and false for low priority threads
	RakNet::SocketDescriptor socketDescriptor(clientPort, 0);
	socketDescriptor.socketFamily=AF_INET;
	client->Startup(8,&socketDescriptor, 1);
	client->SetOccasionalPing(true);

#if LIBCAT_SECURITY==1
	char public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
	FILE *fp = fopen("publicKey.dat", "rb");
	fread(public_key,sizeof(public_key),1,fp);
	fclose(fp);
#endif

#if LIBCAT_SECURITY==1
	RakNet::PublicKey pk;
	pk.remoteServerPublicKey=public_key;
	pk.publicKeyMode=RakNet::PKM_USE_KNOWN_PUBLIC_KEY;
	bool b = client->Connect(server.c_str(), serverPort, "Rumpelstiltskin", (int) strlen("Rumpelstiltskin"), &pk)==RakNet::CONNECTION_ATTEMPT_STARTED;	
#else
	RakNet::ConnectionAttemptResult car = client->Connect(server.c_str(), serverPort, "Rumpelstiltskin", (int) strlen("Rumpelstiltskin"));
	RakAssert(car==RakNet::CONNECTION_ATTEMPT_STARTED);
#endif

	printf("\nMy IP addresses:\n");
	unsigned int i;
	for (i=0; i < client->GetNumberOfAddresses(); i++) {
		printf("%i. %s\n", i+1, client->GetLocalIP(i));
	}

	printf("My GUID is %s\n", client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
	puts("'quit' to quit. 'stat' to show stats. 'ping' to ping.\n'disconnect' to disconnect. 'connect' to reconnnect. Type to talk.");
}

void chat(std::string message) {
  // message is the data to send
  // strlen(message)+1 is to send the null terminator
  // HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
  // RELIABLE_ORDERED means make sure the message arrives in the right order
  const char* data = message.c_str();
  client->Send(data, (int) strlen(data) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void send(std::vector<uint8> data) {
  // message is the data to send
  // strlen(message)+1 is to send the null terminator
  // HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
  // RELIABLE_ORDERED means make sure the message arrives in the right order
//  client->Send(message, (int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void leave(unsigned int index = 0) {
  client->CloseConnection(client->GetSystemAddressFromIndex(index),false);
  printf("Disconnecting.\n");
}

//FIXME: This is a temporary hack
bool handleInput() {

  if (kbhit()) {
	  // Notice what is not here: something to keep our network running.  It's
	  // fine to block on Gets or anything we want
	  // Because the network engine was painstakingly written using threads.
    char message[2048];
	  Gets(message,sizeof(message));

	  if (strcmp(message, "quit")==0) {
		  puts("Quitting.");
      return true;
	  }

	  if (strcmp(message, "stat")==0) {
		
		  rss=client->GetStatistics(client->GetSystemAddressFromIndex(0));
		  StatisticsToString(rss, message, 2);
		  printf("%s", message);
		  printf("Ping=%i\n", client->GetAveragePing(client->GetSystemAddressFromIndex(0)));
	
		  return false;
	  }

	  if (strcmp(message, "disconnect")==0) {
      printf("Enter index to disconnect: ");
      char str[32];
      Gets(str, sizeof(str));
      if (str[0]==0) {
	      strcpy(str,"0");
      }
      int index = atoi(str);

      leave(index);
      return false;
	  }

	  if (strcmp(message, "shutdown")==0) {
		  client->Shutdown(100);
		  printf("Shutdown.\n");
		  return false;
	  }

#if 0
	  if (strcmp(message, "startup")==0) {
		  bool b = client->Startup(8,&socketDescriptor, 1)==RakNet::RAKNET_STARTED;
		  if (b) {
			  printf("Started.\n");
		  } else {
			  printf("Startup failed.\n");
      }
		  return false;
	  }
#endif

	  if (strcmp(message, "ping")==0) {
		  if (client->GetSystemAddressFromIndex(0)!=RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
			  client->Ping(client->GetSystemAddressFromIndex(0));
      }
		  return false;
	  }

	  if (strcmp(message, "getlastping")==0) {
		  if (client->GetSystemAddressFromIndex(0)!=RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
			  printf("Last ping is %i\n", client->GetLastPing(client->GetSystemAddressFromIndex(0)));
      }
		  return false;
	  }

    chat(message);

  }

}

std::deque<std::string> getChat() {
  return {};
}

std::deque<std::vector<uint8>> getData() {
  return {};
}

void update() {

  // Get a packet from either the server or the client

  for (p = client->Receive(); p; client->DeallocatePacket(p), p=client->Receive()) {
	  // We got a packet, get the identifier with our handy function
	  packetIdentifier = GetPacketIdentifier(p);

	  // Check if this is a network message packet
	  switch (packetIdentifier) {
	  case ID_DISCONNECTION_NOTIFICATION:
		  // Connection lost normally
		  printf("ID_DISCONNECTION_NOTIFICATION\n");
		  break;
	  case ID_ALREADY_CONNECTED:
		  // Connection lost normally
		  printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid);
		  break;
	  case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		  printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
		  break;
	  case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		  printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
		  break;
	  case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		  printf("ID_REMOTE_CONNECTION_LOST\n");
		  break;
	  case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		  printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
		  break;
	  case ID_CONNECTION_BANNED: // Banned from this server
		  printf("We are banned from this server.\n");
		  break;			
	  case ID_CONNECTION_ATTEMPT_FAILED:
		  printf("Connection attempt failed\n");
		  break;
	  case ID_NO_FREE_INCOMING_CONNECTIONS:
		  // Sorry, the server is full.  I don't do anything here but
		  // A real app should tell the user
		  printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
		  break;

	  case ID_INVALID_PASSWORD:
		  printf("ID_INVALID_PASSWORD\n");
		  break;

	  case ID_CONNECTION_LOST:
		  // Couldn't deliver a reliable packet - i.e. the other system was abnormally
		  // terminated
		  printf("ID_CONNECTION_LOST\n");
		  break;

	  case ID_CONNECTION_REQUEST_ACCEPTED:
		  // This tells the client they have connected
		  printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
		  printf("My external address is %s\n", client->GetExternalID(p->systemAddress).ToString(true));
		  break;
	  case ID_CONNECTED_PING:
	  case ID_UNCONNECTED_PING:
		  printf("Ping from %s\n", p->systemAddress.ToString(true));
		  break;

	  case ID_ROOM_CHAT:
      //FIXME: Add data to a queue
      printf("Got chat message\n");
      break;

	  case ID_ROOM_DATA: {
		  RakNet::RakString rs;
		  RakNet::BitStream bsIn(p->data, p->length, false);
		  bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
		  bsIn.Read(rs);
		  printf("Got data message: %s\n", rs.C_String());
      //FIXME: Add package to a queue
#if 0
		  pcap_handle->send(reinterpret_cast<const unsigned char*>(rs.C_String()), sizeof(rs.C_String())/sizeof(char));
#endif
		  break;
	  }

	  default:
		  // It's a client, so just show the message
		  printf("%s\n", p->data);
		  break;
	  }
  }

}

};

class Room {

public:

Room() {
  std::cerr << "Nope! Server not implemented yet!" << std::endl;
  assert(false);
}

};
