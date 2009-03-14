#ifndef __REPLY_SERVER_H__
#define __REPLY_SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "db.h"
#include "mypacket.h"

extern DB *db;
extern pthread_mutex_t mtxDB;
extern bool run;

class ReplyServer{
	int port;
	string address;
	int soket;

	bool startListen();

public:
	ReplyServer(string = "", int = 34567);
	~ReplyServer();
	void start();
};

#endif


