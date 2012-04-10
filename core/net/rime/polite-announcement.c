/**
 * \addtogroup rimeexamples
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
 * $Id: polite-announcement.c,v 1.16 2010/06/16 10:10:10 nifi Exp $
 */

/**
 * \file
 *         An example announcement back-end, based on the polite primitive
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "lib/list.h"
#include "net/rime.h"
#include "net/rime/announcement.h"
#include "net/rime/ipolite.h"

#if NETSIM
#include "ether.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

#ifdef POLITE_ANNOUNCEMENT_CONF_MAX_DUPS
#define NUM_DUPS POLITE_ANNOUNCEMENT_CONF_MAX_DUPS
#else /* POLITE_ANNOUNCEMENT_CONF_MAX_DUPS */
#define NUM_DUPS 5
#endif /* POLITE_ANNOUNCEMENT_CONF_MAX_DUPS */

#define ANNOUNCEMENT_MSG_HEADERLEN 2
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[];
};


static struct polite_announcement_state {
  struct ipolite_conn c;
  struct ctimer t;
  clock_time_t interval;
  clock_time_t min_interval, max_interval;
} c;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define MIN(a, b) ((a)<(b)?(a):(b))

/*---------------------------------------------------------------------------*/
static void
send_adv(clock_time_t interval)
{
  struct announcement_msg *adata;
  struct announcement *a;

  packetbuf_clear();
  adata = packetbuf_dataptr();
  adata->num = 0;
  for(a = announcement_list(); a != NULL; a = list_item_next(a)) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  packetbuf_set_datalen(ANNOUNCEMENT_MSG_HEADERLEN +
		      sizeof(struct announcement_data) * adata->num);

  PRINTF("%d.%d: sending neighbor advertisement with %d announcements\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1], adata->num);

  if(adata->num > 0) {
    /* Send the packet only if it contains more than zero announcements. */
    ipolite_send(&c.c, interval, packetbuf_datalen());
  }
}
/*---------------------------------------------------------------------------*/
static void
adv_packet_received(struct ipolite_conn *ipolite, const rimeaddr_t *from)
{
  struct announcement_msg adata;
  struct announcement_data data;
  uint8_t *ptr;
  int i;

  ptr = packetbuf_dataptr();

  /* Copy number of announcements */
  memcpy(&adata, ptr, sizeof(struct announcement_msg));
  PRINTF("%d.%d: adv_packet_received from %d.%d with %d announcements\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
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
send_timer(void *ptr)
{
  send_adv(c.interval);
  ctimer_set(&c.t,
	     c.interval,
	     send_timer, &c);

  c.interval = MIN(c.interval * 2, c.max_interval);
}
/*---------------------------------------------------------------------------*/
static void
new_announcement(uint16_t id, uint8_t has_value, uint16_t newval,
    uint16_t oldval, uint8_t bump)
{
  if(newval != oldval) {
    c.interval = c.min_interval;
    send_timer(&c);
  }
}
/*---------------------------------------------------------------------------*/
static const struct ipolite_callbacks ipolite_callbacks =
  {adv_packet_received, NULL, NULL};
/*---------------------------------------------------------------------------*/
void
polite_announcement_init(uint16_t channel,
			clock_time_t min,
			clock_time_t max)
{
  ipolite_open(&c.c, channel, NUM_DUPS, &ipolite_callbacks);

  c.min_interval = min;
  c.max_interval = max;

  announcement_register_observer_callback(new_announcement);
}
/*---------------------------------------------------------------------------*/
void
polite_announcement_stop(void)
{
  ctimer_stop(&c.t);
  ipolite_close(&c.c);
}
/*---------------------------------------------------------------------------*/
/** @} */
