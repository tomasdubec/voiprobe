#include <pcap.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <netinet/ip.h>
#include "packet.h"
#include "db.h"

#ifndef __PACKET_CATCHER_H__
#define __PACKET_CATCHER_H__

using namespace std;

class PacketCatcher{
	pcap_t *pcap_handle;
	string filter;

	void initFilter(void);
	string itoa(int);
	string getStrIP(int);
public:
	PacketCatcher(string);
	~PacketCatcher();
	void setFilter(string);
	void setFilter(string, string);
	void setFilter(string, int, string, int);
	int getSeqNum(const u_char *);
	int getTime(const u_char *);
	void start(bool);
};

#endif

