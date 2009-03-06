#include "packet.h"

Packet::Packet(const u_char *p, int length){
	rawPacket = new u_char[length];

	memcpy(rawPacket, p, length);
	ip_header = (struct ip*)(rawPacket + SIZE_ETHERNET);
	udp_header = (struct udphdr*)(rawPacket + SIZE_ETHERNET + (ip_header->ip_hl * 4));
	rtp_header = (struct rtp*)(rawPacket + SIZE_ETHERNET + (ip_header->ip_hl * 4) + SIZE_UDP);
}

Packet::~Packet(){
	delete [] rawPacket;
}


