#include "latency_computer.h"

LatencyComputer::LatencyComputer(string hst, int prt){
	hostname = hst;
	port = prt;
}

LatencyComputer::~LatencyComputer(){

}

bool LatencyComputer::connect(){
	hostent *host;
	sockaddr_in serverSock;

	if((host = gethostbyname(hostname.c_str())) == NULL){
		return false;
	}
	if((soketka = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		return false;
	}
	serverSock.sin_family = AF_INET;
	serverSock.sin_port = htons(port);
	memcpy(&(serverSock.sin_addr), host->h_addr, host->h_length);
	if(connect(soketka, (sockaddr *)&serverSock, sizeof(serverSock)) == -1){
		return false;
	}
	return true;
}

void LatencyComputer::start(){
	int seqid, timestamp, realtime, ssrc;
	vpPacket tmp;

	if(!connect())
		return;

	while(true){
		if(db->getLatestOutgoingPacket(seqid, timestamp, realtime, ssrc)){
			tmp.type = PACKET_REQUEST;
			tmp.version = 1;
			tmp.timeshift = 0;
			tmp.packet_number = seqid;
		}
	}
}


