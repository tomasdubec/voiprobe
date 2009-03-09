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

	if(sqlite3_exec(db, "CREATE TABLE out_packets (seqid INT PRIMARY KEY, timestamp INT NOT NULL, realtime INT NNOT NULL, ssrc INT NOT NULL)", NULL, NULL, &err) != SQLITE_OK){
		cerr << "createTables failed: " << err << endl;
		return false;
	}
	if(sqlite3_exec(db, "CREATE TABLE in_packets (seqid INT PRIMARY KEY, timestamp INT NOT NULL, realtime INT NOT NULL, ssrc INT NOT NULL)", NULL, NULL, &err) != SQLITE_OK){
		cerr << "createTables failed: " << err << endl;
		return false;
	}
	return true;
}

bool DB::insertOutgoingPacket(int seqn, int timestamp, int realtime, int ssrc){
	char *err = NULL;

	if(sqlite3_exec(db, string("INSERT INTO out_packets VALUES(" + itoa(seqn) + ", " + itoa(timestamp) + ", " + itoa(realtime) + ", " + itoa(ssrc) + ")").c_str(), NULL, NULL, &err) != SQLITE_OK){
		cerr << "insertOutgoingPacket failed: " << err << endl;
		return false;
	}
	return true;
}

bool DB::insertIncomingPacket(int seqn, int timestamp, int realtime, int ssrc){
	char *err = NULL;

	if(sqlite3_exec(db, string("INSERT INTO in_packets VALUES(" + itoa(seqn) + ", " + itoa(timestamp) + ", " + itoa(realtime) + ", " + itoa(ssrc) + ")").c_str(), NULL, NULL, &err) != SQLITE_OK){
		cerr << "insertIncomingPacket failed: " << err << endl;
		return false;
	}
	return true;
}

int DB::getPacketCallback(void *data, int argc, char **argv, char **colName){
	DB *me;

	me = (DB*)data;

	cout << "**** loaded packet ****\n";
	cout << "seqid: " << argv[0] << endl;
	cout << "timestamp: " << argv[1] << endl;
	cout << "realtime: " << argv[2] << endl;
	cout << "ssrc: " << argv[3] << endl;
	cout << "**** ************* ****\n";

	me->pid = atoi(argv[0]);
	me->timestamp = atoi(argv[1]);
	me->realtime = atoi(argv[2]);
	me->ssrc = atoi(argv[3]);

	return 0;
}

bool DB::getLatestOutgoingPacket(int &sid, int &ts, int &rt, int &ss){
	pid = -1;
	char *err;

	if(sqlite3_exec(db, string("SELECT * FROM out_packets ORDER BY realtime DESC LIMIT 1").c_str(), this->getPacketCallback, this, NULL) != SQLITE_OK){
		cerr << "getLatestOutgoingPacket 1 failed!\n";
		return false;
	}

	if(pid == -1)
		return false;

	sid = pid;
	ts = timestamp;
	rt = realtime;
	ss = ssrc;

	//TODO change this to some kind of deletePacket method
	if(sqlite3_exec(db, string("DELETE FROM out_packets WHERE seqid = " + itoa(sid)).c_str(), this->getPacketCallback, this, &err) != SQLITE_OK){
		cerr << "getLatestOutgoingPacket 2 failed: " << err << endl;
		return false;
	}

	return true;
}

bool DB::getClosest(int id, int &fid, int &ts){
	int rt;

	pid = -1;

	if(sqlite3_exec(db, string("SELECT * FROM in_packets WHERE seqid = " + itoa(id)).c_str(), this->getPacketCallback, this, NULL) != SQLITE_OK){
		cerr << "getClosest 1 failed!\n";
		return false;
	}

	if(pid == -1){
		return false;
	}

	rt = realtime;
	
	pid = -1;

	if(sqlite3_exec(db, string("SELECT * FROM out_packets WHERE realtime > " + itoa(realtime) + " ORDER BY realtime ASC LIMIT 1").c_str(), this->getPacketCallback, this, NULL) != SQLITE_OK){
		cerr << "getClosest 2 failed!\n";
		return false;
	}

	if(pid == -1){ //no outgoing packet after the questioned incoming. will try first before
		cout << "novejsi jsem nenasel\n";
		if(sqlite3_exec(db, string("SELECT * FROM out_packets WHERE realtime < " + itoa(realtime) + " ORDER BY realtime DESC LIMIT 1").c_str(), this->getPacketCallback, this, NULL) != SQLITE_OK){
			cerr << "getClosest 3 failed!\n";
			return false;
		}
	}
	if(pid == -1){
		cout << "starsi taky ne\n";
		return false;
	}

	ts = rt - realtime;
	fid = pid;

	return true;
}

bool DB::getIncomingPacket(int sid, int &ts, int &rt, int &ss){
	pid = -1;

	if(sqlite3_exec(db, string("SELECT * FROM in_packets WHERE seqid = " + itoa(sid)).c_str(), this->getPacketCallback, this, NULL) != SQLITE_OK){
		cerr << "getLatestOutgoingPacket 1 failed!\n";
		return false;
	}

	if(pid == -1){
		return false;
	}
	ts = timestamp;
	rt = realtime;
	ss = ssrc;

	return true;
}


