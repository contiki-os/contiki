#include <string.h>
#include <stdlib.h>
#include "lib/random.h"
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "net/uip-packetqueue.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "rio.h"

uip_ds6_route_info_t uip_ds6_route_info_list[UIP_DS6_ROUTE_INFO_NB];/** \brief Route information table */

static uip_ds6_route_info_t *locrtinfo;

uip_ds6_route_info_t *
uip_ds6_route_info_add(uip_ipaddr_t * ipaddr, uint8_t ipaddrlen,
                       uint8_t flags, unsigned long rlifetime)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *) uip_ds6_route_info_list, UIP_DS6_ROUTE_INFO_NB,
      sizeof(uip_ds6_route_info_t), ipaddr, ipaddrlen,
      (uip_ds6_element_t **) & locrtinfo) == FREESPACE) {
    locrtinfo->isused = 1;
    uip_ipaddr_copy(&locrtinfo->ipaddr, ipaddr);
    locrtinfo->length = ipaddrlen;
    locrtinfo->flags = flags;
    locrtinfo->lifetime = rlifetime;
    PRINTF("Adding route information ");
    PRINT6ADDR(&locrtinfo->ipaddr);
    PRINTF("length %u, flags %x, route lifetime %lx\n",
           ipaddrlen, flags, rlifetime);
    return locrtinfo;
  } else {
    PRINTF("No more space in route information list\n");
  }
  return NULL;
}

void
uip_ds6_route_info_rm(uip_ds6_route_info_t * rtinfo)
{
  if(rtinfo != NULL) {
    rtinfo->isused = 0;
  }
  return;
}

uip_ds6_route_info_t *
uip_ds6_route_info_lookup(uip_ipaddr_t * ipaddr, uint8_t ipaddrlen)
{
  if(uip_ds6_list_loop((uip_ds6_element_t *) uip_ds6_route_info_list,
                       UIP_DS6_ROUTE_INFO_NB, sizeof(uip_ds6_route_info_t),
                       ipaddr, ipaddrlen,
                       (uip_ds6_element_t **) & locrtinfo) == FOUND) {
    return locrtinfo;
  }
  return NULL;
}

void
uip_ds6_route_info_callback(uip_nd6_opt_route_info * rio,
                            uip_ip6addr_t * next_hop)
{
  PRINTF("UIP-RIO: callback\n");
  //TODO Preferences ?
  uip_ds6_route_t *found;

  if((found = uip_ds6_route_lookup(&rio->prefix)) == NULL
     && rio->rlifetime != 0) {
    //New route
    PRINTF("UIP-RIO: new route found !\n");
    PRINTF("UIP-RIO: type=%d\n", rio->type);
    PRINTF("UIP-RIO: flags=%d\n", rio->flagsreserved);
    PRINTF("UIP-RIO: length=%d\nUIP-RIO: Preflen=%d\nUIP-RIO: Prefix=",
           rio->len, rio->preflen);
    PRINT6ADDR(&rio->prefix);
    PRINTF("\nUIP-RIO: lifetime=%d\n", uip_ntohl(rio->rlifetime));
    uip_ds6_route_t *new_route;

    if((new_route =
        uip_ds6_route_add(&rio->prefix, rio->preflen, next_hop,
                          UIP_DEFAULT_METRIC)) == NULL) {
      PRINTF("UIP-RIO: error when adding route\n");
    } else {
      PRINTF("UIP-RIO: route added\n");
      new_route->state.lifetime = uip_ntohl(rio->rlifetime);
    }
  } else {
    PRINTF("UIP-RIO: the route already exists\n");
    if(rio->rlifetime == 0) {
      uip_ds6_route_rm(found);
    } else {
      found->state.lifetime = uip_ntohl(rio->rlifetime);
    }
  }
}
