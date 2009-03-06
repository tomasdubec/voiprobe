#include <sstream>
#include <sqlite3.h>
#include <pthread.h>
#include "packet_catcher.h"
#include "reply_server.h"
#include "latency_computer.h"

using namespace std;

DB *db;
pthread_mutex_t mtxDB = PTHREAD_MUTEX_INITIALIZER;

string itoa(int number){
	stringstream s;

	s << number;

	return s.str();
}

void *runPacketCatcher(void *d){
	PacketCatcher *pcatcher;

	cout << "[*] packet catcher thread created\n";
	pcatcher = new PacketCatcher("eth1", "192.168.0.100", "192.168.0.55");
	pcatcher->start(true);

	delete pcatcher;
}

void *runReplyServer(void *d){
	ReplyServer *rs;

	cout << "[*] reply server thread created\n";
	rs = new ReplyServer();
	rs->start();
}

int main(int argc, char **argv){
	pthread_t thrCatcher;
	pthread_t thrReply;
	LatencyComputer *lc;

	if(sqlite3_threadsafe() != 1){
		cerr << "voiprobe needs SQLite library to be compiled with SQLITE_THREADSAFE set to 1 (serialized). SQLite library on this system is compiled with SQLITE_THREADSAFE == " << sqlite3_threadsafe() << ".\n";
		exit(-1);
	}
	// this interface is experimental and not included in current stable versions of sqlite. but maybe one day... 
	/*if(sqlite3_threadsafe() != 1){
		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	}*/

	db = new DB("cap.db");
	if(!db->createTables()){
		exit(-1);
	}

	if(pthread_create(&thrCatcher, NULL, runPacketCatcher, NULL) != 0){
		cerr << "unable to create packet catcher thread!\n";
		exit(-1);
	}
	if(pthread_create(&thrReply, NULL, runReplyServer, NULL) != 0){
		cerr << "unable to create reply server thread!\n";
		exit(-1);
	}

	lc = new LatencyComputer("localhost", 34567);
	lc->start();

	delete lc;

	pthread_join(thrCatcher, NULL);
	pthread_join(thrReply, NULL);

	delete db;

	return 0;
}

