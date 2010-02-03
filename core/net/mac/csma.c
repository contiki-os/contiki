/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: csma.c,v 1.5 2010/02/03 01:17:54 adamdunkels Exp $
 */

/**
 * \file
 *         A MAC 
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#define CSMA_CONF_REXMIT 1

#include "net/mac/csma.h"
#include "net/rime/packetbuf.h"
#include "net/rime/queuebuf.h"
#include "net/rime/ctimer.h"

#include "lib/random.h"

#include "lib/list.h"
#include "lib/memb.h"

#include <string.h>

struct queued_packet {
  struct queued_packet *next;
  struct queuebuf *buf;
  struct ctimer retransmit_timer;
  uint8_t retransmits;
};

#define MAX_QUEUED_PACKETS 4
LIST(packet_list);
MEMB(packet_memb, struct queued_packet, MAX_QUEUED_PACKETS);

static const struct mac_driver *mac;
static void (* receiver_callback)(const struct mac_driver *);

const struct mac_driver *csma_init(const struct mac_driver *psc);

/*---------------------------------------------------------------------------*/
#if CSMA_CONF_REXMIT
static void
retransmit_packet(void *ptr)
{
  int ret;
  struct queued_packet *q = ptr;

  queuebuf_to_packetbuf(q->buf);
  ret = mac->send();

  queuebuf_free(q->buf);
  list_remove(packet_list, q);
  memb_free(&packet_memb, q);
}
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  struct queuebuf *buf;
  int ret;
  clock_time_t time;
  rimeaddr_t receiver;

  /* Remember packet for later. */
  rimeaddr_copy(&receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  buf = queuebuf_new_from_packetbuf();
  ret = mac->send();
  /*  if(ret != MAC_TX_OK) {
    printf("CSMA: err %d\n", ret);
    }*/

  /* Check if we saw a collission, and if we have a queuebuf with the
     packet available. Only retransmit unicast packets. Retransmit
     only once, for now. */
  if((ret == MAC_TX_COLLISION || ret == MAC_TX_NOACK) &&
     buf != NULL && !rimeaddr_cmp(&receiver, &rimeaddr_null)) {
    struct queued_packet *q;

    q = memb_alloc(&packet_memb);
    if(q == NULL) {
      queuebuf_free(buf);
      return ret;
    }
    q->buf = buf;
    q->retransmits = 0;

    if(ret == MAC_TX_COLLISION) {
      /* If the packet wasn't sent because of a collission, we let the
         other packet get through before we try again. */
      time = mac->channel_check_interval();
      if(time == 0) {
        time = CLOCK_SECOND;
      }
      time = time + (random_rand() % (3 * time));
    } else {
      /* If the packet didn't get an ACK, we retransmit immediately. */
      time = 0;
    }
    
    ctimer_set(&q->retransmit_timer, time,
	       retransmit_packet, q);
    list_add(packet_list, q);
  } else {
    queuebuf_free(buf);
  }
  return ret;
}
#else /* CSMA_CONF_REXMIT */
static int
send_packet(void)
{
  return mac->send();
}
#endif /* CSMA_CONF_REXMIT */
/*---------------------------------------------------------------------------*/
static void
input_packet(const struct mac_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&csma_driver);
  }
}
/*---------------------------------------------------------------------------*/
static int
read_packet(void)
{
  int len;
  len = mac->read();
  return len;
}
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return mac->on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return mac->off(keep_radio_on);
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  if(mac->channel_check_interval) {
    return mac->channel_check_interval();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
#define NAMEBUF_LEN 16
static char namebuf[NAMEBUF_LEN];
const struct mac_driver csma_driver = {
  namebuf,
  NULL,
  send_packet,
  read_packet,
  set_receive_function,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
const struct mac_driver *
csma_init(const struct mac_driver *psc)
{
  memb_init(&packet_memb);
  list_init(packet_list);
  mac = psc;
  mac->set_receive_function(input_packet);
  /*  printf("CSMA with MAC %s, channel check rate %d Hz\n", mac->name,
      CLOCK_SECOND / channel_check_interval());*/
  memcpy(namebuf, "CSMA ", 5);
  memcpy(namebuf + 5, psc->name, NAMEBUF_LEN - 6);
  return &csma_driver;
}
/*---------------------------------------------------------------------------*/
