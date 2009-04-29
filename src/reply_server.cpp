#include "reply_server.h"

ReplyServer::ReplyServer(PacketCatcher *pcat, string a, int p){
	pc = pcat;
	address = a;
	if(p == -1)
		port = 34567;
	else
		port = p;
}

ReplyServer::~ReplyServer(){

}

bool ReplyServer::startListen(){
	sockaddr_in sockName;

	if((soket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		return -1;
	}
	sockName.sin_family = AF_INET;
	sockName.sin_port = htons(port);
	if(address == "")
		sockName.sin_addr.s_addr = INADDR_ANY;
	else
		sockName.sin_addr.s_addr = inet_addr(address.c_str());

	if(bind(soket, (sockaddr *)&sockName, sizeof(sockName)) == -1){
		return false;
	}
	if(listen(soket, 2) == -1){
		return false;
	}
	return true;
}

void ReplyServer::decideMaster(){
	vpPacket tmp;
	int size;


	/*if((size = recv(soketka, buf, 1000, NULL)) == -1){
		perror("reply server decideMaster recv");
		return;
	}
	if(size > 0){
		buf[size]='\0';
		tmp = (vpPacket *)buf;
	}

	if(tmp->type == IAM_MASTER && master){ //hmmm, two masters?
		rep.type = IAM_MASTER;
		rep.version = 1;
		rep.time_shift = 0;
		rep.packet_number = 0;
		run = false;
		cout << "[!] there can't be two masters!\n";
	}
	else if(master){
		//i'am the master:-)!
		rep.type = IAM_MASTER;
		rep.version = 1;
		rep.time_shift = 0;
		rep.packet_number = 0;
		master = true;
	}
	else if((tmp->type == IAM_MASTER) || (ntohl(tmp->packet_number) > getpid())){
		//i'am the slave :-(
		rep.type = ACK;
		rep.version = 1;
		rep.time_shift = 0;
		rep.packet_number = 0;
		master = false;
	}
	else{
		//i'am the master:-)!
		rep.type = IAM_MASTER;
		rep.version = 1;
		rep.time_shift = 0;
		rep.packet_number = 0;
		master = true;
	}

	if(send(soketka, &rep, sizeof(vpPacket), NULL) == -1){
		perror("reply server decideMaster send");
		return;
	}*/

	while(vote){
		//cout << "*** (rs) locking mtxVoteReceiverWait\n";
		pthread_mutex_lock(&mtxVoteReceiverWait); //wait till latency computer computes voteID

		//cout << "*** waiting for voteID\n";
		if((size = recv(soketka, &tmp, sizeof(vpPacket), NULL)) == -1){
			perror("reply server decideMaster recv");
			return;
		}
		//cout << "*** got voteID: " << ntohl(tmp.packet_number) << endl;

		if(tmp.type != PACKET_MASTER_ELECTION){
			cerr << "unexpected packet!\n";
			exit(-1);
		}

		if(ntohl(tmp.packet_number) > voteID){
			master = false;
			vote = false;
			pthread_mutex_unlock(&mtxVoteSenderWait);
		}
		else if(ntohl(tmp.packet_number) < voteID){
			master = true;
			vote = false;
			pthread_mutex_unlock(&mtxVoteSenderWait);
		}
		else{ //voteIDs are the same (what are the odds that this will happen, hm?)
			pthread_mutex_unlock(&mtxVoteSenderWait);
		}
	}
}

void ReplyServer::start(){
	sockaddr_in clientInfo;
	char buf[1000];
	socklen_t addrlen;
	int size;
	//int soketka;
	vpPacket *req;
	vpPacket tmp;
	int fid;
	int32_t timeshift;
	int isrc, osrc;

	if(!startListen()){
		perror("error creating server");
		return;
	}

	//wait for connection from the other probe
	addrlen = sizeof(clientInfo);
	memset(&clientInfo, 0, sizeof(clientInfo));
	soketka = accept(soket, (sockaddr*)&clientInfo, &addrlen);
	if(soketka == -1){
		perror("accept");
		return;
	}

	cout << "[i] accepted connection from second probe\n";

	decideMaster();
	if(!master){	//i am not master, i'll wait for SRC IDs a then let packetcatcher run
		if((size = recv(soketka, &tmp, sizeof(vpPacket), NULL)) == -1){
			perror("error receiving");
			return;
		}
		if(tmp.type == IN_SRC_ID){
			isrc = ntohl(tmp.packet_number);
			cout << "isrc: " << isrc << endl;
		}
		if((size = recv(soketka, &tmp, sizeof(vpPacket), NULL)) == -1){
			perror("error receiving");
			return;
		}
		if(tmp.type == OUT_SRC_ID){
			osrc = ntohl(tmp.packet_number);
			cout << "osrc: " << osrc << endl;
		}
		pc->setSRCIDs(isrc, osrc);
		//acknowledge receiving of SRC IDs to the other probe
		tmp.type = ACK;
		tmp.version = 1;
		tmp.time_shift = 0;
		tmp.packet_number = 0;
		if(send(soketka, &tmp, sizeof(vpPacket), NULL) == -1){
			perror("send reply server ACK SSRCs");
			return;
		}

		pthread_mutex_unlock(&mtxPCWait);
	}

	//reply to requests
	while(run){
		if((size = recv(soketka, buf, 1000, NULL)) == -1){
			perror("error receiving");
			return;
		}
		if(size > 0){
			buf[size]='\0';
			req = (vpPacket *)buf;
			//cout << "got request for " << ntohl(req->packet_number) << endl;
			if(req->type != PACKET_REQUEST)
				continue;
			
			pthread_mutex_lock(&mtxDB);
			if(db->getClosest(ntohl(req->packet_number), fid, timeshift)){
				pthread_mutex_unlock(&mtxDB);
				//cout << "sending reply (ts: " << timeshift << ")\n";
				//cout << "sending reply:\n\tseqid: " << fid << "\n\ttime shift: " << timeshift << endl;
				int32_t test = ntohl(htonl(timeshift));
				//cout << "htonl(htonl(timeshift)): " << test << endl;
				tmp.type = PACKET_REPLY;
				tmp.version = 1;
				tmp.time_shift = htonl(timeshift);
				tmp.packet_number = htonl(fid);
				if(send(soketka, &tmp, sizeof(vpPacket), NULL) == -1){
					perror("send");
					return;
				}
			}
			else{
				pthread_mutex_unlock(&mtxDB);
				//cout << "sending error\n";
				tmp.type = PACKET_ERROR;
				tmp.version = 1;
				tmp.time_shift = 0;
				tmp.packet_number = htonl(PACKET_ERROR_NO_REPLY);
				if(send(soketka, &tmp, sizeof(vpPacket), NULL) == -1){
					perror("send");
					return;
				}
			}
		}
		else{
			cout << "[+] reply server: client disconnected, quitting\n";
			return;
		}
	}
}


