#include "packet_catcher.h"

using namespace std;

PacketCatcher::PacketCatcher(string iface, string src_host, string dst_host){
	char ebuf[PCAP_ERRBUF_SIZE];

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
}

void PacketCatcher::setFilter(string src_host, int src_port, string dst_host, int dst_port){
	initFilter();
	filter += "and src host " + src_host + " and src port " + itoa(src_port) + " and dst host " + dst_host + " and dst port " + itoa(dst_port);
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
	bool ocapture = false, icapture = false;
	int issrc_lock, ossrc_lock; //for locking on first starting stream detected
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
		if(packet == NULL)
			continue;

		gettimeofday(&cas, NULL);
		realtime = (header.ts.tv_sec - start_time) * 1000000 + header.ts.tv_usec;
		//realtime = (cas.tv_sec - start_time) * 1000000 + cas.tv_usec;

		Packet p(packet, header.len);

		if(getStrIP(p.getSHost()) == shost){ // outgoing packet
			if(!ocapture){//we haven't find the begining of outgoing traffic yet
				if(p.getMarker() && p.getTimestamp() == 0){
					ocapture = true;
					ossrc_lock = p.getSsrc();
					cout << "* found start of outgoing comunication" << endl;
				}
				else
					continue;
			}
			if(ossrc_lock == p.getSsrc()){ //it is a packet from our stream
				//cout << "outgoing packet:" << endl;
				db->insertOutgoingPacket(p.getSeqNum(), p.getTimestamp(), realtime, p.getSsrc());
			}
		}
		else{ //incoming packet
			if(!icapture){//we haven't find the begining of incoming traffic yet
				if(p.getMarker() && p.getTimestamp() == 0){
					icapture = true;
					issrc_lock = p.getSsrc();
					cout << "* found start of incoming comunication" << endl;
				}
				else
					continue;
			}
			if(issrc_lock == p.getSsrc()){ //it is a packet from our stream
				//cout << "incoming packet:" << endl;
				db->insertIncomingPacket(p.getSeqNum(), p.getTimestamp(), realtime, p.getSsrc());

				//count jitter
				if(lastTS != -1 && lastRT != -1){
					int dif = abs((realtime - lastRT) - ((p.getTimestamp() - lastTS) / 16 * 1000));
					//jitter = (double)jitter * 0.99 + 0.01*(double)dif;
					/*if(dif > 100000 || dif < 0){
						cout << "error, analysis: (" << p.getSeqNum() << ")\n";
						cout << "\trealtime: " << realtime << "\n\tlastRT: " << lastRT << "\n\tp.getTimestamp(): " << p.getTimestamp() << "\n\tlastTS: " << lastTS << endl;
						cout << "\t(realtime - lastRT): " << (realtime - lastRT) << endl;
						cout << "\t(p.getTimestamp() - lastTS) / 16 * 1000: " << (p.getTimestamp() - lastTS) / 16 * 1000 << endl;
						cout << "\tdif: " << dif << endl;
					}*/
					jitter = jitter + dif - jitter / 16;
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

