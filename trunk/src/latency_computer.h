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
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "mypacket.h"
#include "db.h"

extern DB *db;
extern pthread_mutex_t mtxDB;
extern pthread_mutex_t mtxPCWait;
extern pthread_mutex_t mtxLCWait;
extern pthread_mutex_t mtxVoteSenderWait;
extern pthread_mutex_t mtxVoteReceiverWait;
extern int Latence;
extern bool run;
extern int *histogram;
extern int packetsProcesed;
extern int jitter;
extern bool master;
extern bool vote;
extern int voteID;

class LatencyComputer{
	int seqid, timestamp, realtime, ssrc;
	string hostname; //name of second probe
	int port; //port the second probe is listening on
	int soketka;
	int key; //chared memory id
	int *sharedLatency; //shared memory containing current value of latency

	int histStart, histEnd, histStep, histUnderflowIndex, histOverflowIndex; //histogram parameters

	//bool createSharedMem(void);
	void updateHistogram(int);
	int round(double);
	void decideMaster(void);
public:
	LatencyComputer(string, int, int, int, int);
	~LatencyComputer();
	void start();
	bool connectProbe();
	void sendSRCs(int, int);
};

#endif

