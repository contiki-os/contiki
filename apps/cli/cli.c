/*
 * Copyright (c) 2011-2016, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Minimal and simple serial line API for ipv6 and RPL monitoring
 *         See help command to get an overview
 *
 * \Author
 *         Robert Olsson
 *
 * \Used code from border-router.c with authors
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *
 * \Used code from pdr.c with authors:
 *         Atis Elsts       <atis.elsts@bristol.ac.uk>
 *         Christian Rohner <christian.rohner@it.uu.se>
 *         Robert Olsson    <roolss@kth.se>
 */

#define CLI_VERSION "0.9-2017-03-13\n"

#if CONTIKI_TARGET_AVR_RSS2
#define radio_set_txpower rf230_set_txpower
#define radio_get_txpower rf230_get_txpower
#define radio_get_rssi    rf230_rssi
#endif

#include "contiki.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/framer-802154.h"
#include "net/link-stats.h"
#include <string.h>
#include <stdlib.h>
#include "net/packetbuf.h"
#include "dev/serial-line.h"
#include "dev/i2c.h"
#include "sys/process.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#if CONTIKI_TARGET_AVR_RSS2
#include <avr/wdt.h>
#endif

#ifdef CLI_CONF_COMMAND_PROMPT
#define CLI_COMMAND_PROMPT CLI_CONF_COMMAND_PROMPT
#else
#define CLI_COMMAND_PROMPT  "> "
#endif

#ifdef CLI_CONF_PROJECT
#define CLI_PROJECT CLI_CONF_PROJECT
#else
#define CLI_PROJECT  ""
#endif

const char *delim = " \t\r,";
#define END_OF_FILE 26
uint8_t eof = END_OF_FILE;
uint8_t channel;

#define READY_PRINT_INTERVAL (CLOCK_SECOND * 5)

#if CLI_STANDALONE
PROCESS(cli, "cli app");
AUTOSTART_PROCESSES(&cli);
#endif

