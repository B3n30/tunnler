//
//  main.cpp
//  tunnler
//
//
#include <stdio.h>
#include "pcapHandle.hpp"
#include "wiredManager.hpp"

const char* src_mac_address ="08:96:d7:e4:6a:9e";	//Fill in MAC of your device

int main() {
	pcapHandle* pcap_handle = new pcapHandle();
	pcap_handle->activate();
	
	char filter_arg[100];
	sprintf(filter_arg, "ether src %s", src_mac_address );
	pcap_handle->setFilter(filter_arg);
	pcap_handle->setDumpFile("/tmp/capture.pcap");

	wiredManager* wman = new wiredManager();

	printf("start tunnel\n");
	for(int j=0; j<50; ++j) {
		int length = pcap_handle->next();
		printf("Got a %d byte packet\n", length);
		if(length > 0) {
			wman->send(pcap_handle->data, length);
			pcap_handle->dump();
		}
		if(wman->select()) {
			while(!wman->data.empty()) {
				pcap_handle->send(wman->data.begin()->first, wman->data.begin()->second);
				wman->data.erase(wman->data.begin());
			}

		}


	}
	printf("done tunnel\n");

}