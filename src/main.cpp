//
//  main.cpp
//  tunnler
//
//

#include <deque>
#include <memory>
#include <cstdio>
#include <iostream>

#include "pcapHandle.hpp"
#include "RakPeerInterface.h"
#include "RakString.h"

#include <stdio.h>
#include <unistd.h>

#include "room.h"

const char* src_mac_address ="08:96:d7:e4:6a:9e";	//Fill in MAC of your device
const bool isServer = true;

int main(int argc, char* argv[]) {
	std::deque<u_char*> data_buffer;

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <device> <server> <server-port>" << std::endl;
    return 1;
  }

  std::string devName = std::string(argv[1]);
  std::string server = std::string(argv[2]);
  unsigned int serverPort = atoi(argv[3]);


  std::shared_ptr<RoomMembership> room = std::make_shared<RoomMembership>();
  room->join(server, serverPort);

  while(true) {
    // This sleep keeps RakNet responsive
#ifdef _WIN32
    Sleep(30);
#else
    usleep(30 * 1000);
#endif

    room->handleInput();
    room->update();
  }

  //FIXME: Use wifi device name from devName

	pcapHandle* pcap_handle = new pcapHandle();
	pcap_handle->activate();

	char filter_arg[100];
	sprintf(filter_arg, "ether src %s", src_mac_address );
	pcap_handle->setFilter(filter_arg);
	pcap_handle->setDumpFile("/tmp/capture.pcap");

	std::cout << "start tunnel" << std::endl;
	while(1) {

    // Do connection management
    room->update();

    for(auto& p : room->getData()) {
      std::cout << "Received data!" << std::endl;
    }

    for(auto& p : room->getChat()) {
      std::cout << "Received chat!" << std::endl;
    }

		if(int length = pcap_handle->next()) {
      std::vector<uint8> data_buffer(pcap_handle->data, pcap_handle->data + length);
      std::cout << "Forwarding data!" << std::endl;
      room->send(data_buffer);
		}
	}

  room.reset();
	delete pcap_handle;
	std::cout << "done tunnel" << std::endl;
}
