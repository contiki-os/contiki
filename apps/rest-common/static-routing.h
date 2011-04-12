/*
 * static-routing.h
 *
 *  Created on: Oct 12, 2010
 *      Author: dogan
 */

#ifndef STATICROUTING_H_
#define STATICROUTING_H_

#if !defined (CONTIKI_TARGET_MINIMAL_NET)
#define NODE_IP(nodeid,type,ipaddr) NODE_##nodeid##_##type(ipaddr)

/*desktop machine*/
#define DESKTOP_MACHINE_ID 0
#define NODE_0_GLOBAL(ipaddr) uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x0001)

/*Cooja Nodes*/
#define COOJA_BORDER_ROUTER_ID 1
#define NODE_1_GLOBAL(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7401, 0x0001, 0x0101)
#define NODE_1_LOCAL(ipaddr)    uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7401, 0x0001, 0x0101)

#define NODE_2_GLOBAL(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202)
#define NODE_2_LOCAL(ipaddr)    uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202)

#define NODE_3_GLOBAL(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7403, 0x0003, 0x0303)
#define NODE_3_LOCAL(ipaddr)    uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7403, 0x0003, 0x0303)

#define NODE_6_GLOBAL(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7406, 0x0006, 0x0606)
#define NODE_6_LOCAL(ipaddr)    uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7406, 0x0006, 0x0606)

/*real nodes*/
#define BORDER_ROUTER_ID 11
#define NODE_11_GLOBAL(ipaddr)  uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7400, 0x116e, 0xd5f1)
#define NODE_11_LOCAL(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7400, 0x116e, 0xd5f1)

#define NODE_12_GLOBAL(ipaddr)  uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7400, 0x1160, 0xf95a)
#define NODE_12_LOCAL(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7400, 0x1160, 0xf95a)

#define NODE_13_GLOBAL(ipaddr)  uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7400, 0x117d, 0x3575)
#define NODE_13_LOCAL(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7400, 0x117d, 0x3575)

#define NODE_22_GLOBAL(ipaddr)  uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7400, 0x116e, 0xc0f6)
#define NODE_22_LOCAL(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7400, 0x116e, 0xc0f6)

#define NODE_23_GLOBAL(ipaddr)  uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7400, 0x117d, 0x0d5a)
#define NODE_23_LOCAL(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7400, 0x117d, 0x0d5a)

#define ADD_ROUTE(node_global,node_local)\
do{\
  uip_ipaddr_t ipaddr_local, ipaddr_global;\
  NODE_IP(node_global, GLOBAL, &ipaddr_global);\
  NODE_IP(node_local, LOCAL, &ipaddr_local);\
  uip_ds6_route_add(&ipaddr_global, 128, &ipaddr_local, 0);\
}while(0)

void set_global_address(void);
void configure_routing(void);

#endif /*CONTIKI_TARGET_MINIMAL_NET*/
#endif /* STATICROUTING_H_ */
