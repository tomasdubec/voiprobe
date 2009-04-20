#ifndef __REPLY_SERVER_H__
#define __REPLY_SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "db.h"
#include "packet_catcher.h"
#include "mypacket.h"

extern DB *db;
extern pthread_mutex_t mtxDB;
extern bool run;
extern bool master;

class ReplyServer{
	int port;
	string address;
	int soket, soketka;
	PacketCatcher *pc;

	bool startListen();
	void decideMaster(void);
public:
	ReplyServer(PacketCatcher *, string = "", int = 34567);
	~ReplyServer();
	void start();
};

#endif


