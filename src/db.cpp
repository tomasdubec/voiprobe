#include "db.h"

DB::DB(string f){
	db = NULL;
	filename = f;

	if(sqlite3_open(filename.c_str(), &db) != SQLITE_OK){
		cerr << "unable to open db file " << filename << endl;
		throw "unable to open database!";
	}
}

DB::~DB(){
	if(db != NULL)
		if(sqlite3_close(db) != SQLITE_OK)
			cerr << "unable to close db!\n";

	unlink(filename.c_str());
}

bool DB::createTables(void){
	char *err = NULL;

	if(sqlite3_exec(db, "CREATE TABLE packets (seqid INT PRIMARY KEY, timestamp INT NOT NULL, ssrc INT NOT NULL)", NULL, NULL, &err) != SQLITE_OK){
		cerr << "createTables failed: " << err << endl;
		return false;
	}
	return true;
}

bool DB::insertPacket(int seqn, int timestamp, int ssrc){
	char *err = NULL;

	if(sqlite3_exec(db, string("INSERT INTO packets VALUES(" + itoa(seqn) + ", " + itoa(timestamp) + ", " + itoa(ssrc) + ")").c_str(), NULL, NULL, &err) != SQLITE_OK){
		cerr << "insertPacket failed: " << err << endl;
		return false;
	}
	return true;
}


