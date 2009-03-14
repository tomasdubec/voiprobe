#include "snmp.h"

void Snmp::init_vpLatency(void){
	static oid vpLatency_oid[] = { 1, 3, 6, 1, 3, 200, 1, 0 };

	netsnmp_register_read_only_int_instance("vpLatency", vpLatency_oid, OID_LENGTH(vpLatency_oid), &Latence, NULL);
}

Snmp::Snmp(){

}

Snmp::~Snmp(){

}

void Snmp::start(void){
	//print errors to syslog
	snmp_enable_calllog();

	netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);

	/* initialize tcpip, if necessary */
	SOCK_STARTUP;

	/* initialize the agent library */
	init_agent("vpLatency");

	/* mib code: init_nstAgentSubagentObject from nstAgentSubagentObject.C */
	init_vpLatency();  

	/* example-demon will be used to read example-demon.conf files. */
	init_snmp("voiprobe");

	snmp_log(LOG_INFO,"voiprobe is up and running.\n");

	while(run){
 		agent_check_and_process(1);
	}

	snmp_shutdown("voiprobe");
	SOCK_CLEANUP;
}

