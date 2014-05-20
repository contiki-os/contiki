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
 *         Example file using ORPL for a any-to-any routing: the root and all
 *         nodes with an even node-id send a ping periodically to another node
 *         (which also must be root or have even node-id). Upon receiving a ping,
 *         nodes answer with a poing.
 *         Enables logging as used in the ORPL SenSyS'13 paper.
 *         Can be deployed in the Indriya or Twist testbeds.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include "lib/random.h"
#include "orpl.h"
#include "orpl-routing-set.h"
#include "deployment.h"
#include "simple-energest.h"
#include "simple-udp.h"
#include "cc2420.h"
#include <stdio.h>

#define SEND_INTERVAL   (60 * CLOCK_SECOND)
#define UDP_PORT 1234

static struct simple_udp_connection unicast_connection;
static uint16_t current_cnt = 0;

static const uint16_t any_to_any_list[] = {
#if IN_INDRIYA
  20, 12, 28, 50, 56, 72, 92, 94, 112,
#elif IN_COOJA
  1, 2, 4, 6, 8,
#endif
  0
};

static int
is_id_in_any_to_any(uint16_t id)
{
  const uint16_t *curr = any_to_any_list;
  while(*curr != 0) {
    if(*curr == id) {
      return 1;
    }
    curr++;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "ORPL -- Any-to-any Application");
AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/
void app_send_to(uint16_t id, int ping, uint32_t seqno);
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *dataptr,
         uint16_t datalen)
{
  struct app_data data;
  appdata_copy(&data, (struct app_data*)dataptr);
  if(data.ping) {
    ORPL_LOG_FROM_APPDATAPTR((struct app_data *)dataptr, "App: received ping");
  } else {
    ORPL_LOG_FROM_APPDATAPTR((struct app_data *)dataptr, "App: received pong");
  }
  if(data.ping) {
    app_send_to(data.src, 0, data.seqno | 0x8000l);
  }
}
/*---------------------------------------------------------------------------*/
void
app_send_to(uint16_t id, int ping, uint32_t seqno)
{
  struct app_data data;
  uip_ipaddr_t dest_ipaddr;

  data.seqno = seqno;
  data.src = node_id;
  data.dest = id;
  data.hop = 0;
  data.fpcount = 0;
  data.ping = ping;

  if(ping) {
    ORPL_LOG_FROM_APPDATAPTR(&data, "App: sending ping");
  } else {
    ORPL_LOG_FROM_APPDATAPTR(&data, "App: sending pong");
  }

  orpl_set_curr_seqno(data.seqno);
  set_ipaddr_from_id(&dest_ipaddr, id);

  simple_udp_sendto(&unicast_connection, &data, sizeof(data), &dest_ipaddr);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t global_ipaddr;

  PROCESS_BEGIN();

  if(node_id == 0) {
    NETSTACK_RDC.off(0);
    printf("Node id unset, my mac is ");
    uip_debug_lladdr_print(&rimeaddr_node_addr);
    printf("\n");
    PROCESS_EXIT();
  }

  cc2420_set_txpower(RF_POWER);
  cc2420_set_cca_threshold(RSSI_THR);
  simple_energest_start();

  printf("App: %u starting\n", node_id);

  deployment_init(&global_ipaddr);
  orpl_init(&global_ipaddr, node_id == ROOT_ID, 0);
  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  printf("App: %u starting\n", node_id);

  if(node_id == ROOT_ID) {
    NETSTACK_RDC.off(1);
  } else if(is_id_in_any_to_any(get_node_id())) {
    etimer_set(&periodic_timer, 2 * 60 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_set(&periodic_timer, SEND_INTERVAL);

    while(1) {
      uint16_t target_id;

      etimer_set(&send_timer, random_rand() % (SEND_INTERVAL));
      PROCESS_WAIT_UNTIL(etimer_expired(&send_timer));

      do {
        target_id = get_node_id_from_index((random_rand()>>8)%get_n_nodes());
      } while (target_id == node_id || !is_id_in_any_to_any(target_id));

      if(target_id < node_id || target_id == ROOT_ID) {
        /* After finding an addressable node, send only if destination has lower ID
         * otherwise, next attempt will be at the next period */
        app_send_to(target_id, 1, ((uint32_t)node_id << 16) + current_cnt);
        current_cnt++;
      }

      PROCESS_WAIT_UNTIL(etimer_expired(&periodic_timer));
      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
