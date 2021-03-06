#include <pcap.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <netinet/ip.h>
#include <time.h>
#include "packet.h"
#include "latency_computer.h"
#include "db.h"

#ifndef __PACKET_CATCHER_H__
#define __PACKET_CATCHER_H__

using namespace std;

extern DB *db;
extern pthread_mutex_t mtxDB;
extern bool run;
extern int jitter;
extern int jitter1;

class PacketCatcher{
	pcap_t *pcap_handle;
	string filter;
	string shost, dhost;
	LatencyComputer *lc;
	bool ocapture, icapture;
	int issrc_lock, ossrc_lock; //for locking on first starting stream detected
	int osport, odport, isport, idport; //for locking on first starting stream detected

	void initFilter(void);
	string itoa(int);
	string getStrIP(int);
	void setFilter(string);
	void setFilter(string, string);
	void setFilter(string, int, string, int);
public:
	PacketCatcher(string, string, string, LatencyComputer *);
	~PacketCatcher();
	int getSeqNum(const u_char *);
	int getTime(const u_char *);
	void start(bool);
	void setSRCIDs(int, int);
};

#endif

