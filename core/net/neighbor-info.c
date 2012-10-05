/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *
 * $Id: neighbor-info.c,v 1.18 2010/12/15 14:35:07 nvt-se Exp $
 */
/**
 * \file
 *         A generic module for management of neighbor information.
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/neighbor-info.h"
#include "net/neighbor-attr.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define ETX_LIMIT		15
#define ETX_SCALE		100
#define ETX_ALPHA		90
#define ETX_NOACK_PENALTY       ETX_LIMIT
/*---------------------------------------------------------------------------*/
NEIGHBOR_ATTRIBUTE_GLOBAL(link_metric_t, attr_etx, NULL);
NEIGHBOR_ATTRIBUTE_GLOBAL(unsigned long, attr_timestamp, NULL);

static neighbor_info_subscriber_t subscriber_callback;
/*---------------------------------------------------------------------------*/
static void
update_metric(const rimeaddr_t *dest, int packet_metric)
{
  link_metric_t *metricp;
  link_metric_t recorded_metric, new_metric;
  unsigned long time;
  int first_update = 0;

  metricp = (link_metric_t *)neighbor_attr_get_data(&attr_etx, dest);
  packet_metric = NEIGHBOR_INFO_ETX2FIX(packet_metric);
  if(metricp == NULL || *metricp == 0) {
    recorded_metric = NEIGHBOR_INFO_ETX2FIX(ETX_LIMIT);
    new_metric = packet_metric;
    first_update = 1;
  } else {
    recorded_metric = *metricp;
    /* Update the EWMA of the ETX for the neighbor. */
    new_metric = ((uint16_t)recorded_metric * ETX_ALPHA +
               (uint16_t)packet_metric * (ETX_SCALE - ETX_ALPHA)) / ETX_SCALE;
  }

  PRINTF("neighbor-info: ETX changed from %d to %d (packet ETX = %d) %d\n",
	 NEIGHBOR_INFO_FIX2ETX(recorded_metric),
	 NEIGHBOR_INFO_FIX2ETX(new_metric),
	 NEIGHBOR_INFO_FIX2ETX(packet_metric),
         dest->u8[7]);

  if(neighbor_attr_has_neighbor(dest)) {
    time = clock_seconds();
    neighbor_attr_set_data(&attr_etx, dest, &new_metric);
    neighbor_attr_set_data(&attr_timestamp, dest, &time);
    if((first_update || new_metric != recorded_metric) && subscriber_callback != NULL) {
      subscriber_callback(dest, 1, new_metric);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
add_neighbor(const rimeaddr_t *addr)
{
  switch(neighbor_attr_add_neighbor(addr)) {
  case -1:
    PRINTF("neighbor-info: failed to add a node.\n");
    break;
  case 0:
    PRINTF("neighbor-info: The neighbor is already known\n");
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_info_packet_sent(int status, int numtx)
{
  const rimeaddr_t *dest;
  link_metric_t packet_metric;
#if UIP_DS6_LL_NUD
  uip_ds6_nbr_t *nbr;
#endif /* UIP_DS6_LL_NUD */

  dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(rimeaddr_cmp(dest, &rimeaddr_null)) {
    return;
  }

  packet_metric = numtx;

  PRINTF("neighbor-info: packet sent to %d.%d, status=%d, metric=%u\n",
	dest->u8[sizeof(*dest) - 2], dest->u8[sizeof(*dest) - 1],
	status, (unsigned)packet_metric);

  switch(status) {
  case MAC_TX_OK:
    add_neighbor(dest);
#if UIP_DS6_LL_NUD
    nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)dest);
    if(nbr != NULL &&
       (nbr->state == STALE || nbr->state == DELAY || nbr->state == PROBE)) {
      nbr->state = REACHABLE;
      stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
      PRINTF("neighbor-info : received a link layer ACK : ");
      PRINTLLADDR((uip_lladdr_t *)dest);
      PRINTF(" is reachable.\n");
    }
#endif /* UIP_DS6_LL_NUD */
    break;
  case MAC_TX_NOACK:
    packet_metric = ETX_NOACK_PENALTY;
    break;
  default:
    /* Do not penalize the ETX when collisions or transmission
       errors occur. */
    return;
  }

  update_metric(dest, packet_metric);
}
/*---------------------------------------------------------------------------*/
void
neighbor_info_packet_received(void)
{
  const rimeaddr_t *src;

  src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  if(rimeaddr_cmp(src, &rimeaddr_null)) {
    return;
  }

  PRINTF("neighbor-info: packet received from %d.%d\n",
	src->u8[sizeof(*src) - 2], src->u8[sizeof(*src) - 1]);

  add_neighbor(src);
}
/*---------------------------------------------------------------------------*/
int
neighbor_info_subscribe(neighbor_info_subscriber_t s)
{
  if(subscriber_callback == NULL) {
    neighbor_attr_register(&attr_etx);
    neighbor_attr_register(&attr_timestamp);
    subscriber_callback = s;
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
link_metric_t
neighbor_info_get_metric(const rimeaddr_t *addr)
{
  link_metric_t *metricp;

  metricp = (link_metric_t *)neighbor_attr_get_data(&attr_etx, addr);
  return metricp == NULL ? ETX_LIMIT : *metricp;
}
/*---------------------------------------------------------------------------*/
