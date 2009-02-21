#include "packet_catcher.h"

using namespace std;

PacketCatcher::PacketCatcher(string iface){
	char ebuf[PCAP_ERRBUF_SIZE];

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

void PacketCatcher::setFilter(string host){
	initFilter();
	filter += "and host " + host;
}

void PacketCatcher::setFilter(string src_host, string dst_host){
	initFilter();
	filter += "and src host " + src_host + " and dst host " + dst_host;
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
	DB db("cap.db");
	int i = 0;

	if(pcap_compile(pcap_handle, &bp, filter.c_str(), 1, 0) == -1){
		cerr << "pcap_compile: " << pcap_geterr(pcap_handle) << endl;
		cerr << "filter: " << filter << endl;
		exit(-1);
	}
	if(pcap_setfilter(pcap_handle, &bp) == -1){
		cerr << "pcap_setfilter: " << pcap_geterr(pcap_handle) << endl;
		exit(-1);
	}

	if(!db.createTables())
		return;

	while(i < 30){
		packet = pcap_next(pcap_handle, &header);
		Packet p(packet, header.len);
		cout << "seq_num: " << dec << p.getSeqNum();
		cout << " time: " << p.getTimestamp() << endl;
		db.insertPacket(p.getSeqNum(), p.getTimestamp(), p.getSsrc());
		/*if(packet != NULL){
			cout << "seq_num: " << dec << getSeqNum(packet);
			cout << " time: " << getTime(packet) << endl;
		}*/
		i++;
	}
}

