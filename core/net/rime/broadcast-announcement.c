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
 */

/**
 * \addtogroup rimebroadcastannouncement
 * @{
 */

/**
 * \file
 *         An example announcement back-end, based on the broadcast primitive
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "net/rime/rime.h"
#include "net/rime/announcement.h"
#include "net/rime/broadcast.h"
#include "lib/random.h"
#include "lib/list.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stddef.h>

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

#ifdef BROADCAST_ANNOUNCEMENT_CONF_MAX_DUPS
#define NUM_DUPS BROADCAST_ANNOUNCEMENT_CONF_MAX_DUPS
#else /* BROADCAST_ANNOUNCEMENT_CONF_MAX_DUPS */
#define NUM_DUPS 5
#endif /* BROADCAST_ANNOUNCEMENT_CONF_MAX_DUPS */

#define ANNOUNCEMENT_MSG_HEADERLEN 2
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[];
};


static struct broadcast_announcement_state {
  struct broadcast_conn c;
  struct ctimer send_timer, interval_timer;
  clock_time_t initial_interval, min_interval, max_interval;
  clock_time_t current_interval;
  uint16_t val;
} c;


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
  struct announcement_msg *adata;
  struct announcement *a;

  packetbuf_clear();
  adata = packetbuf_dataptr();
  adata->num = 0;
  for(a = announcement_list(); a != NULL && a->has_value; a = list_item_next(a)) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  packetbuf_set_datalen(ANNOUNCEMENT_MSG_HEADERLEN +
		      sizeof(struct announcement_data) * adata->num);

  PRINTF("%d.%d: sending neighbor advertisement with %d announcements\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], adata->num);

  if(adata->num > 0) {
    /* Send the packet only if it contains more than zero announcements. */
    broadcast_send(&c.c);
  }
  PRINTF("%d.%d: sending neighbor advertisement with val %d\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 c.val);
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_received(struct broadcast_conn *ibc, const linkaddr_t *from)
{
  struct announcement_msg adata;
  struct announcement_data data;
  uint8_t *ptr;
  int i;

  ptr = packetbuf_dataptr();

  /* Copy number of announcements */
  memcpy(&adata, ptr, sizeof(struct announcement_msg));
  PRINTF("%d.%d: adv_packet_received from %d.%d with %d announcements\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], adata.num);

  if(ANNOUNCEMENT_MSG_HEADERLEN + adata.num * sizeof(struct announcement_data) > packetbuf_datalen()) {
    /* The number of announcements is too large - corrupt packet has
       been received. */
    PRINTF("adata.num way out there: %d\n", adata.num);
    return;
  }

  ptr += ANNOUNCEMENT_MSG_HEADERLEN;
  for(i = 0; i < adata.num; ++i) {
    /* Copy announcements */
    memcpy(&data, ptr, sizeof(struct announcement_data));
    announcement_heard(from, data.id, data.value);
    ptr += sizeof(struct announcement_data);
  }
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_sent(struct broadcast_conn *bc, int status, int num_tx)
{
}
/*---------------------------------------------------------------------------*/
static void send_timer(void *ptr);

static void
set_timers(void)
{
  ctimer_set(&c.interval_timer, c.current_interval, send_timer, NULL);
  ctimer_set(&c.send_timer, random_rand() % c.current_interval,
             send_adv, NULL);
}
/*---------------------------------------------------------------------------*/
static void
send_timer(void *ptr)
{
  clock_time_t interval;

  interval = c.current_interval * 2;

  if(interval > c.max_interval) {
    interval = c.max_interval;
  }

  c.current_interval = interval;

  /*  printf("current_interval %lu\n", (long unsigned int) interval);*/

  set_timers();
}
/*---------------------------------------------------------------------------*/
static void
new_announcement(uint16_t id, uint8_t has_value,
                 uint16_t newval, uint16_t oldval, uint8_t bump)
{
  if(bump == ANNOUNCEMENT_BUMP) {
    c.current_interval = c.initial_interval;
    set_timers();
    /*  } else if(newval != oldval) {
    c.current_interval = c.min_interval;
    set_timers();*/
  }
}
/*---------------------------------------------------------------------------*/
static CC_CONST_FUNCTION struct broadcast_callbacks broadcast_callbacks =
  {adv_packet_received, adv_packet_sent };
/*---------------------------------------------------------------------------*/
void
broadcast_announcement_init(uint16_t channel,
                            clock_time_t initial,
                            clock_time_t min,
                            clock_time_t max)
{
  broadcast_open(&c.c, channel, &broadcast_callbacks);
  c.initial_interval = initial;
  c.min_interval = min;
  c.max_interval = max;

  announcement_register_observer_callback(new_announcement);
}
/*---------------------------------------------------------------------------*/
void
broadcast_announcement_stop(void)
{
  ctimer_stop(&c.interval_timer);
  ctimer_stop(&c.send_timer);
  broadcast_close(&c.c);
}
/*---------------------------------------------------------------------------*/
clock_time_t
broadcast_announcement_beacon_interval(void)
{
  return c.current_interval;
}
/*---------------------------------------------------------------------------*/
/** @} */
