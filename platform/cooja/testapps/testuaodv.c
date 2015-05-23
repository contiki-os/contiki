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
 */

#include <stdlib.h>
#include "net/ip/uip.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#include "net/uaodv.h"
#include "net/ipv4/uaodv-rt.h"

#include <stdio.h>

#define COOJA_PORT 1234

PROCESS(test_uaodv_process, "uIP uAODV test process");
AUTOSTART_PROCESSES(&uaodv_process, &test_uaodv_process);

static struct uip_udp_conn *out_conn;
static struct uip_udp_conn *in_conn;
/*---------------------------------------------------------------------*/
PROCESS_THREAD(test_uaodv_process, ev, data)
{
  static uip_ipaddr_t addr;

  PROCESS_BEGIN();

  printf("uIP uAODV test process started\n");

  uip_ipaddr(&addr, 0,0,0,0);
  in_conn = udp_new(&addr, UIP_HTONS(0), NULL);
  uip_udp_bind(in_conn, UIP_HTONS(COOJA_PORT));

  uip_ipaddr(&addr, 10,10,10,4);
  out_conn = udp_new(&addr, UIP_HTONS(COOJA_PORT), NULL);

  button_sensor.configure(SENSORS_ACTIVE, 1);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == sensors_event && data == &button_sensor) {
      struct uaodv_rt_entry *route;

      uip_ipaddr(&addr, 10,10,10,4);
      route = uaodv_rt_lookup_any(&addr);
      if (route == NULL || route->is_bad) {
        printf("%d.%d.%d.%d: lookup %d.%d.%d.%d\n", uip_ipaddr_to_quad(&uip_hostaddr), uip_ipaddr_to_quad(&addr));
        uaodv_request_route_to(&addr);
      } else {
        printf("%d.%d.%d.%d: send to %d.%d.%d.%d\n", uip_ipaddr_to_quad(&uip_hostaddr), uip_ipaddr_to_quad(&addr));
        tcpip_poll_udp(out_conn);
        PROCESS_WAIT_UNTIL(ev == tcpip_event && uip_poll());
        uip_send("cooyah COOJA", 12);
      }
    }

    if(ev == tcpip_event && uip_newdata()) {
      ((char*) uip_appdata)[uip_datalen()] = 0;
      printf("data received from %d.%d.%d.%d: %s\n",
          uip_ipaddr_to_quad(&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->srcipaddr),
          (char *)uip_appdata);
      leds_toggle(LEDS_ALL);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
