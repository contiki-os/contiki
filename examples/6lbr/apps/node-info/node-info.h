#ifndef NODE_INFO_H_
#define NODE_INFO_H_

#include <contiki.h>
#include <contiki-net.h>

/** \brief An entry in the node info table */
typedef struct node_info {
  uint8_t isused;
  uip_ipaddr_t ipaddr;
  clock_time_t last_lookup;
  char my_info[64];             // Full IPv6 address + 32 bits sequence counter + margin
} node_info_t;

extern node_info_t node_info_table[UIP_DS6_ROUTE_NB];          /** \brief Node info table */

void
  node_info_init(void);

node_info_t *node_info_add(uip_ipaddr_t * ipaddr);

void
  node_info_rm(uip_ipaddr_t * ipaddr);

node_info_t *node_info_lookup(uip_ipaddr_t * ipaddr);

#endif
