/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/netstack.h"

#include "dev/serial-line.h"

#include "net/ip/uip.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "net/rime/rime.h"

#include "uart.h"
#include "watchdog.h"

SENSORS(&button_sensor);

#ifndef SERIAL_ID
#define SERIAL_ID { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }
#endif

uint8_t serial_id[] = SERIAL_ID;
uint16_t node_id = 0x0102;

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(linkaddr_t));
#if NETSTACK_CONF_WITH_IPV6
  memcpy(addr.u8, serial_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(linkaddr_t); ++i) {
      addr.u8[i] = serial_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  linkaddr_set_node_addr(&addr);
  printf("Rime started with address ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%d.", addr.u8[i]);
  }
  printf("%d\n", addr.u8[i]);
}
/*---------------------------------------------------------------------------*/
static void
set_rf_params(void)
{
  int chan;
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, RF_CHANNEL);
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &chan);
  printf("RF channel set to %d Hz\n", chan);
}
/*---------------------------------------------------------------------------*/
int contiki_argc = 0;
char **contiki_argv;

int
main(int argc, char **argv)
{
  watchdog_init();
  leds_init();
  uart_init(115200);
  clock_init();

#if NETSTACK_CONF_WITH_IPV6
#if UIP_CONF_IPV6_RPL
  printf(CONTIKI_VERSION_STRING " started with IPV6, RPL\n");
#else
  printf(CONTIKI_VERSION_STRING " started with IPV6\n");
#endif
#else
  printf(CONTIKI_VERSION_STRING " started\n");
#endif

  contiki_argc = argc;
  contiki_argv = argv;

  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();
  rtimer_init();

  set_rime_addr();

  queuebuf_init();

  set_rf_params();
  netstack_init();
  printf("MAC %s RDC %s NETWORK %s\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name, NETSTACK_NETWORK.name);

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, serial_id, sizeof(uip_lladdr.addr));

  process_start(&tcpip_process, NULL);
  printf("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 8; i++) {
      printf("%02x%02x%c", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1],
             i == 7 ? '\n' : ':');
    }
    /* make it hardcoded... */
    lladdr->state = ADDR_AUTOCONF;
  }
#elif NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif

  serial_line_init();
  process_start(&sensors_process, NULL);

  autostart_start(autostart_processes);

  while(1) {
    watchdog_periodic();

    process_run();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  printf("%s\n", m);
}
/*---------------------------------------------------------------------------*/
void
_xassert(const char *file, int line)
{
  printf("%s:%u: failed assertion\n", file, line);
  for(;;) {
    continue;
  }
}
