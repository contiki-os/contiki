/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         ORPL anycast-related aspects. Includes softack callback functions,
 *         anycast address construction and parsing.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "orpl.h"
#include "orpl-routing-set.h"
#include "orpl-anycast.h"
#include "net/packetbuf.h"
#include "cc2420-softack.h"
#include "net/mac/frame802154.h"
#include <string.h>

#if WITH_ORPL

/* The different link-layer addresses used for anycast */
rimeaddr_t anycast_addr_up = {.u8 = {0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa}};
rimeaddr_t anycast_addr_down = {.u8 = {0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb}};
rimeaddr_t anycast_addr_nbr = {.u8 = {0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc}};
rimeaddr_t anycast_addr_recover = {.u8 = {0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd}};

/* Callback functions for 802.15.4 softack driver */
static void orpl_softack_acked_callback(const uint8_t *buf, uint8_t len);
static void orpl_softack_input_callback(const uint8_t *buf, uint8_t len, uint8_t **ackbufptr, uint8_t *acklen);

/* A buffer where extended 802.15.4 are prepared */
static unsigned char ackbuf[3 + EXTRA_ACK_LEN] = {0x02, 0x00};
/* Seqno of the last acked frame */
static uint8_t last_acked_seqno = -1;

/* Set the destination link-layer address in packetbuf in case of anycast.
 * The address contains the following information:
 * - direction, among up, down, nbr, recover
 * - the EDC of the sender
 * - the end-to-end sequence number
 *  */
