//
//  pcapHandle.hpp
//  tunnelr
//
//

#ifndef pcapHandle_hpp
#define pcapHandle_hpp

#include <stdio.h>
#include <pcap.h>

#include <string>
#include <vector>

class pcapHandle{
	public:		//should be private
    const char* m_device;
    pcap_t* m_pcap_handle;
    pcap_dumper_t* m_pdumper;

	public:
	pcapHandle(std::string deviceName = "");
	~pcapHandle();
	int activate();
	int setFilter(const char* filter_arg);
	int next();
	int send(const u_char* data, int length);
	void setDumpFile(const char* fileName);
	void dump();
	void pcapFatal(const char *failed_in, const char *errbuf);
	void pcapFatal(const char *failed_in, int errNum);

    struct pcap_pkthdr header;
    const u_char* data;
	


};

#endif /* pcapHandle_hpp */
