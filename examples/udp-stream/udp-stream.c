/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *         An example sending a UDP stream
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "uip.h"
#include "net/rpl/rpl.h"
#include "sys/node-id.h"
#include "servreg-hack.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"

#define SINK_ID         1
#define SENDER_ID       5
#define DATASIZE			  73
#define STREAMLEN			  QUEUEBUF_CONF_NUM
#define UDP_PORT        9876
#define SERVICE_ID      190

struct msg {
  uint8_t streamno;
  uint8_t seqno;
  uint8_t buf[DATASIZE];
};

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(udpstream_process, "UDP Stream Process");
AUTOSTART_PROCESSES(&udpstream_process);

/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
static void
create_rpl_dag(uip_ipaddr_t *ipaddr)
{
  struct uip_ds6_addr *root_if;
  root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    uip_ipaddr_t prefix;

    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &prefix, 64);
    printf("created a new RPL dag\n");
  } else {
    printf("failed to create a new RPL DAG\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  static int cpt;
  cpt++;
  if(cpt%128==0) {
    printf("Received %d datagrams\n", cpt);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_stream(uip_ipaddr_t *addr, uint16_t streamno)
{
  int i;
  int seqno = 0;
  struct msg msg;

  memset(&msg, 0xaa, sizeof(msg));
  msg.streamno = streamno;
  for(i=0; i<STREAMLEN; i++) {
    msg.seqno = ++seqno;
    simple_udp_sendto(&unicast_connection, (char*)&msg, sizeof(msg), addr);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udpstream_process, ev, data)
{
  static uint16_t streamno;
  static struct etimer et;
  static uip_ipaddr_t *ipaddr;

  PROCESS_BEGIN();
  PROCESS_PAUSE();

  printf("Formatting Coffee FS...\n");
  cfs_coffee_format();
  printf("done.\n");
  /* We need re-initialize queuebuf after formatting */
  queuebuf_init();

  /* Start service registration */
  servreg_hack_init();
  ipaddr = set_global_address();

  if(node_id == SINK_ID) {
    /* The sink creates a dag and waits for UDP datagrams */
    create_rpl_dag(ipaddr);
    servreg_hack_register(SERVICE_ID, ipaddr);
    simple_udp_register(&unicast_connection, UDP_PORT,
                        NULL, UDP_PORT, receiver);
    while(1) {
      PROCESS_WAIT_EVENT();
    }
  } else if(node_id == SENDER_ID) {
    /* The sender looks for the sink and sends UDP streams */
    ipaddr = NULL;
    simple_udp_register(&unicast_connection, UDP_PORT,
                          NULL, UDP_PORT, receiver);

    etimer_set(&et, 10*CLOCK_SECOND);
    etimer_restart(&et);

    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      if(ipaddr != NULL) {
        streamno++;
        send_stream(ipaddr, streamno);
      } else {
        ipaddr = servreg_hack_lookup(SERVICE_ID);
        if(ipaddr != NULL) {
          etimer_set(&et, 2*CLOCK_SECOND);
          printf("Streaming to ");
          uip_debug_ipaddr_print(ipaddr);
          printf("\n");
        } else {
          printf("Service %d not found\n", SERVICE_ID);
        }
      }
      etimer_restart(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
