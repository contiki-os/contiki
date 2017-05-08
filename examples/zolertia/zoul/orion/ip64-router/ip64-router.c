/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zolertia-orion-router
 * @{
 *
 * \file
 *  Example of an Ethernet IP64 router with weberver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "rpl-dag-root.h"
#include "dev/leds.h"
#include "ip64.h"
#include "net/netstack.h"
#include "httpd-simple.h"

#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define BUFFER_LENGTH                  256
/*---------------------------------------------------------------------------*/
#define LEDS_DHCP                      LEDS_GREEN
#define WEBSERVER_CONF_LOADTIME        0
#define WEBSERVER_CONF_FILESTATS       0
#define WEBSERVER_CONF_NEIGHBOR_STATUS 0
/*---------------------------------------------------------------------------*/
PROCESS(router_node_process, "Router node w/ webserver");
/*---------------------------------------------------------------------------*/
#if WEBSERVER_CONF_ROUTE_LINKS
  #define BUF_USES_STACK 1
#endif

PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&router_node_process,&webserver_nogui_process);
/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Zolertia IP64 Router</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
#if BUF_USES_STACK
static char *bufptr, *bufend;
#define ADD(...) do {                                               \
    bufptr += snprintf(bufptr, bufend - bufptr, __VA_ARGS__);       \
  } while(0)
#else
static char buf[BUFFER_LENGTH];
static int blen;
#define ADD(...) do {                                               \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);  \
  } while(0)
#endif

/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) ADD("::");
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static uip_ds6_route_t *r;
  static uip_ds6_nbr_t *nbr;
#if BUF_USES_STACK
  char buf[BUFFER_LENGTH];
#endif
#if WEBSERVER_CONF_LOADTIME
  static clock_time_t numticks;
  numticks = clock_time();
#endif

  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, TOP);
#if BUF_USES_STACK
  bufptr = buf;bufend=bufptr+sizeof(buf);
#else
  blen = 0;
#endif
  ADD("Neighbors<pre>");

  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {

#if WEBSERVER_CONF_NEIGHBOR_STATUS
#if BUF_USES_STACK
      {
        char* j = bufptr + 25;
        ipaddr_add(&nbr->ipaddr);
        while (bufptr < j) ADD(" ");
        switch (nbr->state) {
          case NBR_INCOMPLETE: ADD(" INCOMPLETE");break;
          case NBR_REACHABLE: ADD(" REACHABLE");break;
          case NBR_STALE: ADD(" STALE");break;
          case NBR_DELAY: ADD(" DELAY");break;
          case NBR_PROBE: ADD(" NBR_PROBE");break;
        }
      }
#else
      {
        uint8_t j = blen + 25;
        ipaddr_add(&nbr->ipaddr);
        while (blen < j) ADD(" ");
        switch (nbr->state) {
          case NBR_INCOMPLETE: ADD(" INCOMPLETE");break;
          case NBR_REACHABLE: ADD(" REACHABLE");break;
          case NBR_STALE: ADD(" STALE");break;
          case NBR_DELAY: ADD(" DELAY");break;
          case NBR_PROBE: ADD(" NBR_PROBE");break;
        }
      }
#endif
#else
      ipaddr_add(&nbr->ipaddr);
#endif

      ADD("\n");
#if BUF_USES_STACK
      if(bufptr > bufend - 45) {
        SEND_STRING(&s->sout, buf);
        bufptr = buf; bufend = bufptr + sizeof(buf);
      }
#else
      if(blen > sizeof(buf) - 45) {
        SEND_STRING(&s->sout, buf);
        blen = 0;
      }
#endif
  }
  ADD("</pre>Routes<pre>");
  SEND_STRING(&s->sout, buf);
#if BUF_USES_STACK
  bufptr = buf; bufend = bufptr + sizeof(buf);
#else
  blen = 0;
#endif

  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {

#if BUF_USES_STACK
#if WEBSERVER_CONF_ROUTE_LINKS
    ADD("<a href=http://[");
    ipaddr_add(&r->ipaddr);
    ADD("]/status.shtml>");
    ipaddr_add(&r->ipaddr);
    ADD("</a>");
#else
    ipaddr_add(&r->ipaddr);
#endif
#else
#if WEBSERVER_CONF_ROUTE_LINKS
    ADD("<a href=http://[");
    ipaddr_add(&r->ipaddr);
    ADD("]/status.shtml>");
    SEND_STRING(&s->sout, buf);
    blen = 0;
    ipaddr_add(&r->ipaddr);
    ADD("</a>");
#else
    ipaddr_add(&r->ipaddr);
#endif
#endif
    ADD("/%u (via ", r->length);
    ipaddr_add(uip_ds6_route_nexthop(r));
    if(1 || (r->state.lifetime < 600)) {
      ADD(") %lus\n", (unsigned long)r->state.lifetime);
    } else {
      ADD(")\n");
    }
    SEND_STRING(&s->sout, buf);
#if BUF_USES_STACK
    bufptr = buf; bufend = bufptr + sizeof(buf);
#else
    blen = 0;
#endif
  }
  ADD("</pre>");

#if WEBSERVER_CONF_FILESTATS
  static uint16_t numtimes;
  ADD("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
  numticks = clock_time() - numticks + 1;
  ADD(" <i>(%u.%02u sec)</i>", numticks / CLOCK_SECOND, 
      (100 * (numticks % CLOCK_SECOND)) / CLOCK_SECOND));
#endif

  SEND_STRING(&s->sout, buf);
  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(router_node_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer et;

  /* Turn radio off while initialazing */
  NETSTACK_MAC.off(0);

  /* Initialize the IP64 module so we'll start translating packets */
  ip64_init();

  printf("Waiting for an address...\n");

  /* Wait to get a DHCP address */
  etimer_set(&et, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    leds_toggle(LEDS_DHCP);

    if(ip64_hostaddr_is_configured()) {

      const uip_ip4addr_t *hostaddr = ip64_get_hostaddr();
      const uip_ip4addr_t *netmask = ip64_get_netmask();
      const uip_ip4addr_t *gwaddr = ip64_get_draddr();

      printf("IPv4 DHCP address: %d.%d.%d.%d\n", hostaddr->u8[0],
                                                 hostaddr->u8[1],
                                                 hostaddr->u8[2],
                                                 hostaddr->u8[3]);
      printf("Netmask : %d.%d.%d.%d\n", netmask->u8[0], netmask->u8[1],
                                        netmask->u8[2], netmask->u8[3]);
      printf("Gateway: %d.%d.%d.%d\n", gwaddr->u8[0], gwaddr->u8[1],
                                       gwaddr->u8[2], gwaddr->u8[3]);
      break;
    }
    etimer_reset(&et);
  }

  leds_off(LEDS_DHCP);

  /* Turn the radio on and create the network */
  NETSTACK_MAC.off(1);

  /* Set us up as a RPL root node. */
  rpl_dag_root_init_dag();

  /* ... and do nothing more. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
