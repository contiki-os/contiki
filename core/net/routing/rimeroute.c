/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: rimeroute.c,v 1.5 2009/11/20 14:35:58 nvt-se Exp $
 */
/**
 * \file
 *         A routing module for uip6 that uses Rime's routing.
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 */

#include <string.h>

#include "net/tcpip.h"
#include "net/uip.h"
#include "net/uip-netif.h"
#include "net/rime.h"
#include "net/sicslowpan.h"
#include "net/rime/route.h"
#include "net/rime/rime-udp.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5],lladdr->addr[6], lladdr->addr[7])
#define PRINTRIMEADDR(addr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3],addr->u8[4], addr->u8[5],addr->u8[6], addr->u8[7])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(lladdr)
#define PRINTRIMEADDR(addr)
#endif /* DEBUG == 1*/

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif /* UIP_LOGGING == 1 */

#define ROUTE_DISCOVERY_CHANNEL	70

#ifndef RIMEROUTE_CONF_CACHE_TIMEOUT
#define CACHE_TIMEOUT		600
#else
#define CACHE_TIMEOUT		RIMEROUTE_CONF_CACHE_TIMEOUT
#endif /* !RIMEROUTE_CONF_CACHE_TIMEOUT */

#ifndef RIMEROUTE_CONF_DISCOVERY_TIMEOUT
#define PACKET_TIMEOUT		(CLOCK_SECOND * 10)
#else
#define PACKET_TIMEOUT		(CLOCK_SECOND * RIMEROUTE_CONF_DISCOVERY_TIMEOUT)
#endif /* RIMEROUTE_CONF_DISCOVERY_TIMEOUT */

static void found_route(struct route_discovery_conn *, const rimeaddr_t *);
static void route_timed_out(struct route_discovery_conn *);

static int activate(void);
static int deactivate(void);
static uip_ipaddr_t *lookup(uip_ipaddr_t *, uip_ipaddr_t *);

const struct uip_router rimeroute = { activate, deactivate, lookup };

static const struct route_discovery_callbacks route_discovery_callbacks =
  { found_route, route_timed_out };

static process_event_t rimeroute_event;

PROCESS(rimeroute_process, "UIP6 rime router");

PROCESS_THREAD(rimeroute_process, ev, data)
{
  static struct route_discovery_conn route_discovery_conn;
  rimeaddr_t *dest;

  PROCESS_BEGIN();

  rimeroute_event = process_alloc_event();

  rime_init(rime_udp_init(NULL));
  /* Cache routes for 10 minutes */
  route_set_lifetime(CACHE_TIMEOUT);

  route_discovery_open(&route_discovery_conn,
                       PACKET_TIMEOUT,
                       ROUTE_DISCOVERY_CHANNEL,
                       &route_discovery_callbacks);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == rimeroute_event);
    dest = data;
    PRINTF("discovering route to %d.%d\n", dest->u8[0], dest->u8[1]);
    route_discovery_discover(&route_discovery_conn, dest, PACKET_TIMEOUT);
  }

  route_discovery_close(&route_discovery_conn);

  PROCESS_END();
}

static void
found_route(struct route_discovery_conn *rdc, const rimeaddr_t *dest)
{
}

static void
route_timed_out(struct route_discovery_conn *rdc)
{
}

/************************************************************************/
static int
activate(void)
{
  PRINTF("Rimeroute started\n");

  process_start(&rimeroute_process, NULL);

  return 0;
}

static int
deactivate(void)
{
  PRINTF("Rimeroute stopped\n");

  return 0;
}

static uip_ipaddr_t *
lookup(uip_ipaddr_t *destipaddr, uip_ipaddr_t *nexthop)
{
  static rimeaddr_t rimeaddr;
  struct route_entry *route;
  int i;

  for(i = 1; i < sizeof(rimeaddr); i++) {
    rimeaddr.u8[i] = destipaddr->u8[sizeof(*destipaddr) - sizeof(rimeaddr) + i];
  }
  rimeaddr.u8[0] = 0;

  PRINTF("rimeroute: looking up ");
  PRINT6ADDR(destipaddr);
  PRINTF(" with Rime address ");
  PRINTRIMEADDR((&rimeaddr));
  PRINTF("\n");

  route = route_lookup(&rimeaddr);
  if(route == NULL) {
    process_post(&rimeroute_process, rimeroute_event, &rimeaddr);
    return NULL;
  }

  uip_ip6addr(nexthop, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  uip_netif_addr_autoconf_set(nexthop, (uip_lladdr_t *)&route->nexthop);
  PRINTF("rimeroute: ");
  PRINT6ADDR(destipaddr);
  PRINTF(" can be reached via ");
  PRINT6ADDR(nexthop);
  PRINTF("\n");

  return nexthop;
}
