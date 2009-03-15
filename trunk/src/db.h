#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>
#include <iostream>
#include <stdlib.h>
#include <list>

using namespace std;

extern string itoa(int);

struct __packet{
	int seqid;
	int timestamp;
	int realtime;
	int ssrc;
	bool gotAnswer;
	bool operator<(const __packet&)const;
};

class DB{
	list<__packet> incomingPackets;
	list<__packet> outgoingPackets;
	int req2soon; //for debuging purposes
public:
	DB(string);
	~DB();
	bool insertOutgoingPacket(int, int, int, int);
	bool insertIncomingPacket(int, int, int, int);
	bool getClosest(int, int&, int32_t&);
	bool getLatestOutgoingPacket(int&, int&, int&, int&);
	bool getIncomingPacket(int, int&, int&, int&);
	bool markGotAnswer(int);
};

#endif

