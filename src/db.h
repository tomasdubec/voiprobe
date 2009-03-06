#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

extern string itoa(int);

class DB{
	struct sqlite3 *db;
	string filename;
	int pid, timestamp, realtime, ssrc; //info about loaded packet

	static int getPacketCallback(void *, int, char **, char **);
public:
	DB(string);
	~DB();
	bool createTables(void);
	bool insertOutgoingPacket(int, int, int, int);
	bool insertIncomingPacket(int, int, int, int);
	bool getClosest(int, int&, int&);
	bool getLatestOutgoingPacket(int&, int&, int&, int&);
};

#endif

