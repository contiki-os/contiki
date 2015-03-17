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
 */
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *     An implementation of a 6LBR UDP client. Is used to populate the 6LBR
 *     web server's 'sensors' tab
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"

#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#include "net/ip/uip-debug.h"
/*---------------------------------------------------------------------------*/
#ifndef CETIC_6LBR_NODE_INFO_PORT
#define CETIC_6LBR_NODE_INFO_PORT 3000
#endif

#define MAX_PAYLOAD_LEN    40
#define MSG_INTERVAL       (60 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *client_conn = NULL;
static struct etimer et;
static uip_ip6addr_t dest_addr;
/*---------------------------------------------------------------------------*/
PROCESS(cetic_6lbr_client_process, "6LBR Client Process");
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    PRINTF("Response from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static char *
add_ipaddr(char *buf, const uip_ipaddr_t *addr)
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
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];
  int i;
  uip_ip6addr_t *globaladdr = NULL;
  uint16_t dest_port = CETIC_6LBR_NODE_INFO_PORT;
  int has_dest = 0;
  rpl_dag_t *dag;

  uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
  if(addr_desc != NULL) {
    globaladdr = &addr_desc->ipaddr;
    dag = rpl_get_any_dag();
    if(dag) {
      uip_ipaddr_copy(&dest_addr, globaladdr);
      memcpy(&dest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2);
      has_dest = 1;
    }
  }

  if(has_dest) {
    if(client_conn == NULL) {
      PRINTF("UDP-CLIENT: address destination: ");
      PRINT6ADDR(&dest_addr);
      PRINTF("\n");
      client_conn = udp_new(&dest_addr, UIP_HTONS(dest_port), NULL);

      if(client_conn != NULL) {
        PRINTF("Created a connection with the server ");
        PRINT6ADDR(&client_conn->ripaddr);
        PRINTF(" local/remote port %u/%u\n",
               UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
      } else {
        PRINTF("Could not open connection\n");
      }
    } else {
      if(memcmp(&client_conn->ripaddr, &dest_addr, sizeof(uip_ipaddr_t)) != 0) {
        PRINTF("UDP-CLIENT: new address destination: ");
        PRINT6ADDR(&dest_addr);
        PRINTF("\n");
        uip_udp_remove(client_conn);
        client_conn = udp_new(&dest_addr, UIP_HTONS(dest_port), NULL);
        if(client_conn != NULL) {
          PRINTF("Created a connection with the server ");
          PRINT6ADDR(&client_conn->ripaddr);
          PRINTF(" local/remote port %u/%u\n",
                 UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
        } else {
          PRINTF("Could not open connection\n");
        }
      }
    }
    if(client_conn != NULL) {
      PRINTF("Client sending to: ");
      PRINT6ADDR(&client_conn->ripaddr);
      i = sprintf(buf, "%d | ", ++seq_id);
      dag = rpl_get_any_dag();
      if(dag && dag->instance->def_route) {
        add_ipaddr(buf + i, &dag->instance->def_route->ipaddr);
      } else {
        sprintf(buf + i, "(null)");
      }
      PRINTF(" (msg: %s)\n", buf);
      uip_udp_packet_send(client_conn, buf, strlen(buf));
    } else {
      PRINTF("No connection created\n");
    }
  } else {
    PRINTF("No address configured\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cetic_6lbr_client_process, ev, data)
{

  PROCESS_BEGIN();

  printf("6LBR Client Process\n");

  memset(&dest_addr, 0, sizeof(uip_ipaddr_t));

  etimer_set(&et, MSG_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_set(&et, MSG_INTERVAL);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
