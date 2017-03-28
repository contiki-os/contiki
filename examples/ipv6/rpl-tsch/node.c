/*
 * Copyright (c) 2015, SICS Swedish ICT.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * \file
 *         A RPL+TSCH node able to act as either a simple node (6ln),
 *         DAG Root (6dr) or DAG Root with security (6dr-sec)
 *         Press use button at startup to configure.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "node-id.h"
#include "net/rpl/rpl.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/rpl/rpl-private.h"
#if WITH_ORCHESTRA
#include "orchestra.h"
#endif /* WITH_ORCHESTRA */

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
#if CONFIG_VIA_BUTTON
#include "button-sensor.h"
#endif /* CONFIG_VIA_BUTTON */

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
#if CONFIG_VIA_BUTTON
AUTOSTART_PROCESSES(&node_process, &sensors_process);
#else /* CONFIG_VIA_BUTTON */
AUTOSTART_PROCESSES(&node_process);
#endif /* CONFIG_VIA_BUTTON */

/*---------------------------------------------------------------------------*/
static void
print_network_status(void)
{
  int i;
  uint8_t state;
  uip_ds6_defrt_t *default_route;
#if RPL_WITH_STORING
  uip_ds6_route_t *route;
#endif /* RPL_WITH_STORING */
#if RPL_WITH_NON_STORING
  rpl_ns_node_t *link;
#endif /* RPL_WITH_NON_STORING */

  PRINTF("--- Network status ---\n");

  /* Our IPv6 addresses */
  PRINTF("- Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTF("-- ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }

  /* Our default route */
  PRINTF("- Default route:\n");
  default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  if(default_route != NULL) {
    PRINTF("-- ");
    PRINT6ADDR(&default_route->ipaddr);
    PRINTF(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
  } else {
    PRINTF("-- None\n");
  }

#if RPL_WITH_STORING
  /* Our routing entries */
  PRINTF("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
  route = uip_ds6_route_head();
  while(route != NULL) {
    PRINTF("-- ");
    PRINT6ADDR(&route->ipaddr);
    PRINTF(" via ");
    PRINT6ADDR(uip_ds6_route_nexthop(route));
    PRINTF(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
    route = uip_ds6_route_next(route);
  }
#endif

#if RPL_WITH_NON_STORING
  /* Our routing links */
  PRINTF("- Routing links (%u in total):\n", rpl_ns_num_nodes());
  link = rpl_ns_node_head();
  while(link != NULL) {
    uip_ipaddr_t child_ipaddr;
    uip_ipaddr_t parent_ipaddr;
    rpl_ns_get_node_global_addr(&child_ipaddr, link);
    rpl_ns_get_node_global_addr(&parent_ipaddr, link->parent);
    PRINTF("-- ");
    PRINT6ADDR(&child_ipaddr);
    if(link->parent == NULL) {
      memset(&parent_ipaddr, 0, sizeof(parent_ipaddr));
      PRINTF(" --- DODAG root ");
    } else {
      PRINTF(" to ");
      PRINT6ADDR(&parent_ipaddr);
    }
    PRINTF(" (lifetime: %lu seconds)\n", (unsigned long)link->lifetime);
    link = rpl_ns_node_next(link);
  }
#endif

  PRINTF("----------------------\n");
}
/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  /* 3 possible roles:
   * - role_6ln: simple node, will join any network, secured or not
   * - role_6dr: DAG root, will advertise (unsecured) beacons
   * - role_6dr_sec: DAG root, will advertise secured beacons
   * */
  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;
  node_role = role_6ln;

  int coordinator_candidate = 0;

#ifdef CONTIKI_TARGET_Z1
  /* Set node with MAC address c1:0c:00:00:00:00:01 as coordinator,
   * convenient in cooja for regression tests using z1 nodes
   * */
  extern unsigned char node_mac[8];
  unsigned char coordinator_mac[8] = { 0xc1, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

  coordinator_candidate = (memcmp(node_mac, coordinator_mac, 8) == 0);
#elif CONTIKI_TARGET_COOJA
  coordinator_candidate = (node_id == 1);
#endif

  if(coordinator_candidate) {
    if(LLSEC802154_ENABLED) {
      node_role = role_6dr_sec;
    } else {
      node_role = role_6dr;
    }
  } else {
    node_role = role_6ln;
  }

#if CONFIG_VIA_BUTTON
  {
#define CONFIG_WAIT_TIME 5

    SENSORS_ACTIVATE(button_sensor);
    etimer_set(&et, CLOCK_SECOND * CONFIG_WAIT_TIME);

    while(!etimer_expired(&et)) {
      printf("Init: current role: %s. Will start in %u seconds. Press user button to toggle mode.\n",
             node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec",
             CONFIG_WAIT_TIME);
      PROCESS_WAIT_EVENT_UNTIL(((ev == sensors_event) &&
                                (data == &button_sensor) && button_sensor.value(0) > 0)
                               || etimer_expired(&et));
      if(ev == sensors_event && data == &button_sensor && button_sensor.value(0) > 0) {
        node_role = (node_role + 1) % 3;
        if(LLSEC802154_ENABLED == 0 && node_role == role_6dr_sec) {
          node_role = (node_role + 1) % 3;
        }
        etimer_restart(&et);
      }
    }
  }

#endif /* CONFIG_VIA_BUTTON */

  printf("Init: node starting with role %s\n",
         node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

  tsch_set_pan_secured(LLSEC802154_ENABLED && (node_role == role_6dr_sec));
  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    net_init(&prefix);
  } else {
    net_init(NULL);
  }

#if WITH_ORCHESTRA
  orchestra_init();
#endif /* WITH_ORCHESTRA */

  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 60);
  while(1) {
    print_network_status();
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
