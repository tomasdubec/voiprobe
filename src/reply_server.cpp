#include "reply_server.h"

ReplyServer::ReplyServer(string a, int p){
	address = a;
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

void ReplyServer::start(){
	sockaddr_in clientInfo;
	char buf[1000];
	socklen_t addrlen;
	int size;
	int soketka;
	vpPacket *req;
	vpPacket tmp;
	int fid;
	int32_t timeshift;

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

	cout << "[*] second probe connected\n";

	//reply to requests
	while(true){
		if((size = recv(soketka, buf, 1000, NULL)) == -1){
			perror("error receiving");
			return;
		}
		if(size > 0){
			buf[size]='\0';
			req = (vpPacket *)buf;
			cout << "got request for " << ntohl(req->packet_number) << endl;
			if(req->type != PACKET_REQUEST)
				continue;
			
			pthread_mutex_lock(&mtxDB);
			if(db->getClosest(ntohl(req->packet_number), fid, timeshift)){
				pthread_mutex_unlock(&mtxDB);
				cout << "sending reply (ts: " << timeshift << ")\n";
				//cout << "sending reply:\n\tseqid: " << fid << "\n\ttime shift: " << timeshift << endl;
				int32_t test = ntohl(htonl(timeshift));
				cout << "htonl(htonl(timeshift)): " << test << endl;
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
				cout << "sending error\n";
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
			cout << "[*] reply server: client disconnected, quitting\n";
			return;
		}
	}
}


