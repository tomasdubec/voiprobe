#include "packet.h"

Packet::Packet(const u_char *p, int length){
	rawPacket = new u_char[length];
	const struct ip *ip_header;

	memcpy(rawPacket, p, length);
	ip_header = (struct ip*)(rawPacket + SIZE_ETHERNET);
	rtp_header = (struct rtp*)(rawPacket + SIZE_ETHERNET + (ip_header->ip_hl * 4) + SIZE_UDP);
}

Packet::~Packet(){
	delete [] rawPacket;
}


