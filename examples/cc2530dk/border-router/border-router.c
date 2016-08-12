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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "dev/watchdog.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "dev/radio.h"
#include "debug.h"

static uint8_t prefix_set;

#if DEBUG
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#define PUTBIN(...) putbin(__VA_ARGS__)
#define PUTDEC(...) putdec(__VA_ARGS__)
#define PUTCHAR(...) putchar(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#define PUTBIN(...)
#define PUTDEC(...)
#define PUTCHAR(...)
#endif
/*---------------------------------------------------------------------------*/
PROCESS(border_router_process, "Border Router process");
AUTOSTART_PROCESSES(&border_router_process);
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void) CC_NON_BANKED
{
  int i;
  uint8_t state;

  PUTSTRING("Router's IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state
        == ADDR_PREFERRED)) {
      PUTSTRING("  ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PUTCHAR('\n');
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
request_prefix(void) CC_NON_BANKED
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
/* Set our prefix when we receive one over SLIP */
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  /* Become root of a new DODAG with ID our global v6 address */
  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
  if(dag != NULL) {
    rpl_set_prefix(dag, &ipaddr, 64);
    PUTSTRING("Created a new RPL dag with ID: ");
    PRINT6ADDR(&dag->dag_id);
    PUTCHAR('\n');
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_rf_channel(void)
{
  radio_value_t chan;

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &chan);

  return (uint8_t)chan;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  PUTSTRING("Border Router started\n");
  prefix_set = 0;

  leds_on(LEDS_GREEN);

  /* Request prefix until it has been received */
  while(!prefix_set) {
    leds_on(LEDS_RED);
    PUTSTRING("Prefix request.\n");
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    leds_off(LEDS_RED);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  /* We have created a new DODAG when we reach here */
  PUTSTRING("On Channel ");
  PUTDEC(get_rf_channel());
  PUTCHAR('\n');

  print_local_addresses();

  leds_off(LEDS_GREEN);

  PROCESS_EXIT();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
