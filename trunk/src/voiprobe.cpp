#include <sstream>
#include <sqlite3.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "packet_catcher.h"
#include "reply_server.h"
#include "latency_computer.h"
#include "snmp.h"

using namespace std;

DB *db;
pthread_mutex_t mtxDB = PTHREAD_MUTEX_INITIALIZER;
int Latence;
bool run;
int histColWidth, histColumns, histStart;
int *histogram;
string *legend;
int packetsProcesed;
int jitter, jitter1;

string itoa(int number){
	stringstream s;

	s << number;

	return s.str();
}

void *runPacketCatcher(void *d){
	PacketCatcher *pcatcher = (PacketCatcher *)d;
	//char **argv = (char **)d;

	cout << "[*] packet catcher thread created\n";
	//pcatcher = new PacketCatcher(argv[1], argv[2], argv[3]);
	pcatcher->start(true);

	//delete pcatcher;

	cout << "[+] packet catcher thread quitting\n";
}

void *runReplyServer(void *d){
	ReplyServer *rs;
	int port = *(int*)d;

	cout << "[*] reply server thread created\n";
	rs = new ReplyServer("", port);
	rs->start();

	delete rs;

	cout << "[+] reply server thread quitting\n";
}

void *runSnmpServer(void *d){
	Snmp *s = (Snmp*)d;

	cout << "[*] snmp server thread created\n";
	//s = new Snmp();
	s->start();

	delete s;

	cout << "[+] snmp server thread quitting\n";
}

void usage(string myname){
	cout << "usage: " << myname << " [options]" << endl;
	cout << "where options are:\n";
	cout << "\t-i interface_name\t set interface on which to sniff packets (*)\n";
	cout << "\t-m hostname\t\t of my client (*)\n";
	cout << "\t-r hostname\t\t of remote client (*)\n";
	cout << "\t-s ip_addr\t\t IP address of the other probe (*)\n";
	cout << "\t-p port\t\t\t port on which the other probe listens (default: 34567)\n";
	cout << "\t-l port\t\t\t port on which to listen for connection from the other probe (default: 34567)\n";
	cout << "\t-c number\t\t histogram column width in [ms] (default: 1ms)\n";
	cout << "\t-n number\t\t number of columns in histogram (default: 20)\n";
	cout << "\t-b number\t\t starting time of histogram in [ms] (default: 1ms)\n";
	cout << "\noptions marked (*) are mandatory and must be set!\n";
}

int main(int argc, char **argv){
	pthread_t thrCatcher;
	pthread_t thrReply;
	pthread_t thrSnmp;
	LatencyComputer *lc;
	char c;
	string myClient, remoteClient, probeIP, interface;
	int probePort, listenPort;

	histColWidth = histColumns = histStart = -1;
	myClient = remoteClient = probeIP = interface = "";
	probePort = listenPort = -1;
	Latence = -1;
	jitter = jitter1 = 0;
	run = true;

	while((c = getopt(argc, argv, "i:m:r:s:p:l:c:n:b:")) != -1){
		switch(c){
		case 'i':
			interface = optarg;
			break;
		case 'm':
			myClient = optarg;
			break;
		case 'r':
			remoteClient = optarg;
			break;
		case 's':
			probeIP = optarg;
			break;
		case 'p':
			probePort = atoi(optarg);
			break;
		case 'l':
			listenPort = atoi(optarg);
			break;
		case 'c':
			histColWidth = atoi(optarg);
			break;
		case 'n':
			histColumns = atoi(optarg);
			break;
		case 'b':
			histStart = atoi(optarg);
			break;
		case '?':
			usage(argv[0]);
			exit(0);
		}
	}
	if(myClient == "" || remoteClient == "" || probeIP == "" || interface == ""){
		usage(argv[0]);
		exit(0);
	}
	if(histColWidth == -1) histColWidth = 1;
	if(histColumns == -1) histColumns = 20;
	if(histStart == -1) histStart = 1;
	cout << "[i] histogram: from " << histStart << "ms to " << histStart + (histColumns * histColWidth)<< "ms with " << histColWidth << "ms steps\n";
	histogram = new int[histColumns + 3];
	histogram[histColumns + 2] = -1;
	memset(histogram, 0, (histColumns + 2) * sizeof(int));

	/*if(sqlite3_threadsafe() != 1){
		cerr << "voiprobe needs SQLite library to be compiled with SQLITE_THREADSAFE set to 1 (serialized). SQLite library on this system is compiled with SQLITE_THREADSAFE == " << sqlite3_threadsafe() << ".\n";
		exit(-1);
	}*/
	// this interface is experimental and not included in current stable versions of sqlite. but maybe one day... 
	/*if(sqlite3_threadsafe() != 1){
		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	}*/

	db = new DB("cap.db");
/*	if(!db->createTables()){
		exit(-1);
	}*/

	PacketCatcher pcatcher(interface, myClient, remoteClient);

	if(pthread_create(&thrCatcher, NULL, runPacketCatcher, (void*)(&pcatcher)) != 0){
		cerr << "unable to create packet catcher thread!\n";
		exit(-1);
	}

	if(pthread_create(&thrReply, NULL, runReplyServer, (void*)(&listenPort)) != 0){
		cerr << "unable to create reply server thread!\n";
		exit(-1);
	}

	Snmp s(histStart, histColumns, histColWidth);
	if(pthread_create(&thrSnmp, NULL, runSnmpServer, (void*)(&s)) != 0){
		cerr << "unable to create snmp server thread!\n";
		exit(-1);
	}

	cout << "[*] latency computer thread created\n";

	lc = new LatencyComputer(probeIP, probePort, histStart, histColumns, histColWidth);
	lc->start();

	delete lc;

	cout << "[+] latency computer thread quitting\n";

	pthread_join(thrCatcher, NULL);
	pthread_join(thrReply, NULL);
	pthread_join(thrSnmp, NULL);

	delete [] histogram;
	delete db;

	return 0;
}

