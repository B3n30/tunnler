//
//  pcapHandle.cpp
//  tunnler
//
//
#include <stdio.h>
#include "pcapHandle.hpp"

pcapHandle::pcapHandle(std::string deviceName) {
    char errbuf[PCAP_ERRBUF_SIZE];
	int errNum;

  if (deviceName == "") {
	  m_device = pcap_lookupdev(errbuf);
	  if(m_device == NULL)
		  pcapFatal("pcap_lookupdev", errbuf);
	  printf("Device is %s\n", m_device);
  } else {
    m_device = deviceName.c_str();
  }

	m_pcap_handle = pcap_create(m_device, errbuf);
	if(m_pcap_handle == NULL)
		pcapFatal("pcap_create", errbuf);

	if((errNum=pcap_can_set_rfmon(m_pcap_handle) != 1))
		pcapFatal("pcap_can_set_rfmon", errNum);

	if((errNum=pcap_set_rfmon(m_pcap_handle,1)))
		pcapFatal("pcap_set_rfmon", errNum);

	pcap_set_snaplen(m_pcap_handle, 2048);
	pcap_set_promisc(m_pcap_handle, 1);
	pcap_set_timeout(m_pcap_handle, 64);
/*
*/
}

pcapHandle::~pcapHandle() {
	if(m_pdumper)
		pcap_dump_close(m_pdumper);
	pcap_set_rfmon(m_pcap_handle,0);
	pcap_close(m_pcap_handle);
}

int pcapHandle::activate(){
    char errbuf[PCAP_ERRBUF_SIZE];
	int res = pcap_activate(m_pcap_handle);
	if (pcap_datalink(m_pcap_handle) != DLT_IEEE802_11) {
		printf("Device %s doesn't provide 802.11 headers, but might be required for 3ds\n",m_device);
//		sprintf(errbuf,"Device %s doesn't provide 802.11 headers\n", m_device);
//		pcapFatal("pcap_datalink", errbuf);
	}

	if (pcap_datalink(m_pcap_handle) != DLT_IEEE802_11_RADIO) {
		sprintf(errbuf,"Device %s doesn't provide 802.11 radiotap headers\n", m_device);
		pcapFatal("pcap_datalink", errbuf);
	}
	return res;
}

int pcapHandle::setFilter(const char* filter_arg) {
	struct bpf_program filter;
	if(pcap_compile(m_pcap_handle,&filter,filter_arg,0,PCAP_NETMASK_UNKNOWN) == -1) {
		fprintf(stderr,"Error calling pcap_compile\n");
		return -1;
	}
    if(pcap_setfilter(m_pcap_handle,&filter) == -1) {
		fprintf(stderr,"Error setting filter\n");
		return -1;
	}
	return 0;
}

int pcapHandle::next(){
	data = pcap_next(m_pcap_handle, &header);
	if(data)
		return header.len;
	return 0;
}

int pcapHandle::send(const u_char* data, int length) {
	return pcap_inject(m_pcap_handle, data, length);
}

void pcapHandle::setDumpFile(const char* fileName) {
	m_pdumper = pcap_dump_open(m_pcap_handle, fileName);
}

void pcapHandle::dump() {
	pcap_dump((u_char*)m_pdumper, &header, data);
}

void pcapHandle::pcapFatal(const char* failed_in, const char* errbuf) {
	printf("Fatal Error in %s: %s\n", failed_in, errbuf);
	throw 1;
}


void pcapHandle::pcapFatal(const char* failed_in, int errNum) {
	printf("Fatal Error in %s: %d\n", failed_in, errNum);
	throw 1;
}
