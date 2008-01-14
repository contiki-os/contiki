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
 * $Id: neighbor-discovery.c,v 1.3 2008/01/14 09:34:00 adamdunkels Exp $
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
#include "net/rime/nf.h"
#include "net/rime/neighbor-discovery.h"

#include "dev/radio-sensor.h"

#include "lib/random.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

struct adv_msg {
  u16_t val;
};

#define SINK 0
#define HOPCOUNT_MAX TREE_MAX_DEPTH

#define MAX_HOPLIM 10

#define MAX_INTERVAL CLOCK_SECOND * 30
#define MIN_INTERVAL CLOCK_SECOND * 10
#define NEW_VAL_INTERVAL CLOCK_SECOND * 2

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
send_adv(struct neighbor_discovery_conn *c, clock_time_t interval)
{
  struct adv_msg *hdr;

  rimebuf_clear();
  rimebuf_set_datalen(sizeof(struct adv_msg));
  hdr = rimebuf_dataptr();
  hdr->val = c->val;
  /*  ibc_send(&c->c);*/
  ipolite_send(&c->c, interval, sizeof(struct adv_msg));
  if(c->u->sent) {
    c->u->sent(c);
  }
  PRINTF("%d.%d: sending neighbor advertisement with val %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 c->val);
}
/*---------------------------------------------------------------------------*/
static void
/*adv_packet_received(struct ibc_conn *ibc, rimeaddr_t *from)*/
adv_packet_received(struct ipolite_conn *ibc, rimeaddr_t *from)
{
  struct neighbor_discovery_conn *c = (struct neighbor_discovery_conn *)ibc;
  struct adv_msg *msg = rimebuf_dataptr();
/*   struct neighbor *n; */

  PRINTF("%d.%d: adv_packet_received from %d.%d with val %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], msg->val);

  if(c->u->recv) {
    c->u->recv(c, from, msg->val);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_timer(void *ptr)
{
  struct neighbor_discovery_conn *tc = ptr;
  
  send_adv(tc, MAX_INTERVAL);
  /*  ctimer_set(&tc->t,
	     MIN_INTERVAL + random_rand() % (MAX_INTERVAL - MIN_INTERVAL),
	     send_timer, tc);*/
  ctimer_set(&tc->t,
	     MAX_INTERVAL,
	     send_timer, tc);
}
/*---------------------------------------------------------------------------*/
/*static const struct ibc_callbacks ibc_callbacks =
  {adv_packet_received};*/
static const struct ipolite_callbacks ipolite_callbacks =
  {adv_packet_received};
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_open(struct neighbor_discovery_conn *c, uint16_t channel,
	 const struct neighbor_discovery_callbacks *cb)
{
  /*  ibc_open(&c->c, channel, &ibc_callbacks);*/
  ipolite_open(&c->c, channel, &ipolite_callbacks);
  c->u = cb;
}
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_close(struct neighbor_discovery_conn *c)
{
  /*  ibc_close(&c->c);*/
  ctimer_stop(&c->t);
  ipolite_close(&c->c);
}
/*---------------------------------------------------------------------------*/
void
neighbor_discovery_start(struct neighbor_discovery_conn *c, uint16_t val)
{
  if(val < c->val) {
    c->val = val;
    send_adv(c, NEW_VAL_INTERVAL);
    ctimer_set(&c->t, NEW_VAL_INTERVAL, send_timer, c);
  } else {
    c->val = val;
    send_adv(c, MIN_INTERVAL);
    ctimer_set(&c->t, MIN_INTERVAL, send_timer, c);
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
