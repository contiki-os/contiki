/*
 * Copyright (c) 2014, Thingsquare, www.thingsquare.com.
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


#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ip/uip-debug.h"

#include <stdio.h>
#include <string.h>

#define PING_INTERVAL CLOCK_SECOND * 4

/*---------------------------------------------------------------------------*/
PROCESS(pinger_node_process, "Pinger node process");
AUTOSTART_PROCESSES(&pinger_node_process);
/*---------------------------------------------------------------------------*/
static const uip_ipaddr_t *
get_parent(void)
{
  rpl_dag_t *dag;

  dag = rpl_get_any_dag();
  if(dag != NULL && dag->preferred_parent != NULL) {
    return rpl_get_parent_ipaddr(dag->preferred_parent);
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
echo_reply_callback(uip_ipaddr_t *sender,
                   uint8_t ttl, uint8_t *data, uint16_t datalen)
{
  printf("Got ping reply\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pinger_node_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer ping_timer;
  static struct uip_icmp6_echo_reply_notification n;
  const uip_ipaddr_t *parent;

  PROCESS_BEGIN();

  uip_icmp6_echo_reply_callback_add(&n, echo_reply_callback);

  etimer_set(&periodic_timer, PING_INTERVAL);
  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    parent = get_parent();
    if(parent != NULL) {
      uip_ds6_nbr_t *n = uip_ds6_nbr_lookup(parent);
      if(n != NULL) {
        const uip_lladdr_t *lladdr;
        int metric;
        lladdr = uip_ds6_nbr_get_ll(n);
        if(lladdr != NULL) {
          metric = rpl_get_parent_link_metric(lladdr);
          printf("Pinging parent ");
          uip_debug_ipaddr_print(parent);
          printf(" with etx %d\n", metric);
          memset(uip_buf, 0, UIP_BUFSIZE);
          uip_icmp6_send(parent,
                         ICMP6_ECHO_REQUEST, 0, 10);
        }
      }

    } else {
      printf("No parent\n");
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
