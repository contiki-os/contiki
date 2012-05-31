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

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"

#if CONTIKI_TARGET_SENSINODE
#include "dev/sensinode-sensors.h"
#include "debug.h"
#else
#define putstring(s)
#endif

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

#define SERVER_REPLY          1

/* Should we act as RPL root? */
#define SERVER_RPL_ROOT       1

#if SERVER_RPL_ROOT
static uip_ipaddr_t ipaddr;
#endif
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor adc_sensor;
/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  memset(buf, 0, MAX_PAYLOAD_LEN);
  if(uip_newdata()) {
    leds_on(LEDS_RED);
    len = uip_datalen();
    memcpy(buf, uip_appdata, len);
    PRINTF("%u bytes from [", len);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("]:%u", UIP_HTONS(UIP_UDP_BUF->srcport));
    PRINTF(" V=%u", *buf);
    PRINTF(" I=%u", *(buf + 1));
    PRINTF(" T=%u", *(buf + 2));
    PRINTF(" Val=%u\n", *(uint16_t *)(buf + 3));
#if SERVER_REPLY
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;

    uip_udp_packet_send(server_conn, buf, len);
    /* Restore server connection to allow data from any node */
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
#endif
  }
  leds_off(LEDS_RED);
  PRINTF("sent\n");
  return;
}
/*---------------------------------------------------------------------------*/
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON && (DEBUG==DEBUG_PRINT))
static void
print_stats()
{
  PRINTF("tl=%lu, ts=%lu, bs=%lu, bc=%lu\n",
      rimestats.toolong, rimestats.tooshort, rimestats.badsynch, rimestats.badcrc);
  PRINTF("llrx=%lu, lltx=%lu, rx=%lu, tx=%lu\n",
      rimestats.llrx, rimestats.lltx, rimestats.rx, rimestats.tx);
}
#else
#define print_stats()
#endif
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state
        == ADDR_PREFERRED)) {
      PRINTF("  ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
#if SERVER_RPL_ROOT
void
create_dag()
{
  rpl_dag_t *dag;

  uip_ip6addr(&ipaddr, 0x2001, 0x630, 0x301, 0x6453, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  print_local_addresses();

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
  if(dag != NULL) {
    uip_ip6addr(&ipaddr, 0x2001, 0x630, 0x301, 0x6453, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("Created a new RPL dag with ID: ");
    PRINT6ADDR(&dag->dag_id);
    PRINTF("\n");
  }
}
#endif /* SERVER_RPL_ROOT */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  static const struct sensors_sensor *b1;
  static const struct sensors_sensor *b2;
#endif

  PROCESS_BEGIN();
  putstring("Starting UDP server\n");

#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  putstring("Button 1: Print RIME stats\n");
  putstring("Button 2: Reboot\n");
#endif

#if SERVER_RPL_ROOT
  create_dag();
#endif

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
  b1 = sensors_find(BUTTON_1_SENSOR);
  b2 = sensors_find(BUTTON_2_SENSOR);
#endif

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
#if (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON)
    } else if(ev == sensors_event && data != NULL) {
      if(data == b1) {
        print_stats();
      } else if(data == b2) {
        watchdog_reboot();
      }
#endif /* (CONTIKI_TARGET_SENSINODE && BUTTON_SENSOR_ON) */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
