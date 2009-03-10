#include "latency_computer.h"

LatencyComputer::LatencyComputer(string hst, int prt){
	hostname = hst;
	port = prt;
}

LatencyComputer::~LatencyComputer(){

}

bool LatencyComputer::connectProbe(){
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
	int seqid, timestamp, realtime, ssrc, size, attempt = 0;
	int rsid, rtimestamp, rrealtime, rssrc;
	int latency;
	vpPacket tmp;

	while(true){
		cout << "[*] connecting to second probe (" << attempt + 1 << ". attempt)...";
		if(attempt < 3 && !connectProbe()){
			cout << "failed\n";
		}
		else{
			cout << "success\n";
			break;
		}
		attempt++;
		if(attempt == 3){
			return;
		}
		sleep(5);
	}
	
	cout << "[*] latency computer connected to probe\n";

	while(true){
		pthread_mutex_lock(&mtxDB);
		if(db->getLatestOutgoingPacket(seqid, timestamp, realtime, ssrc)){
			pthread_mutex_unlock(&mtxDB);
			tmp.type = PACKET_REQUEST;
			tmp.version = 1;
			tmp.time_shift = 0;
			tmp.packet_number = htonl(seqid);
			cout << "\t\t\t\t\trequesting seqid " << seqid << endl;
			if(send(soketka, (void *)&tmp, sizeof(vpPacket), NULL) == -1){
				perror("LatencyComputer send error");
				return;
			}
			if((size = recv(soketka, &tmp, sizeof(vpPacket), NULL)) == -1){
				perror("LatencyComputer recv error");
				return;
			}
			if(tmp.type == PACKET_REPLY){
				rsid = ntohl(tmp.packet_number);
				cout << "\t\t\t\t\tgot reply " << rsid << " (ts: " << (int32_t)ntohl(tmp.time_shift) << ")\n";
				pthread_mutex_lock(&mtxDB);
				if(db->getIncomingPacket(rsid, rtimestamp, rrealtime, rssrc)){
					latency = rrealtime - realtime + (int32_t)ntohl(tmp.time_shift);
					cout << "\t\t\t\t\t\t\t\tlatency: " << (double)latency / (double)2000 << "ms\n";
					globalLatency += latency;
					latencyCount++;
				}
				pthread_mutex_unlock(&mtxDB);
			}
			else if(tmp.type == PACKET_ERROR)
				cout << "\t\t\t\t\treceived error\n";
		}
		else{
			pthread_mutex_unlock(&mtxDB);
			sleep(1);
		}
	}
}


