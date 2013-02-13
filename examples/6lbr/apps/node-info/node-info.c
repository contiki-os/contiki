#include "contiki.h"
#include "node-info.h"
#include "uip-ds6.h"
#include "uip-ds6-route.h"
#include "string.h"

#if CETIC_NODE_INFO

node_info_t node_info_table[UIP_DS6_ROUTE_NB];          /** \brief Node info table */

static struct uip_ds6_notification node_info_route_notification;

void
node_info_route_notification_cb(int event,
                                uip_ipaddr_t * route,
                                uip_ipaddr_t * nexthop, int num_routes)
{
  if(event == UIP_DS6_NOTIFICATION_ROUTE_RM) {
    node_info_rm(route);
  }
}

void
node_info_init(void)
{
  memset(node_info_table, 0, sizeof(node_info_table));
  uip_ds6_notification_add(&node_info_route_notification,
                           node_info_route_notification_cb);
}

node_info_t *
node_info_add(uip_ipaddr_t * ipaddr)
{
  node_info_t *node = NULL;

  if(uip_ds6_list_loop
     ((uip_ds6_element_t *) node_info_table, UIP_DS6_ROUTE_NB,
      sizeof(node_info_t), ipaddr, 128,
      (uip_ds6_element_t **) & node) == FREESPACE) {
    node->isused = 1;
    uip_ipaddr_copy(&(node->ipaddr), ipaddr);
    node->last_lookup = clock_time();
  }

  return node;
}

void
node_info_rm(uip_ipaddr_t * ipaddr)
{
  node_info_t *node_info = node_info_lookup(ipaddr);

  if(node_info != NULL) {
    node_info->isused = 0;
  }
}

node_info_t *
node_info_lookup(uip_ipaddr_t * ipaddr)
{
  node_info_t *node;

  if(uip_ds6_list_loop((uip_ds6_element_t *) node_info_table,
                       UIP_DS6_ROUTE_NB, sizeof(node_info_t), ipaddr, 128,
                       (uip_ds6_element_t **) & node) == FOUND) {
    return node;
  }
  return NULL;
}

#endif
