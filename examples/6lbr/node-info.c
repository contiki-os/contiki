#include "node-info.h"
#include "uip-ds6.h"

#if CETIC_NODE_INFO

node_info_t node_info_table[UIP_DS6_ROUTE_NB];          /** \brief Node info table */

node_info_t *
node_info_add(uip_ipaddr_t *ipaddr)
{
  node_info_t  *node = NULL;
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)node_info_table, UIP_DS6_ROUTE_NB,
      sizeof(node_info_t), ipaddr, 128,
      (uip_ds6_element_t **)&node) == FREESPACE) {
    node->isused = 1;
    uip_ipaddr_copy(&(node->ipaddr), ipaddr);
    node->last_lookup = clock_time();
  }

  return node;
}

node_info_t *
node_info_lookup(uip_ipaddr_t *ipaddr)
{
  node_info_t *node;
  if(uip_ds6_list_loop((uip_ds6_element_t *)node_info_table,
		  UIP_DS6_ROUTE_NB, sizeof(node_info_t), ipaddr, 128,
		       (uip_ds6_element_t **)&node) == FOUND) {
    return node;
  }
  return NULL;
}

#endif
