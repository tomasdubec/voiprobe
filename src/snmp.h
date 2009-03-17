#ifndef __VOIPROBE_SNMP_H__
#define __VOIPROBE_SNMP_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <iostream>

#define COLUMN_LATENCYINTERVAL		1
#define COLUMN_PACKETCOUNT		2

using namespace std;

extern int Latence;
extern bool run;

struct vpLatencyHistogramTable_entry{
    char            latencyInterval[50];
    size_t          latencyInterval_len;
    long            packetCount;
    int             valid;
};

class Snmp{
	void init_vpLatency(void);
public:
	Snmp();
	~Snmp();
	void start(void);
};

#endif

