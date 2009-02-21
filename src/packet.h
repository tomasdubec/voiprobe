#ifndef __PACKET_H__
#define __PACKET_H__

#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_ETHERNET 14
#define SIZE_UDP 8

struct rtp{
	#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int csrc_cnt:4;
	unsigned int extension:1;
	unsigned int padding:1;
	unsigned int version:2;
	unsigned int payload_type:7;
	unsigned int marker:1;
	#endif
	#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int version:2;
	unsigned int padding:1;
	unsigned int extension:1;
	unsigned int csrc_cnt:4;
	unsigned int marker:1;
	unsigned int payload_type:7;
	#endif
	u_int16_t sequence_number;
	u_int32_t timestamp;
	u_int32_t ssrc_id;
};

class Packet{
	u_char *rawPacket;
	const struct rtp *rtp_header;
public:
	Packet(const u_char*, int);
	~Packet();
	int getSeqNum(){return ntohs(rtp_header->sequence_number);}
	int getTimestamp(){return ntohl(rtp_header->timestamp);}
	int getSsrc(){return ntohl(rtp_header->ssrc_id);}
	int getMarker(){return rtp_header->marker;}
};

#endif

