/*
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
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \file
 *         border-router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 *         Thomas Blank <thomas-blank@weptech.de>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/rime/rimestats.h"
#include "lib/print-stats.h"

#if WITH_IP64
#include "net/ip64/ip64.h"
#include "net/ip64/ip64-addrmap.h"
#endif

#if WITH_SLIP
#include "lpm.h"
#endif

#include "dev/button-sensor.h"
#include "dev/leds.h"

#include "project-conf.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dev/slip.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/
#if !WITH_SLIP
/*
 * The fixed prefix for the RPL DAG. This is used when we have now way to assign
 * a prefix from the outside, like we can via SLIP.
 */
static uip_ipaddr_t fixed_rpl_prefix =
{ { 0xfd, 0x4d, 0x42, 0x67, 0x5f, 0x8c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
#endif /* !WITH_SLIP */

static uip_ipaddr_t rpl_prefix;
static uint8_t rpl_prefix_set;

#if STAT_INTERVAL
static int stat_prescaler;
#endif
/*---------------------------------------------------------------------------*/
PROCESS(border_router_process, "Border router process");
/*---------------------------------------------------------------------------*/
/* Start configured webserver. */
#if WEBSERVER == 0
/* No webserver */
AUTOSTART_PROCESSES(&border_router_process);
#elif WEBSERVER > 1
/* Use an external webserver application */
#include "webserver-nogui.h"
AUTOSTART_PROCESSES(&border_router_process, &webserver_nogui_process);
#else
/* Webserver as defined in httpd-simple.c and httpd-simple-handler.c */
#include "httpd-simple-handler.h"
AUTOSTART_PROCESSES(&border_router_process, &httpd_simple_handler);
#endif /* WEBSERVER */

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
#if PRINT_LOCAL_IPS
  int i;
  uint8_t state;

  printf("IPv6 addresses:\n");

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      printf(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }
#endif /* PRINT_LOCAL_IPS */
}
/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  slip_write("?P", 2);
}
/*---------------------------------------------------------------------------*/
void
set_rpl_prefix(uip_ipaddr_t *prefix)
{
  memcpy(&rpl_prefix, prefix, 16);
  rpl_prefix_set = 1;
}
/*---------------------------------------------------------------------------*/
static void
create_rpl_dag(void)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;

  if(!rpl_prefix_set) {
    printf("No RPL prefix set!");
    return;
  }

  memcpy(&ipaddr, &rpl_prefix, 16);

  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
  if(dag != NULL) {
    rpl_set_prefix(dag, &rpl_prefix, 64);
    printf("Created a new RPL dag using prefix:\n ");
    uip_debug_ipaddr_print(&rpl_prefix);
    printf("\n");
  } else {
    printf("Failed to create a new RPL dag!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
print_nat64_table(void)
{
#if WITH_IP64 && PRINT_NAT64_TABLE
  struct ip64_addrmap_entry *m = ip64_addrmap_list();
  if(m == NULL) {
    printf("NAT64 Table empty.\n");
  } else {
    printf("NAT64 Table:\n");
    for(; m != NULL; m = list_item_next(m)) {
      printf(" [");
      uip_debug_ipaddr_print((uip_ipaddr_t *)&(m->ip6addr));
      printf("]:%u <- Our Port %u -> %u.%u.%u.%u:%u   %s\n   6to4 lookups: %lu, 4to6 lookups: %lu, Protocol: %u\n",
             m->ip6port,
             m->mapped_port,
             m->ip4addr.u8[0], m->ip4addr.u8[1], m->ip4addr.u8[2], m->ip4addr.u8[3],
             m->ip4port,
             (m->flags & FLAGS_RECYCLABLE) ? "Stale" : "",
             m->ip6to4,
             m->ip4to6,
             m->protocol
             );
    }
  }
#endif /* PRINT_NAT64_TABLE */
}
/*---------------------------------------------------------------------------*/
static void
print_dhcpv4()
{
#if WITH_IP64 && PRINT_IP_CONF
  const uip_ip4addr_t *hostaddr = ip64_get_hostaddr();
  const uip_ip4addr_t *netmask = ip64_get_netmask();
  const uip_ip4addr_t *draddr = ip64_get_draddr();

  printf("Done:\n");
  printf(" IPv4 address  : %d.%d.%d.%d\n",
         hostaddr->u8[0], hostaddr->u8[1],
         hostaddr->u8[2], hostaddr->u8[3]);
  printf(" netmask       : %d.%d.%d.%d\n",
         netmask->u8[0], netmask->u8[1],
         netmask->u8[2], netmask->u8[3]);
  printf(" default router: %d.%d.%d.%d\n",
         draddr->u8[0], draddr->u8[1],
         draddr->u8[2], draddr->u8[3]);
#endif /* PRINT_IP_CONF */
}
/*---------------------------------------------------------------------------*/
static void
print_usage_stats()
{
#if STAT_INTERVAL
  if(!stat_prescaler) {
    printf("Usage statistics:\n");
    printf(" uptime         : %lu\n", clock_seconds());
    printf(" neighbors      : %i\n", uip_ds6_nbr_num());
    printf(" routes         : %i\n", uip_ds6_route_num_routes());

    print_nat64_table();
#if PRINT_RIME_STATS
    print_stats();
#endif /* PRINT_RIME_STATS */

#if BLINK_INTERVAL
    stat_prescaler = STAT_INTERVAL;
#else
    stat_prescaler = 1;
#endif
  }
  stat_prescaler--;
#endif /* STAT_INTERVAL */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{

  static struct etimer et;

  PROCESS_BEGIN();

  /*
   * While waiting for the IPv4 address to be assigned via DHCP or a prefix
   * assigned via SLIP, the future border router can join an existing DAG as a
   * parent or child, or acquire a default router that will later take
   * precedence over the fallback interface. Prevent that by turning the radio
   * off until we are initialized as a DAG root.
   */

  NETSTACK_MAC.off(0);

#if WITH_SLIP
  /*
   * When SLIP is enabled, we must disable higher LPM's because otherwise the
   * UART will take too long to turn on when it receives packets over the serial
   * line, producing utter garbage in the process.
   */
  lpm_set_max_pm(LPM_PM0);
#endif
#if WITH_IP64

  /*
   * Initialize the IP64 module so we'll start translating packets.
   * If IP64_CONF_DHCP is set, we will first try to obtain an IPv4
   * address via DHCP.
   */

  ip64_init();
#endif

  /* Wait for IPv4 address to be assigned via DHCP */
#if PRINT_IP_CONF
  printf("Requesting IP address");
#endif /* PRINT_IP_CONF */

  leds_off(LEDS_YELLOW);

  rpl_prefix_set = 0;
  while(!rpl_prefix_set) {

#if WITH_IP64
    if(ip64_hostaddr_is_configured()) {
      set_rpl_prefix(&fixed_rpl_prefix);
      break;
    }
#endif

#if WITH_SLIP
    request_prefix();
#endif

#if PRINT_IP_CONF
    printf(".");
#endif /* PRINT_IP_CONF */
    leds_toggle(LEDS_YELLOW);

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  leds_off(LEDS_YELLOW);
  print_dhcpv4();

  create_rpl_dag();

  /* Now turn the radio on, but disable radio duty cycling */
  NETSTACK_MAC.off(1);

  print_local_addresses();
  printf("RPL border router up and running\n");

#if BLINK_INTERVAL
  etimer_set(&et, BLINK_INTERVAL * CLOCK_SECOND);
#elif STAT_INTERVAL
  etimer_set(&et, STAT_INTERVAL * CLOCK_SECOND);
#endif /* BLINK_INTERVAL */

  while(1) {

    PROCESS_YIELD();

    /* Handle BTN1 */
    if(ev == sensors_event && data == &button_sensor) {
      printf("Initiating global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }

    /* "Heartbeat" indication */
    if(ev == PROCESS_EVENT_TIMER && data == &et) {

#if BLINK_INTERVAL
      if(leds_get() & LEDS_YELLOW) {
        leds_off(LEDS_YELLOW);
        etimer_set(&et, BLINK_INTERVAL * CLOCK_SECOND);
      } else {
        leds_on(LEDS_YELLOW);
        etimer_set(&et, 1);

        print_usage_stats();
      }
#elif STAT_INTERVAL
      etimer_set(&et, STAT_INTERVAL * CLOCK_SECOND);
      print_usage_stats();
#endif /* BLINK_INTERVAL */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
