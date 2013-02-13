#ifndef CETIC_BRIDGE_H_
#define CETIC_BRIDGE_H_

#include "net/rpl/rpl.h"

PROCESS_NAME(cetic_bridge_process);

typedef uint8_t ethaddr_t[6];

//Initialisation flags
extern int ethernet_ready;
extern int eth_mac_addr_ready;

// WSN Side
extern uip_lladdr_t wsn_mac_addr;
extern uip_ipaddr_t wsn_ip_addr;
extern uip_ipaddr_t wsn_ip_local_addr;  //Created from wsn_mac_addr

extern uip_ip6addr_t wsn_net_prefix;

extern rpl_dag_t *cetic_dag;

// Ethernet side
extern ethaddr_t eth_mac_addr;
extern uip_lladdr_t eth_mac64_addr;     //Created from eth_mac_addr

extern uip_ipaddr_t eth_ip_addr;
extern uip_ipaddr_t eth_ip_local_addr;  //Created from eth_mac_addr

extern uip_ipaddr_t eth_net_prefix;

extern uip_ipaddr_t eth_dft_router;


#endif
