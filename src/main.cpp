//
//  main.cpp
//  tunnler
//
//
#include <stdio.h>
#include <deque>
#include "BitStream.h"
#include "messageIdentifiers.h"
#include "pcapHandle.hpp"
#include "RakPeerInterface.h"
#include "RakString.h"

#define SERVER_PORT 60000
#define MAX_CLIENTS 10
#define SERVER_IP "127.0.0.1"

const char* src_mac_address ="08:96:d7:e4:6a:9e";	//Fill in MAC of your device
const bool isServer = true;

enum GameMessages
{
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1
};

int main() {
	std::deque <u_char *>data_buffer;

	pcapHandle* pcap_handle = new pcapHandle();
	pcap_handle->activate();

	char filter_arg[100];
	sprintf(filter_arg, "ether src %s", src_mac_address );
	pcap_handle->setFilter(filter_arg);
	pcap_handle->setDumpFile("/tmp/capture.pcap");

	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();

	if(isServer) {
		RakNet::SocketDescriptor sd(SERVER_PORT,0);
		peer->Startup(MAX_CLIENTS, &sd, 1);
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	} else {
		RakNet::SocketDescriptor sd;
		peer->Startup(1,&sd, 1);
		peer->Connect(SERVER_IP, SERVER_PORT, 0,0);
	}

	printf("start tunnel\n");
	while(1) {
		if(int length = pcap_handle->next()) {
			u_char* b = reinterpret_cast<u_char*>(malloc(length));
			memcpy(b, pcap_handle->data, length);
			data_buffer.push_back(b);
		}
		for (RakNet::Packet* packet=peer->Receive(); packet; peer->DeallocatePacket(packet), packet=peer->Receive())
		{
			switch (packet->data[0])
				{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					printf("Another client has disconnected.\n");
					break;
				case ID_REMOTE_CONNECTION_LOST:
					printf("Another client has lost the connection.\n");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					printf("Another client has connected.\n");
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					{
						printf("Our connection request has been accepted.\n");
						if(data_buffer.size()) {
							RakNet::BitStream bsOut;
							bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
							bsOut.Write(data_buffer.front());
							peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
							data_buffer.pop_front();
						}
					}
					break;
				case ID_NEW_INCOMING_CONNECTION:
					printf("A connection is incoming.\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					if (isServer){
						printf("A client has disconnected.\n");
					} else {
						printf("We have been disconnected.\n");
					}
					break;
				case ID_CONNECTION_LOST:
					if (isServer){
						printf("A client lost the connection.\n");
					} else {
						printf("Connection lost.\n");
					}
					break;
				case ID_GAME_MESSAGE_1:
					{
						RakNet::RakString rs;
						RakNet::BitStream bsIn(packet->data,packet->length,false);
						bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
						bsIn.Read(rs);
						printf("%s\n", rs.C_String());
						pcap_handle->send(reinterpret_cast<const unsigned char*>(rs.C_String()), sizeof(rs.C_String())/sizeof(char));
						break;
					}
				default:
					printf("Message with identifier %i has arrived.\n", packet->data[0]);
					break;
				}
		}
	}
	RakNet::RakPeerInterface::DestroyInstance(peer);
	delete pcap_handle;
	printf("done tunnel\n");

}