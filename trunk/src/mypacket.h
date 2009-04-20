#ifndef __MYPACKET_H__
#define __MYPACKET_H__

#include <netinet/ip.h>

#define PACKET_REQUEST 1
#define PACKET_REPLY 2
#define PACKET_ERROR 3
#define PACKET_MASTER_ELECTION 4
#define IAM_MASTER 5
#define MASTER_ACK 6
#define IN_SRC_ID 7
#define OUT_SRC_ID 8

#define PACKET_ERROR_NO_REPLY 1 //probe has no packet in oposite direction to reply with

struct vpPacket{
	#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int type:6;
	unsigned int version:2;
	#endif
	#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int version:2;
	unsigned int type:6;
	#endif
	int32_t time_shift;
	u_int32_t packet_number;
};
#endif
