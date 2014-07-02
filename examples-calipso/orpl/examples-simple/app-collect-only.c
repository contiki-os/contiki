/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *         Example file using ORPL for a data collection.
 *         Enables logging as used in the ORPL SenSyS'13 paper.
 *         Can be deployed in the Indriya or Twist testbeds.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include "lib/random.h"
#include "orpl.h"
#include "simple-udp.h"
#include "cc2420.h"
#include "node-id.h"
#include <string.h>
#include <stdio.h>

#define SEND_INTERVAL   (60 * CLOCK_SECOND)
#define UDP_PORT 1234
#define ROOT_ID 1
uip_ipaddr_t root_ipaddr;

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "ORPL -- Collect-only Application");
AUTOSTART_PROCESSES(&unicast_sender_process);
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
  printf("App: received 0x%lx\n", *((uint32_t*)data));
}
/*---------------------------------------------------------------------------*/
void app_send_to(uint16_t id) {

  static unsigned int cnt;
  uint32_t seqno = ((uint32_t)node_id << 16) + cnt;

  printf("App: sending 0x%lx\n", seqno);

  simple_udp_sendto(&unicast_connection, &seqno, sizeof(seqno), &root_ipaddr);

  cnt++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t global_ipaddr;

  PROCESS_BEGIN();

  printf("App: %u starting\n", node_id);

  uip_ip6addr(&root_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);

  if(node_id == ROOT_ID) {
    memcpy(&global_ipaddr, &root_ipaddr, 16);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_MANUAL);
    rpl_dag_t *dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(dag, &global_ipaddr, 64);
  } else {
    uip_ip6addr(&global_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
  }

  orpl_init(&global_ipaddr, node_id == ROOT_ID, 1);
  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  if(node_id == ROOT_ID) {
    NETSTACK_RDC.off(1);
  } else {
    etimer_set(&periodic_timer, SEND_INTERVAL);
    while(1) {
      etimer_set(&send_timer, random_rand() % (SEND_INTERVAL));
      PROCESS_WAIT_UNTIL(etimer_expired(&send_timer));

      if(orpl_current_edc() != 0xffff) {
        app_send_to(ROOT_ID);
      } else {
        printf("App: not in DODAG\n");
      }

      PROCESS_WAIT_UNTIL(etimer_expired(&periodic_timer));
      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
