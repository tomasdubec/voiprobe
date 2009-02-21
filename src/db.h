#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>
#include <iostream>

using namespace std;

extern string itoa(int);

class DB{
	struct sqlite3 *db;
	string filename;
public:
	DB(string);
	~DB();
	bool createTables(void);
	bool insertPacket(int, int, int);
};

#endif

