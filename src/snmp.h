#ifndef __VOIPROBE_SNMP_H__
#define __VOIPROBE_SNMP_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <iostream>

#define COLUMN_LATENCYINDEX		1
#define COLUMN_LATENCYINTERVAL		2
#define COLUMN_PACKETCOUNT		3

using namespace std;

extern int Latence;
extern bool run;
extern int *histogram;
extern int packetsProcesed;
extern string *legend;
extern string itoa(int);

struct vpLatencyHistogramTable_entry{
    char            latencyInterval[50];
    size_t          latencyInterval_len;
    long            packetCount;
    int             valid;
};


class Snmp{
	int histStart, histEnd, histStep, histUnderflowIndex, histOverflowIndex;

	void init_vpLatency(void);
	void init_vpPacketsProcesed(void);
	void init_vpLatencyHistogramTable(void);

public:
	Snmp(int, int, int);
	~Snmp();
	void start(void);
};

#endif

