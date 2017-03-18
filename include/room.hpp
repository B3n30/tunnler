//
//  room.hpp
//  tunnler
//
//  Created by Benedikt Thomas on 18.03.17.
//
//

#ifndef room_hpp
#define room_hpp

#include <deque>
#include <string>
#include <vector>

#include "RakNetStatistics.h"
#include "RakPeerInterface.h"

typedef unsigned char uint8;

class RoomMembership {
	private:

	RakNet::RakNetStatistics *rss;

	// Pointers to the interfaces of our server and client.
	// Note we can easily have both in the same program
	RakNet::RakPeerInterface *client=RakNet::RakPeerInterface::GetInstance();


	// Holds packets
	RakNet::Packet* p;

	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;

	// Record the first client that connects to us so we can pass it to the ping function
	RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	static unsigned char GetPacketIdentifier(const RakNet::Packet *p);
	std::deque<std::string> chatQueue;
	std::deque<std::vector <unsigned char> > dataQueue;

	const char* u_name;

	public:

	RoomMembership();
	~RoomMembership();
	void join(std::string server = "127.0.0.1", unsigned int serverPort = 1234, unsigned int clientPort = 0);
	void chat(std::string message);
	void send(std::vector<unsigned char> data);
	void registerUsername(std::string username);
	void leave(unsigned int index = 0);
	bool handleInput();

	std::deque<std::string> getChat();
	std::deque<std::vector<unsigned char> > getData();
	void clearChat();
	void clearData();
	void update();
};

#endif /* room_hpp */