void
orpl_anycast_set_packetbuf_addr()
{
  uint16_t *ptr = (uint16_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  /* Check is the address is an anycast address */
  if(rimeaddr_cmp((rimeaddr_t*)ptr, &anycast_addr_up) || rimeaddr_cmp((rimeaddr_t*)ptr, &anycast_addr_down)
      || rimeaddr_cmp((rimeaddr_t*)ptr, &anycast_addr_nbr) || rimeaddr_cmp((rimeaddr_t*)ptr, &anycast_addr_recover)) {
    uint32_t seqno = orpl_packetbuf_seqno();
    /* Append EDC and sequence number */
    ptr[1] = orpl_current_edc();
    ptr[2] = seqno >> 16;
    ptr[3] = seqno;
  }
}

/* The frame was acked (i.e. we wanted to ack it AND it was not corrupt).
 * Store the last acked sequence number to avoid repeatedly acking in case
 * we're not duty cycled (e.g. border router) */
static void
orpl_softack_acked_callback(const uint8_t *frame, uint8_t framelen)
{
	last_acked_seqno = frame[2];
}

/* Called for every incoming frame from interrupt. We check if we want to ack the
 * frame and prepare an ACK if needed */
static void
orpl_softack_input_callback(const uint8_t *frame, uint8_t framelen, uint8_t **ackbufptr, uint8_t *acklen)
{
	uint8_t fcf, is_data, ack_required, seqno;
	int do_ack = 0;

	fcf = frame[0];
	is_data = (fcf & 7) == 1;
	ack_required = (fcf >> 5) & 1;
	seqno = frame[2];

	if(is_data) {
		if(ack_required) { /* This is unicast or unicast, parse it */
			do_ack = orpl_anycast_parse_802154_frame((uint8_t *)frame, framelen, 0).do_ack;
		} else { /* We also ack broadcast, even if we didn't modify the framer
		and still send them with ack_required unset */
			if(seqno != last_acked_seqno) {
				do_ack = 1;
			}
		}
	}

	if(do_ack) { /* Prepare ack */
	  rpl_rank_t curr_edc = orpl_current_edc();
		*ackbufptr = ackbuf;
		*acklen = sizeof(ackbuf);
		ackbuf[2] = seqno;
		/* Append our address to the standard 802.15.4 ack */
		rimeaddr_copy((rimeaddr_t*)(ackbuf+3), &rimeaddr_node_addr);
		/* Append our rank to the ack */
		ackbuf[3+8] = curr_edc & 0xff;
		ackbuf[3+8+1] = (curr_edc >> 8)& 0xff;
	} else {
		*acklen = 0;
	}
}

/* Parse a link-layer address, extract anycast direction, sender EDC, end-to-end sequence number.
 * Return 1 if anycast, 0 otherwise */
static int
anycast_parse_addr(rimeaddr_t *addr, enum anycast_direction_e *anycast_direction,
    uint16_t *curr_edc, uint32_t *seqno)
{
  int up = 0;
  int down = 0;
  int nbr = 0;
  int recover = 0;

  int i;
  uint8_t addr_host_order[8];
  /* Convert from litte endian (802.15.4) to big endian (Conitki addresses) */
  for(i=0; i<8; i++) {
    addr_host_order[i] = addr->u8[7-i];
  }

  /* Compare only the 2 first bytes, as other bytes carry curr_edc and seqno */
  if(!memcmp(addr_host_order, &anycast_addr_up, 2)) {
    if(anycast_direction) *anycast_direction = direction_up;
    up = 1;
  } else if(!memcmp(addr_host_order, &anycast_addr_down, 2)) {
    if(anycast_direction) *anycast_direction = direction_down;
    down = 1;
  } else if(!memcmp(addr_host_order, &anycast_addr_nbr, 2)) {
    if(anycast_direction) *anycast_direction = direction_nbr;
    nbr = 1;
  } else if(!memcmp(addr_host_order, &anycast_addr_recover, 2)) {
    if(anycast_direction) *anycast_direction = direction_recover;
    recover = 1;
  }

  uint16_t *ptr = (uint16_t*)addr_host_order;
  /* Extrace sender EDC */
  if(curr_edc) *curr_edc = ptr[1];
  /* Extrace end-to-end sequence number */
  if(seqno) *seqno = (((uint32_t)ptr[2]) << 16) + (uint32_t)ptr[3];

  if(!up && !down && !nbr && !recover) {
    return 0; /* This is not an anycast address */
  } else {
    return 1; /* This is an anycast address */
  }
}

/* Parse a modified 802.15.4 frame and return information regarding routing and software acks.
 * When set_dest_addr, set the destination address as ours if we have acked the frame. */
struct anycast_parsing_info
orpl_anycast_parse_802154_frame(uint8_t *data, uint8_t len, int set_dest_addr)
{
  frame802154_fcf_t fcf;
  uint8_t *dest_addr = NULL;
  uint8_t *src_addr = NULL;
  struct anycast_parsing_info ret;
  int i;

  memset(&ret, 0, sizeof(ret));

  if(len < 3) {
    return ret;
  }

  /* Decode the FCF */
  fcf.frame_type = data[0] & 7;
  fcf.ack_required = (data[0] >> 5) & 1;
  dest_addr = data + 3 + 2;
  src_addr = data + 3 + 2 + 8;

  /* This is a unciast or anycast data frame */
  if(fcf.frame_type == FRAME802154_DATAFRAME && fcf.ack_required == 1) {
    uint8_t src_addr_host_order[8];
    uint8_t dest_addr_host_order[8];

    /* Convert from 802.15.4 little endian to Contiki's big-endian addresses */
    for(i=0; i<8; i++) {
      src_addr_host_order[i] = src_addr[7-i];
      dest_addr_host_order[i] = dest_addr[7-i];
    }

    /* Parse the destination address */
    if(anycast_parse_addr((rimeaddr_t*)dest_addr, &ret.direction, &ret.neighbor_edc, &ret.seqno)) {
      rpl_rank_t curr_edc = orpl_current_edc();

      /* Calculate destination IPv6 address */
      /* TODO ORPL: better document this addressing */
      uip_ipaddr_t dest_ipv6;
      memcpy(&dest_ipv6, &global_ipv6, 8); /* override prefix */
      memcpy(((char*)&dest_ipv6)+8, data + 33, 8);

      if(uip_ip6addr_cmp(&dest_ipv6, &global_ipv6)) {
        /* Take the data if it is for us */
        ret.do_ack = 1;
      } else if(rimeaddr_cmp((rimeaddr_t*)dest_addr_host_order, &rimeaddr_node_addr)) {
        /* Unicast, for us */
        ret.do_ack = 1;
      } else if(ret.direction == direction_up) {
        /* Routing upwards. ACK if our rank is better. */
        if(ret.neighbor_edc > ORPL_EDC_W && curr_edc < ret.neighbor_edc - ORPL_EDC_W) {
          ret.do_ack = 1;
        } else {
          /* We don't route upwards, now check if we are a common ancester of the source
           * and destination. We do this by checking our routing set against the destination. */
          if(!orpl_blacklist_contains(ret.seqno) && orpl_routing_set_contains(&dest_ipv6)) {
            /* Traffic is going up but we have destination in our routing set.
             * Ack it and start routing downwards (towards the destination) */
            ret.do_ack = 1;
          }
        }
      } else if(ret.direction == direction_down) {
        /* Routing downwards. ACK if destination is reachable neighbor or
         * we it is in subdodag and we have a worse rank */
        if(!orpl_blacklist_contains(ret.seqno)
            && (orpl_is_reachable_neighbor(&dest_ipv6)
                || (curr_edc > ORPL_EDC_W && curr_edc - ORPL_EDC_W > ret.neighbor_edc
                && orpl_routing_set_contains(&dest_ipv6)))) {
          ret.do_ack = 1;
        }

      } else if(ret.direction == direction_recover) {
        /* This packet is sent back from a child that experiences false positive. Only
         * the nodes that did forward this same packet downwards before are allowed to
         * take the packet back during a recovery, before sending down again. This is
         * to avoid duplicates during the recovery process. */
        ret.do_ack = orpl_acked_down_contains(ret.seqno, (const rimeaddr_t *)src_addr_host_order);
      }
    }

    /* Set destination address to ours in case we acked the packet to it doesn't
     * get dropped next */
    if(ret.do_ack && set_dest_addr) {
      int i;
      for(i=0; i<8; i++) {
        dest_addr[i] = rimeaddr_node_addr.u8[7-i];
      }
    }
  }

  /* Return routing/acknowledging information */
  return ret;
}

/* Anycast-specific inits */
void
orpl_anycast_init()
{
  /* Subscribe to 802.15.4 softack driver */
  cc2420_softack_subscribe(orpl_softack_input_callback, orpl_softack_acked_callback);
}

#endif /* WITH_ORPL */
