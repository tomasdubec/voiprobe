#include "packet_catcher.h"

using namespace std;

PacketCatcher::PacketCatcher(string iface, string src_host, string dst_host, LatencyComputer *l){
	char ebuf[PCAP_ERRBUF_SIZE];

	lc = l;
	ocapture = icapture = false;
	
	shost = src_host;
	dhost = dst_host;

	if((pcap_handle = pcap_open_live(iface.c_str(), 65000, 0, 1, ebuf)) == NULL){
		cerr << "pcap_open_live: " << ebuf << endl;
		exit(-1);
	}
}

PacketCatcher::~PacketCatcher(){
	if(pcap_handle != NULL)
		pcap_close(pcap_handle);
}

string PacketCatcher::itoa(int i){
	stringstream o;

	o << i;

	return o.str();
}

/*
 * converts 32bit integer into IP address in dot notation (123.456.789.012)
 */
string PacketCatcher::getStrIP(int ip){
	int p1, p2, p3, p4;
	stringstream ret;

	p1 = ip;
	p1 = (p1 & 0xff000000) >> 24;
	p2 = ip;
	p2 = (p2 & 0x00ff0000) >> 16;
	p3 = ip;
	p3 = (p3 & 0x0000ff00) >> 8;
	p4 = ip;
	p4 = (p4 & 0x000000ff);
	
	ret << p1 << "." << p2 << "." << p3 << "." << p4;

	return ret.str();
}

void PacketCatcher::initFilter(void){
	filter = "udp ";
}

void PacketCatcher::setFilter(string host1, string host2){
	initFilter();
	filter += "and (host " + host1 + " or host " + host2 + ")";
	cout << "filter: " << filter << endl;
}

void PacketCatcher::setFilter(string src_host, int src_port, string dst_host, int dst_port){
	initFilter();
	filter += "and src host " + src_host + " and src port " + itoa(src_port) + " and dst host " + dst_host + " and dst port " + itoa(dst_port);
}

void PacketCatcher::setSRCIDs(int is, int os){
	icapture = true;
	ocapture = true;

	ossrc_lock = os;
	issrc_lock = is;

	//cout << "locking on incoming src id " << is << endl;
	//cout << "locking on outgoing src id " << os << endl;
}

/*
 * @param adapt - look for start of conversation and adapt filter accordingly
 */
void PacketCatcher::start(bool adapt){
	struct bpf_program bp;
	struct pcap_pkthdr header;
	const struct ip *ip_header;
	const struct rtp *rtp_header;
	const u_char *packet;
	int i = 0;
	struct timeval cas;
	int start_time;
	long realtime;
	int lastTS = -1, lastRT = -1; //last timestamp and last realtime for jitter calculation

	setFilter(shost, dhost);

	if(pcap_compile(pcap_handle, &bp, filter.c_str(), 1, 0) == -1){
		cerr << "pcap_compile: " << pcap_geterr(pcap_handle) << endl;
		cerr << "filter: " << filter << endl;
		exit(-1);
	}
	if(pcap_setfilter(pcap_handle, &bp) == -1){
		cerr << "pcap_setfilter: " << pcap_geterr(pcap_handle) << endl;
		exit(-1);
	}

	start_time = time(NULL);

	while(run){
		packet = pcap_next(pcap_handle, &header);
		if(packet == NULL || header.len < SIZE_ETHERNET + SIZE_UDP + sizeof(rtp)){
			continue;
		}

		gettimeofday(&cas, NULL);
		realtime = (header.ts.tv_sec - start_time) * 1000000 + header.ts.tv_usec;
		//realtime = (cas.tv_sec - start_time) * 1000000 + cas.tv_usec;

		Packet p(packet, header.len);
		//is this really a RTP packet?
		if(p.getVersion() != 1 && p.getVersion() != 2){
			continue; //not an RTP packet
		}

	//	cout << "getStrIP(p.getSHost(): " << getStrIP(p.getSHost()) << endl;
		if(getStrIP(p.getSHost()) == shost){ // outgoing packet
			if(!ocapture){//we haven't find the begining of outgoing traffic yet
				if(p.getMarker() && p.getTimestamp() == 0){
					ocapture = true;
					ossrc_lock = p.getSsrc();
					osport = p.getSPort();
					odport = p.getDPort();
					cout << "* found start of outgoing comunication (ports " << osport << ":" << odport << ")" << endl;
					if(master && icapture == true){ //i am master and i have locked myself on streams in both directions. i'll tell the other probe
						lc->sendSRCs(ossrc_lock, issrc_lock);
					}
				}
				else
					continue;
			}
			if(ossrc_lock == p.getSsrc()){ //it is a packet from our stream
				//cout << "outgoing packet:" << endl;
				pthread_mutex_lock(&mtxDB);
				db->insertOutgoingPacket(p.getSeqNum(), p.getTimestamp(), realtime, p.getSsrc());
				pthread_mutex_unlock(&mtxDB);
			}
		}
		else{ //incoming packet
			if(!icapture){//we haven't find the begining of incoming traffic yet
				if(p.getMarker() && p.getTimestamp() == 0){
					icapture = true;
					issrc_lock = p.getSsrc();
					isport = p.getSPort();
					idport = p.getDPort();
					cout << "* found start of incoming comunication (ports " << isport << ":" << idport << ")" << endl;
					if(master && ocapture == true){ //i am master and i have locked myself on streams in both directions. i'll tell the other probe
						lc->sendSRCs(ossrc_lock, issrc_lock);
					}
				}
				else
					continue;
			}
			if(issrc_lock == p.getSsrc()){ //it is a packet from our stream
				//cout << "incoming packet:" << endl;
				pthread_mutex_lock(&mtxDB);
				db->insertIncomingPacket(p.getSeqNum(), p.getTimestamp(), realtime, p.getSsrc());
				pthread_mutex_unlock(&mtxDB);

				//count jitter
				if(lastTS != -1 && lastRT != -1){
					int dif = abs((realtime - lastRT) - (((p.getTimestamp() - lastTS) / 16) * 1000)); //dif v us
					int dif1 = abs((((realtime - lastRT) * 16) / 1000) - (p.getTimestamp() - lastTS)); //dif v rtp jednotkach

					/*cout << "---------------------------------------------------------------\n";
					cout << "\trealtime: " << realtime << "\n\tlastRT: " << lastRT << "\n\tp.getTimestamp(): " << p.getTimestamp() << "\n\tlastTS: " << lastTS << endl;
					cout << "\t(realtime - lastRT): " << (realtime - lastRT) << endl;
					cout << "\t(p.getTimestamp() - lastTS) / 16 * 1000: " << (p.getTimestamp() - lastTS) / 16 * 1000 << endl;
					cout << "\tdif: " << dif << endl;*/

					jitter = (0.9 * (double)jitter) + (0.1 * (double)dif); //jitter
					//jitter = dif; //jitter

					//jitter1 = (jitter1) * 0.9 + ((double)(jitter1 + ((dif - jitter1) / 16))) * 0.1; //jitter dle rfc1889
					jitter1 = jitter1 + ((double)(dif1 - jitter1) / 16.0); //jitter dle rfc1889
				}
				lastTS = p.getTimestamp();
				lastRT = realtime;
			}
		}

		/*cout << "\tseq_num: " << dec << p.getSeqNum();
		cout << " time: " << p.getTimestamp();
		cout << " ssrc: 0x" << hex << p.getSsrc() << dec << endl << endl;*/

		i++;
	}
}

