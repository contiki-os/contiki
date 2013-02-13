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
#include "net/rpl/rpl.h"
#include "net/uip.h"

#include <string.h>

#define DEBUG DEBUG_FULL
#include "net/uip-debug.h"

#define SEND_INTERVAL    15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN    40

static struct uip_udp_conn *client_conn = NULL;
extern uip_ds6_prefix_t uip_ds6_prefix_list[];
uip_ip6addr_t *dest_addr;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
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
char *
add_ipaddr(char * buf, const uip_ipaddr_t *addr)
{
  uint16_t a;
  unsigned int i;
  int f;
  char *p = buf;

  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        p += sprintf(p, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        p += sprintf(p, ":");
      }
      p += sprintf(p, "%04x", a);
    }
  }
  return p;
}

static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];
  int i;
  rpl_dag_t *dag = rpl_get_any_dag();
  uip_ipaddr_t *globaladdr = NULL;
  uip_ipaddr_t newdest_addr;

  if((globaladdr = &uip_ds6_get_global(-1)->ipaddr) != NULL) {
    uip_ipaddr_copy(&newdest_addr, globaladdr);
    memcpy(&newdest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2);
  }

  if (client_conn == NULL) {
    if (dag != NULL) {
      /* new connection with remote host */
      /* Determine the correct IP to send packets */
      if(dag->prefix_info.length == 0) { //No prefix announced : link-local address used
        PRINTF("UDP-CLIENT : no prefix announced yet, DODAGID used as destination\n");
        client_conn = udp_new(&dag->dag_id, UIP_HTONS(3000), NULL);
      } else {
        /* At least one prefix announced : building of the global address to reach using prefixes */
        memcpy(dest_addr, &newdest_addr, sizeof(uip_ipaddr_t));
        PRINTF("UDP-CLIENT: address destination: ");
        PRINT6ADDR(dest_addr);
        PRINTF("\n");
        client_conn = udp_new(dest_addr, UIP_HTONS(3000), NULL);
      }
      udp_bind(client_conn, UIP_HTONS(3001));
      PRINTF("Created a connection with the server ");
      PRINT6ADDR(&client_conn->ripaddr);
      PRINTF(" local/remote port %u/%u\n",
      UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
      print_local_addresses();
    } else {
      PRINTF("No dag configured yet\n");
    }
  } else {
    if(memcmp(&client_conn->ripaddr, &newdest_addr, sizeof(uip_ipaddr_t)) != 0) {
      if(dag->prefix_info.length != 0) {
        PRINTF("UPD-CLIENT : new prefix announced, connection changed\n");
        memcpy(dest_addr, &newdest_addr, sizeof(uip_ipaddr_t));
        client_conn = udp_new(dest_addr, UIP_HTONS(3000), NULL);
        udp_bind(client_conn, UIP_HTONS(3001));
        PRINTF("Created a connection with the server ");
        PRINT6ADDR(&client_conn->ripaddr);
        PRINTF(" local/remote port %u/%u\n",
        UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
        print_local_addresses();
      }
    }
    PRINTF("Client sending to: ");
    PRINT6ADDR(&client_conn->ripaddr);
    i = sprintf(buf, "%d | ", ++seq_id);
    add_ipaddr(buf + i, &dag->instance->def_route->ipaddr);
    PRINTF(" (msg: %s)\n", buf);
    #if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
    uip_udp_packet_send(client_conn, buf, UIP_APPDATA_SIZE);
    #else /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
    uip_udp_packet_send(client_conn, buf, strlen(buf));
    #endif /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  PRINTF("UDP client process started\n");
  dest_addr = malloc(sizeof(uip_ipaddr_t));
  memset(dest_addr, 0, sizeof(uip_ipaddr_t));
  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
