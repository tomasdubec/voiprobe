#include "latency_computer.h"

/*
 * @hst - other probe hostname
 * @prt - other probe port
 * @hs - histogram start
 * @hc - histogram column count
 * @cw - histogram column width
 */
LatencyComputer::LatencyComputer(string hst, int prt, int hs, int hc, int cw){
	hostname = hst;
	if(prt == -1)
		port = 34567;
	else
		port = prt;

	histStart = hs;
	histEnd = hs + (hc * cw);
	histStep = cw;
	histUnderflowIndex = hc;
	histOverflowIndex = hc + 1;

	packetsProcesed = 0;
	//createSharedMem();
}

LatencyComputer::~LatencyComputer(){

}

bool LatencyComputer::createSharedMem(void){
	int shmid;
	int tmp;

	if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0){
		perror("shmget");
		return false;
	}

	if((sharedLatency = (int *)shmat(shmid, NULL, 0)) == (int *)-1) {
		perror("shmat");
		return false;
	}

	return true;
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

int LatencyComputer::round(double x){
	return int(x + 0.5);
}

/*
 * @l - latency in [us]
 */
void LatencyComputer::updateHistogram(int l){
	double latency = (double)l / 1000.0;

	if(latency < histStart){ //this latency is out of defined histogram
		histogram[histUnderflowIndex]++;
	}
	else if(latency > histEnd){ //this latency is out of defined histogram
		histogram[histOverflowIndex]++;
	}
	else{ //latency fits in histogram
		int index = floor((double)((double)(latency - histStart) / (double)histStep));
		histogram[index]++;
	}
}

void LatencyComputer::decideMaster(void){
	vpPacket tmp;
	int size;

	tmp.type = PACKET_MASTER_ELECTION;
	tmp.version = 1;
	tmp.time_shift = 0;
	tmp.packet_number = htonl(getpid());

	if(send(soketka, (void *)&tmp, sizeof(vpPacket), NULL) == -1){
		perror("decideMaster");
		return;
	}
	if((size = recv(soketka, &tmp, sizeof(vpPacket), NULL)) == -1){
		perror("LatencyComputer decidemaster recv");
		return;
	}
	if(tmp.type == IAM_MASTER){
		//iam slave :-(
		master = false;
		cout << "[i] i am slave\n";

	}
	else if(tmp.type == MASTER_ACK){
		//iam master :-)!
		master = true;
		cout << "[i] i am MASTER\n";
		pthread_mutex_unlock(&mtxPCWait); //let packet catcher go
	}
}

void LatencyComputer::sendSRCs(int i, int o){
	vpPacket tmp;

	//cout << "sending incoming src id " << i << endl;
	//cout << "sending outgoing src id " << o << endl;

	tmp.type = IN_SRC_ID;
	tmp.version = 1;
	tmp.time_shift = 0;
	tmp.packet_number = htonl(i);

	if(send(soketka, (void *)&tmp, sizeof(vpPacket), NULL) == -1){
		perror("sendSRCs 1");
		return;
	}

	tmp.type = OUT_SRC_ID;
	tmp.version = 1;
	tmp.time_shift = 0;
	tmp.packet_number = htonl(o);

	if(send(soketka, (void *)&tmp, sizeof(vpPacket), NULL) == -1){
		perror("sendSRCs 2");
		return;
	}
}

void LatencyComputer::start(){
	int seqid, timestamp, realtime, ssrc, size, attempt = 0;
	int rsid, rtimestamp, rrealtime, rssrc;
	int latency;
	vpPacket tmp;

	while(true){
		cout << "[i] connecting to second probe (" << attempt + 1 << ". attempt)...";
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
	
	cout << "[i] latency computer connected to probe\n";
	cout << "[i] deciding master...\n";
	decideMaster();

	while(run){
		/*if(packetsProcesed % 100 == 0 && packetsProcesed != 0){
			cout << "[";
			for(int i = 0; i < histOverflowIndex; i++){
				cout << round((double)histogram[i] / (double)packetsProcesed * 100.0) << "%|";
			}
				cout << round((double)histogram[histOverflowIndex] / (double)packetsProcesed * 100.0) << "%]\n";
		}*/
		pthread_mutex_lock(&mtxDB);
		if(db->getLatestOutgoingPacket(seqid, timestamp, realtime, ssrc)){
			pthread_mutex_unlock(&mtxDB);
			tmp.type = PACKET_REQUEST;
			tmp.version = 1;
			tmp.time_shift = 0;
			tmp.packet_number = htonl(seqid);
			//cout << "\t\t\t\t\trequesting seqid " << seqid << endl;
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
				//cout << "\t\t\t\t\tgot reply " << rsid << " (ts: " << (int32_t)ntohl(tmp.time_shift) << ")\n";
				pthread_mutex_lock(&mtxDB);
				if(db->getIncomingPacket(rsid, rtimestamp, rrealtime, rssrc)){
					packetsProcesed++;
					latency = rrealtime - realtime + (int32_t)ntohl(tmp.time_shift);
					updateHistogram(latency);
					if(Latence == -1)
						Latence = latency;
					else
						Latence = (Latence * 0.99) + (latency * 0.01);

					if(!db->markGotAnswer(seqid)){
						cerr << "error setting gotAnswer\n";
					}
				}
				pthread_mutex_unlock(&mtxDB);
			}
			else if(tmp.type == PACKET_ERROR){
				//cout << "\t\t\t\t\treceived error\n";
			}
		}
		else{
			pthread_mutex_unlock(&mtxDB);
			usleep(1000);
		}
	}
}

