#ifndef __MYPACKET_H__
#define __MYPACKET_H__

#define PACKET_REQUEST 1
#define PACKET_REPLY 2

struct vpPacket{
	#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int type:6;
	unsigned int version:2;
	#endif
	#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int version:2;
	unsigned int type:6;
	#endif
	u_int16_t time_shift;
	u_int32_t packet_number;
};
#endif
