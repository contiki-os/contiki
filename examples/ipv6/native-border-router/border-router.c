/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         border-router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/slip.h"
#include "cmd.h"
#include "border-router.h"
#include "border-router-cmds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

#define MAX_SENSORS 4

extern long slip_sent;
extern long slip_received;

static uip_ipaddr_t prefix;
static uint8_t prefix_set;
static uint8_t mac_set;

static uint8_t sensor_count = 0;

/* allocate MAX_SENSORS char[32]'s */
static char sensors[MAX_SENSORS][32];

extern int contiki_argc;
extern char **contiki_argv;
extern const char *slip_config_ipaddr;

CMD_HANDLERS(border_router_cmd_handler);

PROCESS(border_router_process, "Border router process");

#if WEBSERVER==0
/* No webserver */
AUTOSTART_PROCESSES(&border_router_process,&border_router_cmd_process);
#elif WEBSERVER>1
/* Use an external webserver application */
#include "webserver-nogui.h"
AUTOSTART_PROCESSES(&border_router_process,&border_router_cmd_process,
		    &webserver_nogui_process);
#else
/* Use simple webserver with only one page */
#include "httpd-simple.h"
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
AUTOSTART_PROCESSES(&border_router_process,&border_router_cmd_process,
		    &webserver_nogui_process);

static const char *TOP = "<html><head><title>ContikiRPL</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
static char buf[128];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0 && sizeof(buf) - blen >= 2) {
        buf[blen++] = ':';
        buf[blen++] = ':';
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0 && blen < sizeof(buf)) {
        buf[blen++] = ':';
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static int i;
  static uip_ds6_route_t *r;
  static uip_ds6_nbr_t *nbr;

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  blen = 0;
  ADD("Neighbors<pre>");
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
    ipaddr_add(&nbr->ipaddr);
    ADD("\n");
    if(blen > sizeof(buf) - 45) {
      SEND_STRING(&s->sout, buf);
      blen = 0;
    }
  }

  ADD("</pre>Routes<pre>");
  SEND_STRING(&s->sout, buf);
  blen = 0;
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {
    ipaddr_add(&r->ipaddr);
    ADD("/%u (via ", r->length);
    ipaddr_add(uip_ds6_route_nexthop(r));
    if(r->state.lifetime < 600) {
      ADD(") %lus\n", (unsigned long)r->state.lifetime);
    } else {
      ADD(")\n");
    }
    SEND_STRING(&s->sout, buf);
    blen = 0;
  }
  ADD("</pre>");
//if(blen > 0) {
  SEND_STRING(&s->sout, buf);
// blen = 0;
//}

  if(sensor_count > 0) {
    ADD("</pre>Sensors<pre>");
    SEND_STRING(&s->sout, buf);
    blen = 0;
    for(i = 0; i < sensor_count; i++) {
      ADD("%s\n", sensors[i]);
      SEND_STRING(&s->sout, buf);
      blen = 0;
    }
    ADD("</pre>");
    SEND_STRING(&s->sout, buf);
  }


  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}

#endif /* WEBSERVER */

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTA("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA(" %p: =>", &uip_ds6_if.addr_list[i]);
      uip_debug_ipaddr_print(&(uip_ds6_if.addr_list[i]).ipaddr);
      PRINTA("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
request_mac(void)
{
  write_to_slip((uint8_t *)"?M", 2);
}
/*---------------------------------------------------------------------------*/
void
border_router_set_mac(const uint8_t *data)
{
  memcpy(uip_lladdr.addr, data, sizeof(uip_lladdr.addr));
  linkaddr_set_node_addr((linkaddr_t *)uip_lladdr.addr);

  /* is this ok - should instead remove all addresses and
     add them back again - a bit messy... ?*/
  uip_ds6_init();
  rpl_init();

  mac_set = 1;
}
/*---------------------------------------------------------------------------*/
void
border_router_print_stat()
{
  printf("bytes received over SLIP: %ld\n", slip_received);
  printf("bytes sent over SLIP: %ld\n", slip_sent);
}

/*---------------------------------------------------------------------------*/
/* Format: <name=value>;<name=value>;...;<name=value>*/
/* this function just cut at ; and store in the sensor array */
void
border_router_set_sensors(const char *data, int len)
{
  int i;
  int last_pos = 0;
  int sc = 0;
  for(i = 0;i < len; i++) {
    if(data[i] == ';') {
      sensors[sc][i - last_pos] = 0;
      memcpy(sensors[sc++], &data[last_pos], i - last_pos);
      last_pos = i + 1; /* skip the ';' */
    }
    if(sc == MAX_SENSORS) {
      sensor_count = sc;
      return;
    }
  }
  sensors[sc][len - last_pos] = 0;
  memcpy(sensors[sc++], &data[last_pos], len - last_pos);
  sensor_count = sc;
}
/*---------------------------------------------------------------------------*/
static void
set_prefix_64(const uip_ipaddr_t *prefix_64)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);

  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
  if(dag != NULL) {
    rpl_set_prefix(dag, &prefix, 64);
    PRINTF("created a new RPL dag\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  prefix_set = 0;

  PROCESS_PAUSE();

  PRINTF("RPL-Border router started\n");

  slip_config_handle_arguments(contiki_argc, contiki_argv);

  /* tun init is also responsible for setting up the SLIP connection */
  tun_init();

  while(!mac_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_mac();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  if(slip_config_ipaddr != NULL) {
    uip_ipaddr_t prefix;

    if(uiplib_ipaddrconv((const char *)slip_config_ipaddr, &prefix)) {
      PRINTF("Setting prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      set_prefix_64(&prefix);
    } else {
      PRINTF("Parse error: %s\n", slip_config_ipaddr);
      exit(0);
    }
  }

#if DEBUG
  print_local_addresses();
#endif

  /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_MAC.off(1);

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    /* do anything here??? */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
