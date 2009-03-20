#include <pcap.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <netinet/ip.h>
#include <time.h>
#include "packet.h"
#include "db.h"

#ifndef __PACKET_CATCHER_H__
#define __PACKET_CATCHER_H__

using namespace std;

extern DB *db;
extern pthread_mutex_t mtxDB;
extern bool run;
extern int jitter;

class PacketCatcher{
	pcap_t *pcap_handle;
	string filter;
	string shost, dhost;

	void initFilter(void);
	string itoa(int);
	string getStrIP(int);
	void setFilter(string);
	void setFilter(string, string);
	void setFilter(string, int, string, int);
public:
	PacketCatcher(string, string, string);
	~PacketCatcher();
	int getSeqNum(const u_char *);
	int getTime(const u_char *);
	void start(bool);
};

#endif

