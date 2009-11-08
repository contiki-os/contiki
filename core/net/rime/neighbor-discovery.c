/**
 * \addtogroup rimeneighbordiscovery
 * @{
 */

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
 * This file is part of the Contiki operating system.
 *
 * $Id: neighbor-discovery.c,v 1.15 2009/11/08 19:40:17 adamdunkels Exp $
 */

/**
 * \file
 *         Neighbor discovery
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime.h"
#include "net/rime/neighbor.h"
#include "net/rime/neighbor-discovery.h"

#include "dev/radio-sensor.h"

#include "lib/random.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

struct adv_msg {
  uint16_t val;
};

#define SINK 0
#define HOPCOUNT_MAX TREE_MAX_DEPTH

#define MAX_HOPLIM 10

/*#define MAX_INTERVAL CLOCK_SECOND * 60
#define MIN_INTERVAL CLOCK_SECOND * 10
#define NEW_VAL_INTERVAL CLOCK_SECOND * 2*/

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_adv(void *ptr)
{
  struct neighbor_discovery_conn *c = ptr;
  struct adv_msg *hdr;

  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct adv_msg));
  hdr = packetbuf_dataptr();
  hdr->val = c->val;
  broadcast_send(&c->c);
  if(c->u->sent) {
    c->u->sent(c);
  }
  PRINTF("%d.%d: sending neighbor advertisement with val %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 c->val);
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_received(struct broadcast_conn *ibc, const rimeaddr_t *from)
{
  struct neighbor_discovery_conn *c = (struct neighbor_discovery_conn *)ibc;
  struct adv_msg *msg = packetbuf_dataptr();

  PRINTF("%d.%d: adv_packet_received from %d.%d with val %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], msg->val);

  /* If we receive an announcement with a lower value than ours, we
     cancel our own announcement. */
  if(msg->val < c->val) {
    ctimer_stop(&c->send_timer);
  }
  
  if(c->u->recv) {
    c->u->recv(c, from, msg->val);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_timer(void *ptr)
{
  struct neighbor_discovery_conn *tc = ptr;

  ctimer_set(&tc->send_timer,
	     tc->max_interval / 2 + random_rand() % (tc->max_interval / 2),
	     send_adv, tc);
  ctimer_set(&tc->interval_timer,
	     tc->max_interval,
	     send_timer, tc);
}
/*---------------------------------------------------------------------------*/
static CC_CONST_FUNCTION struct broadcast_callbacks broadcast_callbacks =
  {adv_packet_received};
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_open(struct neighbor_discovery_conn *c, uint16_t channel,
			clock_time_t initial,
			clock_time_t min,
			clock_time_t max,
			const struct neighbor_discovery_callbacks *cb)
{
  broadcast_open(&c->c, channel, &broadcast_callbacks);
  c->u = cb;
  c->initial_interval = initial;
  c->min_interval = min;
  c->max_interval = max;
}
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_close(struct neighbor_discovery_conn *c)
{
  broadcast_close(&c->c);
  ctimer_stop(&c->send_timer);
  ctimer_stop(&c->interval_timer);
}
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_set_val(struct neighbor_discovery_conn *c, uint16_t val)
{
  c->val = val;
}
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_start(struct neighbor_discovery_conn *c, uint16_t val)
{
  clock_time_t interval;

  if(val < c->val) {
    interval = c->initial_interval;
  } else {
    interval = c->min_interval;
  }
  c->val = val;
  ctimer_set(&c->interval_timer, interval, send_timer, c);
  ctimer_set(&c->send_timer, interval / 2 + random_rand() % (interval / 2),
	     send_adv, c);
}
/*---------------------------------------------------------------------------*/
/** @} */
