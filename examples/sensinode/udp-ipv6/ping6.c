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
#include <stdio.h>

#if CONTIKI_TARGET_SENSINODE
#include "dev/sensinode-sensors.h"
#include "debug.h"
#endif

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static struct etimer ping6_periodic_timer;
static uint8_t count = 0;
static uip_ipaddr_t dest_addr;

PROCESS(ping6_process, "PING6 process");
/*---------------------------------------------------------------------------*/
static void
ping6handler()
{
  if(count < PING6_NB) {
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
        count, PING6_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
    UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


    PRINTF("Echo Request to ");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF(" from ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");
    UIP_STAT(++uip_stat.icmp.sent);

    tcpip_ipv6_output();

    count++;
    etimer_set(&ping6_periodic_timer, 3 * CLOCK_SECOND);
  } else {
    count = 0;
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping6_process, ev, data)
{
  
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  static struct sensors_sensor * btn;
#endif

  PROCESS_BEGIN();
  PRINTF("ping6 running.\n");
  PRINTF("Button 1: 5 pings 16 byte payload.\n");

  uip_ip6addr(&dest_addr,0x2001,0x470,0x55,0,0x0215,0x2000,0x0002,0x0302);
  count = 0;
 
  /* Check if we have buttons */
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  btn = sensors_find(BUTTON_1_SENSOR);
#endif

  while(1) {
    PROCESS_YIELD();

#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
    if(ev == sensors_event) {
      if(data == btn && count == 0) {
        ping6handler();
      }
    }
#endif
    if(etimer_expired(&ping6_periodic_timer)) {
      ping6handler();
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
