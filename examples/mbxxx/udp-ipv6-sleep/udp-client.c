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

#include "board-sensors.h"
#include "board.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define SEND_INTERVAL		25 * CLOCK_SECOND

/* Seconds during which the system must be in deep sleep.
 * During deep sleep all the OS is frozen. */
#define SLEEP_INTERVAL_SECONDS		6
/* System does not go to sleep until AWAKE_INTERVAL is passed.
 * This is done to let IP stack receive Neigbhor Advertisements
 * and eventual response packets from the server. */
#define AWAKE_INTERVAL		3 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

static struct uip_udp_conn *client_conn;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    printf("Response from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  printf("Client sending to: ");
  PRINT6ADDR(&client_conn->ripaddr);
  sprintf(buf, "Hello %d from the client", ++seq_id);
  printf(" (msg: %s)\n", buf);
#if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
  uip_udp_packet_send(client_conn, buf, UIP_APPDATA_SIZE);
#else /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
  uip_udp_packet_send(client_conn, buf, strlen(buf));
#endif /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
#if UIP_CONF_ROUTER
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}
#endif /* UIP_CONF_ROUTER */
/*---------------------------------------------------------------------------*/
static void
set_connection_address(uip_ipaddr_t *ipaddr)
{
#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)
#ifdef UDP_CONNECTION_ADDR
  if(uiplib_ipaddrconv(QUOTEME(UDP_CONNECTION_ADDR), ipaddr) == 0) {
    PRINTF("UDP client failed to parse address '%s'\n", QUOTEME(UDP_CONNECTION_ADDR));
  }
#elif UIP_CONF_ROUTER
  uip_ip6addr(ipaddr,UIP_DS6_DEFAULT_PREFIX,0,0,0,0x0280,0xe102,0x0000,0x008a);
#else
  uip_ip6addr(ipaddr,0xfe80,0,0,0,0x0280,0xe102,0x0000,0x008a);
#endif /* UDP_CONNECTION_ADDR */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et, wake_timer, periodic_timer;
  uip_ipaddr_t ipaddr;

  PROCESS_BEGIN();
  PRINTF("UDP client process started\n");

#if UIP_CONF_ROUTER
  set_global_address();
#endif

  print_local_addresses();

  set_connection_address(&ipaddr);

  /* new connection with remote host */
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  etimer_set(&et, CLOCK_SECOND*10);
  PROCESS_WAIT_UNTIL(etimer_expired(&et)); // Wait for DAD and Router Discovery procedure to end.

  etimer_set(&et, SEND_INTERVAL);
  etimer_set(&wake_timer, AWAKE_INTERVAL);
  etimer_set(&periodic_timer, 1);

  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&wake_timer)){  // if timer hasn't expired do not go in deep sleep, in order to receive a response.
		printf("Sleeping...\r\n");
		halBoardPowerDown();//sensorsPowerDown();
		sleep_seconds(SLEEP_INTERVAL_SECONDS); // Put system in deep sleep mode for a while.
		halBoardPowerUp();//sensorsPowerUp();
		printf("Awake\r\n");
    }
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
      etimer_restart(&wake_timer);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }

    /* Make the process be called almost immediately,
     * so that it can force the system to go into deep sleep. */
    etimer_restart(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
