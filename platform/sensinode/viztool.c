/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Small UDP app used to retrieve neighbor cache and routing table
 *         entries and send them to an external endpoint
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip-ds6-route.h"

#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#ifndef VIZTOOL_MAX_PAYLOAD_LEN
#define VIZTOOL_MAX_PAYLOAD_LEN 60
#endif

static struct uip_udp_conn *server_conn;
static unsigned char buf[VIZTOOL_MAX_PAYLOAD_LEN];
static int8_t len;

#define VIZTOOL_UDP_PORT   60001

/* Request Bits */
#define REQUEST_TYPE_ND         1
#define REQUEST_TYPE_RT         2
#define REQUEST_TYPE_DRT        3
#define REQUEST_TYPE_ADDR       4
#define REQUEST_TYPE_TOTALS  0xFF

extern uip_ds6_netif_t uip_ds6_if;
static uip_ds6_route_t *rt;
static uip_ds6_defrt_t *defrt;
static uip_ipaddr_t *addr;
/*---------------------------------------------------------------------------*/
static uint8_t
process_request() CC_NON_BANKED
{
  uint8_t len;
  uint8_t count; /* How many did we pack? */
  uint8_t i;
  uint8_t left;
  uint8_t entry_size;
  uip_ds6_nbr_t *nbr;

  left = VIZTOOL_MAX_PAYLOAD_LEN - 1;
  len = 2; /* start filling the buffer from position [2] */
  count = 0;
  if(buf[0] == REQUEST_TYPE_ND) {
    /* Neighbors */
    PRINTF("Neighbors\n");
    for(nbr = nbr_table_head(ds6_neighbors);
        nbr != NULL;
        nbr = nbr_table_next(ds6_neighbors, nbr)) {
      entry_size = sizeof(i) + sizeof(uip_ipaddr_t) + sizeof(uip_lladdr_t)
              + sizeof(nbr->state);
      PRINTF("%02u: ", i);
      PRINT6ADDR(&nbr->ipaddr);
      PRINTF(" - ");
      PRINTLLADDR(&nbr->lladdr);
      PRINTF(" - %u\n", nbr->state);

      memcpy(buf + len, &i, sizeof(i));
      len += sizeof(i);
      memcpy(buf + len, uip_ds6_nbr_get_ipaddr(nbr), sizeof(uip_ipaddr_t));
      len += sizeof(uip_ipaddr_t);
      memcpy(buf + len, uip_ds6_nbr_get_ll(nbr), sizeof(uip_lladdr_t));
      len += sizeof(uip_lladdr_t);
      memcpy(buf + len, &nbr->state,
          sizeof(nbr->state));
      len += sizeof(nbr->state);

      count++;
      left -= entry_size;

      if(left < entry_size) {
        break;
      }
    }
  } else if(buf[0] == REQUEST_TYPE_RT) {
    uint32_t flip = 0;

    PRINTF("Routing table\n");
    rt = uip_ds6_route_head();

    for(i = buf[1]; i < uip_ds6_route_num_routes(); i++) {
      if(rt != NULL) {
        entry_size = sizeof(i) + sizeof(rt->ipaddr)
          + sizeof(rt->length)
          + sizeof(rt->state.lifetime)
          + sizeof(rt->state.learned_from);

        memcpy(buf + len, &i, sizeof(i));
        len += sizeof(i);
        memcpy(buf + len, &rt->ipaddr, sizeof(rt->ipaddr));
        len += sizeof(rt->ipaddr);
        memcpy(buf + len, &rt->length, sizeof(rt->length));
        len += sizeof(rt->length);

        PRINT6ADDR(&rt->ipaddr);
        PRINTF(" - %02x", rt->length);
        PRINTF(" - ");
        PRINT6ADDR(uip_ds6_route_nexthop(rt));

        flip = uip_htonl(rt->state.lifetime);
        memcpy(buf + len, &flip, sizeof(flip));
        len += sizeof(flip);
        PRINTF(" - %08lx", rt->state.lifetime);

        memcpy(buf + len, &rt->state.learned_from,
               sizeof(rt->state.learned_from));
        len += sizeof(rt->state.learned_from);

        PRINTF(" - %02x [%u]\n", rt->state.learned_from, entry_size);

        count++;
        left -= entry_size;

        rt = uip_ds6_route_next(rt);

        if(left < entry_size) {
          break;
        }
      }
    }
  } else if(buf[0] == REQUEST_TYPE_DRT) {
    uint32_t flip = 0;

    PRINTF("Default Route\n");
    addr = uip_ds6_defrt_choose();
    if(addr != NULL) {
      defrt = uip_ds6_defrt_lookup(addr);
    }

    i = buf[1];

    if(defrt != NULL && i < 1) {
      entry_size = sizeof(i) + sizeof(defrt->ipaddr)
        + sizeof(defrt->isinfinite);

      memcpy(buf + len, &i, sizeof(i));
      len += sizeof(i);
      memcpy(buf + len, &defrt->ipaddr, sizeof(defrt->ipaddr));
      len += sizeof(defrt->ipaddr);
      memcpy(buf + len, &defrt->isinfinite, sizeof(defrt->isinfinite));
      len += sizeof(defrt->isinfinite);

      PRINT6ADDR(&defrt->ipaddr);
      PRINTF(" - %u\n", defrt->isinfinite);
      count++;
      left -= entry_size;
    }
  } else if(buf[0] == REQUEST_TYPE_ADDR) {
    PRINTF("Unicast Addresses\n");
    for(i = buf[1]; i < UIP_DS6_ADDR_NB; i++) {
      if(uip_ds6_if.addr_list[i].isused) {
        entry_size = sizeof(i) + sizeof(uip_ds6_if.addr_list[i].ipaddr);

        memcpy(buf + len, &i, sizeof(i));
        len += sizeof(i);
        memcpy(buf + len, &uip_ds6_if.addr_list[i].ipaddr,
               sizeof(uip_ds6_if.addr_list[i].ipaddr));
        len += sizeof(uip_ds6_if.addr_list[i].ipaddr);

        PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
        PRINTF("\n");
        count++;
        left -= entry_size;

        if(left < entry_size) {
          break;
        }
      }
    }
  } else if(buf[0] == REQUEST_TYPE_TOTALS) {
    memset(&buf[2], 0, 4);
    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
      if(uip_ds6_if.addr_list[i].isused) {
        buf[2]++;
      }
    }
    for(nbr = nbr_table_head(ds6_neighbors);
        nbr != NULL;
        nbr = nbr_table_next(ds6_neighbors, nbr)) {
        buf[3]++;
    }

    buf[4] = uip_ds6_route_num_routes();
    buf[5] = 1;

    len += 4;
    count = 4;
  } else {
    return 0;
  }
  buf[1] = count;
  return len;
}
/*---------------------------------------------------------------------------*/
PROCESS(viztool_process, "Network Visualization Tool Process");
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void) CC_NON_BANKED
{
  if(uip_newdata()) {
    memset(buf, 0, VIZTOOL_MAX_PAYLOAD_LEN);

    PRINTF("%u bytes from [", uip_datalen());
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));

    memcpy(buf, uip_appdata, uip_datalen());

    len = process_request();
    if(len) {
      server_conn->rport = UIP_UDP_BUF->srcport;
      uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
      uip_udp_packet_send(server_conn, buf, len);
      PRINTF("Sent %u bytes\n", len);
    }

    /* Restore server connection to allow data from any node */
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
  }
  return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(viztool_process, ev, data)
{

  PROCESS_BEGIN();

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(VIZTOOL_UDP_PORT));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
