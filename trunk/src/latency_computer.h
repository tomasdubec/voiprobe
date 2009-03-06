#ifndef __LATENCY_COMPUTER_H__
#define __LATENCY_COMPUTER_H__

#include "mypacket.h"
#include "db.h"

extern DB *db;

class LatencyComputer{
	int seqid, timestamp, realtime, ssrc;
	string hostname; //name of second probe
	int port; //port the second probe is listening on
	int soketka;
public:
	LatencyComputer(string, int);
	~LatencyComputer();
	void start();
	bool connect();
};

#endif

