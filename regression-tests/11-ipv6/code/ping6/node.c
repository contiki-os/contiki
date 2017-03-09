/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"

#include "serial-shell.h"
#include "shell-ping.h"

#if WITH_TSCH || WITH_RPL
#include "button-sensor.h"
#endif

#if WITH_RPL
#include "net/rpl/rpl.h"
#endif

#define DEBUG 1
#include "net/ip/uip-debug.h"

#if DEBUG == 0
#error DEBUG must be more than zero
#endif

#include <stdio.h>

PROCESS(node_process, "Node");
AUTOSTART_PROCESSES(&node_process);

#if WITH_TSCH
char banner[] = "TSCH";
#elif WITH_IEEE802154E_2012
char banner[] = "CSMA + IEEE 802.15.4e-2012";
#elif WITH_RPL
char banner[] = "CSMA + RPL";
#else
char banner[] = "CSMA";
#endif
char ready[] = "ready for test";

static void
callback(uip_ipaddr_t *src, uint8_t t, uint8_t *d, uint16_t l)
{
  uip_ds6_nbr_t *nbr;
  const uip_lladdr_t *lladdr;

  if(src == NULL) {
    return;
  }
  nbr = uip_ds6_nbr_lookup(src);
  if(nbr != NULL) {
    lladdr = uip_ds6_nbr_get_ll(nbr);
  } else {
    lladdr = NULL;
  }
  PRINTF("post-ping: uip_ds6_nbr_get_ll() returns ");
  if(lladdr == NULL) {
    PRINTF("null");
  } else {
    PRINTLLADDR(lladdr);
  }
  PRINTF(" for ");
  PRINT6ADDR(src);
  PRINTF("\n");

  process_poll(&node_process);
}

PROCESS_THREAD(node_process, ev, data)
{
#if WITH_TSCH || WITH_RPL
  static struct etimer et;
#endif
#if WITH_RPL
  uip_ds6_addr_t *addr;
#else
  uip_ip6addr_t prefix;
#endif
  static struct uip_icmp6_echo_reply_notification notification;

  PROCESS_BEGIN();

  printf("%s node\n", banner);

  serial_shell_init();
  shell_ping_init();
  uip_icmp6_echo_reply_callback_add(&notification, callback);

#if !WITH_RPL
  /* need to install prefix for on-link determination */
  uiplib_ip6addrconv("fd00::", &prefix);
  uip_ds6_prefix_add((uip_ipaddr_t *)&prefix, 64,
                     0, 0, 0, 0);
#endif

#if WITH_TSCH || WITH_RPL
  SENSORS_ACTIVATE(button_sensor);
  etimer_set(&et, CLOCK_SECOND);

  PROCESS_YIELD_UNTIL((ev == sensors_event &&
                       data == &button_sensor) ||
                      etimer_expired(&et));

  if(etimer_expired(&et) == 0) {
#if WITH_TSCH
    /* button was pushed; set it as a coordinator */
    printf("become a TSCH coordinator\n");
    tsch_set_coordinator(1);
#elif WITH_RPL
    /* button was pushed; set it as a RPL DODAG root */
    addr = uip_ds6_get_global(ADDR_PREFERRED);
    if(addr == NULL) {
      printf("fatal error\n");
    } else {
      printf("become a RPL DODAG root\n");
      rpl_set_root(RPL_DEFAULT_INSTANCE, &addr->ipaddr);
  }
#endif
    printf("%s\n", ready);
  } else {
    /* node needs to associate with the coordinator */
    while(1) {
      etimer_reset(&et);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
#if WITH_TSCH
      if(tsch_queue_get_time_source() != NULL) {
        printf("%s\n", ready);
        break;
      }
#elif WITH_RPL
      if(rpl_get_any_dag() != NULL) {
        printf("%s\n", ready);
        break;
      }
#endif
    }
  }
#else
  printf("%s\n", ready);
#endif /* WITH_TSCH */

  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

  PROCESS_END();
}
