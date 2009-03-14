#ifndef __LATENCY_COMPUTER_H__
#define __LATENCY_COMPUTER_H__

#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "mypacket.h"
#include "db.h"

extern DB *db;
extern pthread_mutex_t mtxDB;
extern int Latence;
extern bool run;

class LatencyComputer{
	int seqid, timestamp, realtime, ssrc;
	string hostname; //name of second probe
	int port; //port the second probe is listening on
	int soketka;
	int key; //chared memory id
	int *sharedLatency; //shared memory containing current value of latency

	bool createSharedMem(void);
public:
	LatencyComputer(string, int);
	~LatencyComputer();
	void start();
	bool connectProbe();
};

#endif