static void
print_help(void)
{
  printf("%s\n", CLI_PROJECT);
  printf("cli: version=%s", CLI_VERSION);
  printf("show dag     - - DODAG info\n");
  printf("show neighbor -- neighbor list\n");
  printf("show routes\n");
  printf("show stats\n");
  printf("show channel\n");
  printf("show version\n");
  printf("set channel  -- set [11-26] channel\n");
  printf("set debug  -- select debug info\n");
  printf("i2c       -- probe i2c bus\n");
  printf("help         -- this menu\n");
  printf("repair       -- global dag repair\n");
  printf("upgr         -- reboot via bootloader\n");

  printf("Uptime %lu sec\n", clock_seconds());
}
static uint8_t
radio_get_channel(void)
{
  radio_value_t chan;

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &chan) ==
     RADIO_RESULT_OK) {
    return chan;
  }
  printf("Err get_chan\n");
  return 0;
}
static void
radio_set_channel(uint8_t channel)
{
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel) ==
     RADIO_RESULT_OK) {
    printf("Err set_chan=%d\n", channel);
    return;
  }
}
static const char *
get_nbr_state_name(uint8_t state)
{
  switch(state) {
  case NBR_INCOMPLETE:
    return "INCOMPLETE";
  case NBR_REACHABLE:
    return "REACHABLE";
  case NBR_STALE:
    return "STALE";
  case NBR_DELAY:
    return "DELAY";
  case NBR_PROBE:
    return "PROBE";
  default:
    return "unknown";
  }
}
void
show_dag(void)
{
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  uip_ipaddr_t ipaddr;
  rpl_parent_t *parent;
  rpl_instance_t *end;

  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  /* uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *)addr); */

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance) {
    if(instance->used == 1) {
      parent = rpl_find_parent_any_dag(instance, &ipaddr);
      if(parent != NULL) {

        /* prinf("DAG %u: \n"); */
      }
    }
  }

  dag = rpl_get_any_dag();
  if(dag != NULL) {
    printf("DAG_ID: ");
    uip_debug_ipaddr_print(&dag->dag_id);

    printf("\nDAG: rank:%d version:%d\n", dag->rank, dag->version);
    printf("DAG: grounded:%d preferenc:%d\n", dag->grounded, dag->preference);
    printf("DAG: used:%d joined:%d\n", dag->used, dag->joined);
    printf("DAG: lifetime:%lu\n", dag->lifetime);

#if 0
    pp = dag > preferred_parent;
    if(pp) {
      pp->dag;
      if(ppd) {
        printf("DAG_ID: ");
        uip_debug_ipaddr_print(&dag->dag_id);
      }
    }
#endif

    instance = dag->instance;
    printf("INSTANCE: instance_id: %d used; %d\n",
           instance->instance_id, instance->used);

#ifdef RPL_CONF_STATS
    printf("    dio_sent: %d dio_recv: %d dio_totint: %d\n",
           instance->dio_totsend, instance->dio_totrecv,
           instance->dio_totint);
#endif
  } else {
    printf("No DAG joined\n");
  }
  rpl_print_neighbor_list();
}
void
show_rpl_stats(voiD)
{
#ifdef RPL_CONF_STATS
  printf("rpl.mem_overflows=%-u\n", rpl_stats.mem_overflows);
  printf("rpl.local_repairs=%-u\n", rpl_stats.local_repairs);
  printf("rpl.global_repairs=%-u\n", rpl_stats.global_repairs);
  printf("rpl.malformed_msgs=%-u\n", rpl_stats.malformed_msgs);
  printf("rpl.resets=%-u\n", rpl_stats.resets);
  printf("rpl.parent_switch=%-u\n", rpl_stats.parent_switch);
  printf("rpl.forward_errors=%-u\n", rpl_stats.forward_errors);
  printf("rpl.loop_errors=%-u\n", rpl_stats.loop_errors);
  printf("rpl.root_repairs=%-u\n", rpl_stats.root_repairs);
#endif
}
void
show_routes(void)
{
  uip_ds6_route_t *r;
  uip_ds6_defrt_t *defrt;
  uip_ipaddr_t *ipaddr;
  defrt = NULL;
  if((ipaddr = uip_ds6_defrt_choose()) != NULL) {
    defrt = uip_ds6_defrt_lookup(ipaddr);
  }
  if(defrt != NULL) {
    printf("DefRT: :: -> ");
    uip_debug_ipaddr_print(&defrt->ipaddr);
    if(defrt->isinfinite) {
      printf(" (infinite lifetime)\n");
    } else {
      printf(" lifetime: %lu sec\n", stimer_remaining(&defrt->lifetime));
    }
  } else {
    printf("DefRT: :: -> NULL\n");
  }

  printf("Routes:\n");
  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
    printf(" ");
    uip_debug_ipaddr_print(&r->ipaddr);
    printf(" -> ");
    if(uip_ds6_route_nexthop(r) != NULL) {
      uip_debug_ipaddr_print(uip_ds6_route_nexthop(r));
    } else {
      printf("NULL");
    }
    if(r->state.lifetime < 600) {
      printf(" %ld s\n", r->state.lifetime);
    } else {
      printf(" >600 s\n");
    }
  }
  printf("\n");
}
void
show_neighbors(void)
{
  uip_ds6_nbr_t *nbr;
  rpl_instance_t *def_instance;
  const uip_lladdr_t *lladdr;
  rpl_parent_t *p;
  uint16_t rank;
  const struct link_stats *ls;

  def_instance = rpl_get_default_instance();

  printf("Neighbors IPv6         \t  Sec  State       Rank      RPL Flg   ETX  RSSI Freshness\n");
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
    uip_debug_ipaddr_print(&nbr->ipaddr);
    lladdr = uip_ds6_nbr_get_ll(nbr);
    p = nbr_table_get_from_lladdr(rpl_parents, (const linkaddr_t *)lladdr);
    rank = p != NULL ? p->rank : 0;
#if UIP_ND6_SEND_NS || UIP_ND6_SEND_RA
    if(stimer_expired(&nbr->reachable)) {
      printf("\t %5c ", '-');
    } else {
      printf("\t %5lu ", stimer_remaining(&nbr->reachable));
    }
#else
    printf("\t  N/A  ");
#endif
    printf("%-10s ", get_nbr_state_name(nbr->state));
    printf(" %d.%02d[%3d] %c",
           rank / 128, (int)(rank * 100L / 128) % 100, rank,
           p != NULL ? 'P' : ' ');
    if(def_instance != NULL && def_instance->current_dag != NULL &&
       def_instance->current_dag->preferred_parent == p) {
      printf("*");
    } else {
      printf(" ");
    }
    if(p != NULL) {
      printf(" %2x", p->flags);
    } else {
      printf("   ");
    }
    lladdr = uip_ds6_nbr_get_ll(nbr);
    ls = link_stats_from_lladdr((const linkaddr_t *)lladdr);
    printf("     %-4u  %-2u    %-u", ls->etx, ls->rssi, ls->freshness);
    printf("\n");
  }
  printf("\n");
}
void
show_net_all(void)
{
  show_dag();
  show_routes();
  show_neighbors();
}
static int
cmd_chan(uint8_t verbose)
{
  uint8_t tmp;
  char *p = strtok(NULL, delim);

  if(p) {
    tmp = atoi((char *)p);
    if(tmp >= 11 && tmp <= 26) {
      channel = tmp;
      radio_set_channel(channel);
    } else {
      printf("Invalid chan=%d\n", tmp);
      return 0;
    }
  }
  if(verbose) {
    printf("chan=%d\n", radio_get_channel());
  }
  return 1;
}
void
debug_cmd(char *p)
{
  printf("To be added\n");
}
void
handle_serial_input(const char *line)
{
  char *p;
  /* printf("in: '%s'\n", line); */
  p = strtok((char *)&line[0], (const char *)delim);

  if(!p) {
    return;
  }

  printf("\n");

  /* Show commands */
  if(!strcmp(p, "sh") || !strcmp(p, "sho") || !strcmp(p, "show")) {
    p = strtok(NULL, (const char *)delim);
    if(p) {
      if(!strcmp(p, "d") || !strcmp(p, "da") || !strcmp(p, "dag")) {
        show_dag();
      } else if(!strcmp(p, "s") || !strcmp(p, "stat") || !strcmp(p, "stats")) {
        show_rpl_stats();
      } else if(!strcmp(p, "r") || !strcmp(p, "ro") || !strcmp(p, "route")) {
        show_routes();
      } else if(!strcmp(p, "n") || !strcmp(p, "ne") || !strcmp(p, "neigh")) {
        show_neighbors();
      } else if(!strcmp(p, "ch") || !strcmp(p, "chan")) {
        cmd_chan(1);
      } else if(!strcmp(p, "v") || !strcmp(p, "ver")) {
        printf("%s", CLI_VERSION);
      }
    }
  }
  /* Set commands */
  else if(!strcmp(p, "sh") || !strcmp(p, "sho") || !strcmp(p, "show")) {
    p = strtok(NULL, (const char *)delim);
    if(p) {
      if(!strcmp(p, "de") || !strcmp(p, "debug")) {
        debug_cmd(p);
      }
    }
  }
  /* Misc commands */
  else if(!strcmp(p, "li") || !strcmp(p, "list")) {
    show_net_all();
  }

#ifdef CONTIKI_TARGET_AVR_RSS2
  else if(!strcmp(p, "i2c")) {
    printf("I2C: ");
    i2c_probed = i2c_probe();
    printf("\n");
  } else if(!strcmp(p, "upgr") || !strcmp(p, "upgrade")) {
    printf("OK\n");
    printf("%c", eof);
    cli();
    wdt_enable(WDTO_15MS);
    while(1) ;
  }
#endif
  else if(!strcmp(p, "rep") || !strcmp(p, "repair")) {
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  } else if(!strcmp(p, "help") || !strcmp(p, "h")) {
    print_help();
  } else { printf("Illegal command '%s'\n", p);
  }
  printf(CLI_COMMAND_PROMPT);
}
#ifdef CLI_STANDALONE

static struct etimer periodic;

PROCESS_THREAD(cli, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND);

  for(;;) {
    PROCESS_WAIT_EVENT();

    /* printf("event %u (%u) at %u, data %p\n", (uint16_t)ev, (uint16_t)serial_line_event_message, currentState, data); */
    printf("event %u (%u) at  data %p\n", (uint16_t)ev, (uint16_t)serial_line_event_message, data);

    if(etimer_expired(&periodic)) {
      etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
    if(ev == PROCESS_EVENT_POLL) {
      etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
  }
  PROCESS_END();
}

#endif
