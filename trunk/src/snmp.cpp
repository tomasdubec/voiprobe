#include "snmp.h"

int vpLatencyHistogramTable_handler(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests){
	netsnmp_request_info *request;
	netsnmp_table_request_info *table_info;
	int *table_entry;

	switch (reqinfo->mode){
	case MODE_GET:
		for(request = requests; request; request = request->next){
			table_entry = (int *) netsnmp_extract_iterator_context(request);
			table_info = netsnmp_extract_table_info(request);

			switch (table_info->colnum) {
			case COLUMN_LATENCYINDEX:
				if(!table_entry){
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry - histogram);
				break;
			case COLUMN_LATENCYINTERVAL:
				if(!table_entry){
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				//snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char*)"ahoj", 4);
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *)legend[table_entry - histogram].c_str(), legend[table_entry - histogram].length());
				break;
			case COLUMN_PACKETCOUNT:
				if(!table_entry){
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, *table_entry);
				break;
			default:
				netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
				break;
			}
		}
		break;
	}
	return SNMP_ERR_NOERROR;
}

netsnmp_variable_list *vpLatencyHistogramTable_get_next_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list *put_index_data, netsnmp_iterator_info *mydata){
	int *entry = (int *) *my_loop_context;
	int *next_entry = entry + 1;
	netsnmp_variable_list *idx = put_index_data;

	if((*entry) != -1){
		snmp_set_var_typed_integer(idx, ASN_INTEGER, entry - histogram);
		idx = idx->next_variable;
		*my_data_context = (void *) entry;
		*my_loop_context = (void *) next_entry;
		return put_index_data;
	}
	else{
		return NULL;
	}
}

netsnmp_variable_list * vpLatencyHistogramTable_get_first_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list * put_index_data, netsnmp_iterator_info *mydata){
	*my_loop_context = histogram;
	return vpLatencyHistogramTable_get_next_data_point(my_loop_context, my_data_context, put_index_data, mydata);
}

Snmp::Snmp(int hs, int hc, int cw){
	histStart = hs;
	histEnd = hs + (hc * cw);
	histStep = cw;
	histUnderflowIndex = hc;
	histOverflowIndex = hc + 1;

	//initialize histogram legend
	legend = new string[hc + 2];
	for(int a = 0; a < hc; a++){
		legend[a] = "<" + itoa(hs + (a * cw)) + ", " + itoa(hs + ((a + 1) * cw)) + ")";
	}
	legend[hc] = "underflow";
	legend[hc + 1] = "overflow";
}

Snmp::~Snmp(){
	delete [] legend;
}

void Snmp::init_vpLatency(void){
	static oid vpLatency_oid[] = { 1, 3, 6, 1, 3, 200, 1, 0 };

	netsnmp_register_read_only_int_instance("vpLatency", vpLatency_oid, OID_LENGTH(vpLatency_oid), &Latence, NULL);
}

void Snmp::init_vpPacketsProcesed(void){
	static oid vpPacketsProcesed_oid[] = { 1, 3, 6, 1, 3, 200, 2, 0 };

	netsnmp_register_read_only_int_instance("vpPacketsProcesed", vpPacketsProcesed_oid, OID_LENGTH(vpPacketsProcesed_oid), &packetsProcesed, NULL);
}

void Snmp::init_vpJitter(void){
	static oid vpJitter_oid[] = { 1, 3, 6, 1, 3, 200, 3, 0 };

	netsnmp_register_read_only_int_instance("vpJitter", vpJitter_oid, OID_LENGTH(vpJitter_oid), &jitter, NULL);
}

void Snmp::init_vpRTPJitter(void){
	static oid vpJitter_oid[] = { 1, 3, 6, 1, 3, 200, 4, 0 };

	netsnmp_register_read_only_int_instance("vpJitter", vpJitter_oid, OID_LENGTH(vpJitter_oid), &jitter1, NULL);
}

void Snmp::init_vpLatencyHistogramTable(void){
	static oid vpLatencyHistogramTable_oid[] = { 1, 3, 6, 1, 3, 200, 5 };
	size_t vpLatencyHistogramTable_oid_len = OID_LENGTH(vpLatencyHistogramTable_oid);
	netsnmp_handler_registration *reg;
	netsnmp_iterator_info *iinfo;
	netsnmp_table_registration_info *table_info;

	reg = netsnmp_create_handler_registration("vpLatencyHistogramTable", vpLatencyHistogramTable_handler, vpLatencyHistogramTable_oid, vpLatencyHistogramTable_oid_len, HANDLER_CAN_RONLY);

	table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
	netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER, 0);
	table_info->min_column = COLUMN_LATENCYINDEX;
	table_info->max_column = COLUMN_PACKETCOUNT;

	iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
	iinfo->get_first_data_point = vpLatencyHistogramTable_get_first_data_point;
	iinfo->get_next_data_point = vpLatencyHistogramTable_get_next_data_point;
	iinfo->table_reginfo = table_info;

	netsnmp_register_table_iterator(reg, iinfo);
}

void Snmp::start(void){
	//print errors to syslog
	snmp_enable_calllog();

	netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);

	/* initialize tcpip, if necessary */
	SOCK_STARTUP;

	/* initialize the agent library */
	init_agent("voiprobe");

	/* mib code: init_nstAgentSubagentObject from nstAgentSubagentObject.C */
	init_vpLatency();
	init_vpJitter();
	init_vpRTPJitter();
	init_vpPacketsProcesed();
	init_vpLatencyHistogramTable();

	/* example-demon will be used to read example-demon.conf files. */
	init_snmp("voiprobe");

	snmp_log(LOG_INFO,"voiprobe is up and running.\n");

	while(run){
 		agent_check_and_process(1);
	}

	snmp_shutdown("voiprobe");
	SOCK_CLEANUP;
}

