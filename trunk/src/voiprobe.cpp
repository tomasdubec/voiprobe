#include <sstream>
#include <sqlite3.h>
#include <pthread.h>
#include "packet_catcher.h"
#include "reply_server.h"
#include "latency_computer.h"
#include "snmp.h"

using namespace std;

DB *db;
pthread_mutex_t mtxDB = PTHREAD_MUTEX_INITIALIZER;
int Latence;
bool run;

string itoa(int number){
	stringstream s;

	s << number;

	return s.str();
}

void *runPacketCatcher(void *d){
	PacketCatcher *pcatcher;
	char **argv = (char **)d;

	cout << "[*] packet catcher thread created\n";
	pcatcher = new PacketCatcher(argv[1], argv[2], argv[3]);
	//pcatcher = new PacketCatcher("eth1", "192.168.0.100", "192.168.0.55");
	pcatcher->start(true);

	delete pcatcher;

	cout << "[*] packet catcher thread quitting\n";
}

void *runReplyServer(void *d){
	ReplyServer *rs;

	cout << "[*] reply server thread created\n";
	rs = new ReplyServer();
	rs->start();

	delete rs;

	cout << "[*] reply server thread quitting\n";
}

void *runSnmpServer(void *d){
	Snmp *s;

	cout << "[*] snmp server thread created\n";
	s = new Snmp();
	s->start();

	delete s;

	cout << "[*] snmp server thread quitting\n";
}

void usage(void){
	cout << "usage: voiprobe interface my_client_IP other_client_IP other_probe_IP other_probe_port" << endl;
}

int main(int argc, char **argv){
	pthread_t thrCatcher;
	pthread_t thrReply;
	pthread_t thrSnmp;
	LatencyComputer *lc;

	Latence = -1;
	run = true;

	if(argc < 5){
		usage();
		exit(0);
	}

	if(sqlite3_threadsafe() != 1){
		cerr << "voiprobe needs SQLite library to be compiled with SQLITE_THREADSAFE set to 1 (serialized). SQLite library on this system is compiled with SQLITE_THREADSAFE == " << sqlite3_threadsafe() << ".\n";
		exit(-1);
	}
	// this interface is experimental and not included in current stable versions of sqlite. but maybe one day... 
	/*if(sqlite3_threadsafe() != 1){
		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	}*/

	db = new DB("cap.db");
/*	if(!db->createTables()){
		exit(-1);
	}*/

	if(pthread_create(&thrCatcher, NULL, runPacketCatcher, (void*)argv) != 0){
		cerr << "unable to create packet catcher thread!\n";
		exit(-1);
	}
	if(pthread_create(&thrReply, NULL, runReplyServer, NULL) != 0){
		cerr << "unable to create reply server thread!\n";
		exit(-1);
	}
	if(pthread_create(&thrSnmp, NULL, runSnmpServer, NULL) != 0){
		cerr << "unable to create snmp server thread!\n";
		exit(-1);
	}

	cout << "[*] latency computer thread created\n";

	lc = new LatencyComputer(argv[4], atoi(argv[5]));
	lc->start();

	delete lc;

	cout << "[*] latency computer thread quitting\n";

	pthread_join(thrCatcher, NULL);
	pthread_join(thrReply, NULL);

	delete db;

	return 0;
}

