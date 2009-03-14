#ifndef __VOIPROBE_SNMP_H__
#define __VOIPROBE_SNMP_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

extern int Latence;
extern bool run;

class Snmp{
public:
	Snmp();
	~Snmp();
	void init_vpLatency(void);
	void start(void);
};

#endif

