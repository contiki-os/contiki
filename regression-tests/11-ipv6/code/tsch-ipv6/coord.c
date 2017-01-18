/*
 * Copyright (c) 2017, Yasuyuki Tanaka
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
#include "contiki-net.h"
#include "net/mac/tsch/tsch.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#include "common.h"

#define UIP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*
 * This process ends once it receives one unicast packet and one broadcast
 * packet. Each time the process receives a packet, it sends it back to the
 * source by the same communication type as used for the received packet.
 */

PROCESS(coord_process, "TSCH Coordinator");
AUTOSTART_PROCESSES(&coord_process);

PROCESS_THREAD(coord_process, ev, data)
{
  static uip_ds6_addr_t *myaddr;
  static uip_ipaddr_t ipaddr;
  static uip_ipaddr_t linklocal_all_node;
  static struct uip_udp_conn *conn_unicast;
  static struct uip_udp_conn *conn_broadcast;
  static int unicast_received = 0;
  static int broadcast_received = 0;

  linkaddr_t *dest_lladdr;

  PROCESS_BEGIN();

  uiplib_ip6addrconv(NODE_ADDR, &ipaddr);
  conn_unicast = udp_new(&ipaddr, UIP_HTONS(APP_PORT), NULL);
  uip_udp_bind(conn_unicast, UIP_HTONS(APP_PORT));

  uip_create_linklocal_allnodes_mcast(&linklocal_all_node);
  conn_broadcast = udp_new(&linklocal_all_node, UIP_HTONS(APP_PORT), NULL);
  uip_udp_bind(conn_broadcast, UIP_HTONS(APP_PORT));

  myaddr = uip_ds6_get_link_local(-1);

  tsch_set_coordinator(1);

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event && uip_newdata()) {
      dest_lladdr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

      if(uip_ipaddr_cmp(&UIP_IP_BUF->destipaddr, &myaddr->ipaddr) &&
         linkaddr_cmp(dest_lladdr, &linkaddr_null) == 0) {
        /* unicast */
        PRINTF("APP: receive a unicast packet\n");
        uip_udp_packet_send(conn_unicast, uip_appdata, uip_datalen());
        PRINTF("APP: send a unicast packet\n");
        unicast_received = 1;
      } else {
        /* treated as broadcast */
        PRINTF("APP: receive a broadcast packet\n");
        uip_udp_packet_send(conn_broadcast, uip_appdata, uip_datalen());
        PRINTF("APP: send a broadcast packet\n");
        broadcast_received = 1;
      }
    }

    if(unicast_received && broadcast_received) {
      break;
    }
  }

  PRINTF("APP: done\n");

  PROCESS_END();
}
