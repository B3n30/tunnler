//
//  main.cpp
//  tunnler
//
//
#include <deque>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "pcapHandle.hpp"
#include "room.hpp"

const char* src_mac_address ="08:96:d7:e4:6a:9e";	//Fill in MAC of your device


int main(int argc, char* argv[]) {
	std::deque <u_char *>data_buffer;

	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <device> <server> <server-port>" << std::endl;
 		return 1;
 	}

	std::string deviceName = std::string(argv[1]);
	std::string server = std::string(argv[2]);
	unsigned int serverPort = atoi(argv[3]);

	pcapHandle* pcap_handle = new pcapHandle(deviceName);
	pcap_handle->activate();

	char filter_arg[100];
	sprintf(filter_arg, "ether src %s", src_mac_address );
	pcap_handle->setFilter(filter_arg);

	std::shared_ptr<RoomMembership> room = std::make_shared<RoomMembership>();
	room->join(server, serverPort);

	printf("start digging a tunnel\n");
	while(true) {
		if(int length = pcap_handle->next()) {
			std::vector<uint8> data_buffer(pcap_handle->data, pcap_handle->data + length);
			room->send(data_buffer);
		}

		room->handleInput();
		room->update();

		for(auto& p : room->getData()) {
			pcap_handle->send(p.data(), p.size());
		}
		room->clearData();

		for(auto& p : room->getChat()) {
			std::cout << p.c_str() << std::endl;
 		}
		room->clearChat();
	}

	room.reset();
	delete pcap_handle;
	std::cout << "tunnel done" << std::endl;
}