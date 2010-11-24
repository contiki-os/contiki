/*
 * static-routing.c
 *
 *  Created on: Oct 12, 2010
 *      Author: dogan
 */

#include "static-routing.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


#if !UIP_CONF_IPV6_RPL
#include "contiki-net.h"
#include "node-id.h"

void set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}

void configure_routing(void)
{
  PRINTF("configure_routing\n");

  if (node_id < 10) { /*COOJA*/
    /*Go to desktop machine over border router*/
    ADD_ROUTE(DESKTOP_MACHINE_ID, COOJA_BORDER_ROUTER_ID);
  } else { /*SKY*/
    if (node_id < 20) { /*First hops (ids between 10-20)*/
      /*Go to desktop machine over border router*/
      ADD_ROUTE(DESKTOP_MACHINE_ID, BORDER_ROUTER_ID);
    }

    switch(node_id) {
      case 12:
        ADD_ROUTE(22, 22); /*Go to next hop over the local address of next hop*/
        break;
      case 13:
        ADD_ROUTE(23, 23); /*Go to next hop over the local address of next hop*/
        break;

      case 22:
        ADD_ROUTE(0, 12);  /*Go to desktop machine over the corresponding first hop*/
        break;
      case 23:
        ADD_ROUTE(0, 13);  /*Go to desktop machine over the corresponding first hop*/
        break;
      default:
        break;
    }
  }
}
#endif /*!UIP_CONF_IPV6_RPL*/
