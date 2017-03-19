//
//  room.cpp
//  tunnler
//
//

#include "room.hpp"
#include "roomMesseges.hpp"

#include "MessageIdentifiers.h"


#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"
#include "GetTime.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "Kbhit.h"

#include "Gets.h"

//FIXME: RakNet AUR package seems to suck, this is a workaround
#undef LIBCAT_SECURITY
#define LIBCAT_SECURITY 0

#if LIBCAT_SECURITY==1
#include "SecureHandshake.h" // Include header for secure handshake
#endif

#include <iostream>
#include <vector>

unsigned char RoomMembership::GetPacketIdentifier(const RakNet::Packet *p) {
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

RoomMembership::RoomMembership() {
	client=RakNet::RakPeerInterface::GetInstance();
	clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;
}

RoomMembership::~RoomMembership() {
  //FIXME: Is this blocking?!
	// Be nice and let the server know we quit.
	client->Shutdown(300);

	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(client);
}

void RoomMembership::join(std::string server, unsigned int serverPort, unsigned int clientPort) {
  
	client->AllowConnectionResponseIPMigration(false);

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

void RoomMembership::chat(std::string message) {
	const char* data = message.c_str();
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)ID_ROOM_CHAT);
	bs.Write((unsigned short)strlen(data));
	bs.Write(data,strlen(data));
	client->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void RoomMembership::send(std::vector<unsigned char> data) {
	const char* c_data = reinterpret_cast<char*>(data.data());
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)ID_ROOM_DATA);
	bs.Write(data.size());
	bs.Write(c_data,data.size());
	client->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void RoomMembership::leave(unsigned int index) {
  client->CloseConnection(client->GetSystemAddressFromIndex(index),false);
  printf("Disconnecting.\n");
}

//FIXME: This is a temporary hack
bool RoomMembership::handleInput() {

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
	return false;

}

std::deque<std::string> RoomMembership::getChat() {
  return chatQueue;
}

std::deque<std::vector<unsigned char> > RoomMembership::getData() {
  return dataQueue;
}

void RoomMembership::clearChat() {
	chatQueue.clear();
}

void RoomMembership::clearData() {
	dataQueue.clear();
}


void RoomMembership::update() {

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
		  printf("ID_ALREADY_CONNECTED with guid %s\n",p->guid.ToString());		// Not threadsafe
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

	  case ID_ROOM_CHAT: {
			RakNet::RakString rs;
			RakNet::BitStream bsIn(p->data, p->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			chatQueue.push_back(rs.C_String());
			break;
		}
	  case ID_ROOM_DATA: {
		  std::vector<uint8> data_buffer(p->data+sizeof(RakNet::MessageID)+8, p->data+p->length);
		  dataQueue.push_back(data_buffer);
		  break;
	  }

	  default:
		  // It's a client, so just show the message
		  printf("%s\n", p->data);
		  break;
	  }
  }

}

class Room {

public:

Room() {
  std::cerr << "Nope! Server not implemented yet!" << std::endl;
  assert(false);
}

};