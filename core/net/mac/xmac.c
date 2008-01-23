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
 * $Id: xmac.c,v 1.14 2008/01/23 15:00:54 adamdunkels Exp $
 */

/**
 * \file
 *         A simple power saving MAC protocol based on X-MAC [SenSys 2006]
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/pt.h"
#include "net/mac/xmac.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "net/rime.h"
#include "net/rime/timesynch.h"
#include "dev/radio.h"
#include "lib/random.h"
/*#include "lib/bb.h"*/

#include "sys/timetable.h"

#include "contiki-conf.h"

#if CHAMELEON
#include "net/chameleon/packattr.h"
#endif

#define WITH_CHANNEL_CHECK 0   /* Seems to work bad */
#define WITH_TIMESYNCH 1
#define WITH_RECEIVER 1
#define WITH_QUEUE 0

#if !CHAMELEON
#if WITH_RECEIVER
extern
#else
static
#endif
rimeaddr_t uc_receiver;
#endif /* !CHAMELEON */

struct xmac_hdr {
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

#ifdef XMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (XMAC_CONF_ON_TIME)
#else
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 100)
#endif

#ifdef XMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (XMAC_CONF_OFF_TIME)
#else
#define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / 2 - DEFAULT_ON_TIME)
#endif

#define DEFAULT_STROBE_WAIT_TIME (DEFAULT_ON_TIME / 2)

struct xmac_config xmac_config = {
  DEFAULT_ON_TIME,
  DEFAULT_OFF_TIME,
  2 * DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
  DEFAULT_STROBE_WAIT_TIME
};

#include <stdio.h>
static struct rtimer rt;
static struct pt pt;

static int xmac_is_on = 0;

static volatile unsigned char waiting_for_packet = 0;
static volatile unsigned char someone_is_sending = 0;
static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;

static const struct radio_driver *radio;

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE


#define CPRINTF(...)

#define BB_SET(x,y)
#define BB_INC(x,y)
/* #define BB_SET(x,y) bb_set(x,y) */
/* #define BB_INC(x,y) bb_inc(x,y) */
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#define LEDS_TOGGLE(x) leds_toggle(x)
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE
#define LEDS_ON(x)
#define LEDS_OFF(x)
#define LEDS_TOGGLE(x)
#define PRINTF(...)
#endif

static void (* receiver_callback)(const struct mac_driver *);

#define xmac_timetable_size 256
TIMETABLE_NONSTATIC(xmac_timetable);
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(radio_is_on == 0) {
    radio_is_on = 1;
    radio->on();
    LEDS_ON(LEDS_RED);
    CPRINTF("/");
    TIMETABLE_TIMESTAMP(xmac_timetable, "on");
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(radio_is_on != 0) {
    radio_is_on = 0;
    radio->off();
    TIMETABLE_TIMESTAMP(xmac_timetable, "off");
    LEDS_OFF(LEDS_RED);
    CPRINTF("\\");
  }
}
/*---------------------------------------------------------------------------*/
static char
powercycle(struct rtimer *t, void *ptr)
{
  int r;
#if WITH_TIMESYNCH
  rtimer_clock_t should_be, adjust;
#endif /* WITH_TIMESYNCH */

  CPRINTF("*");
  
  PT_BEGIN(&pt);

  while(1) {
    /* Only wait for some cycles to pass for someone to start sending */
    if(someone_is_sending > 0) {
      someone_is_sending--;
    }

    if(xmac_config.off_time > 0) {
      if(waiting_for_packet == 0) {
	if(xmac_is_on &&
	   we_are_sending == 0) {
	  off();
	}
      } else {
	waiting_for_packet++;
	if(waiting_for_packet > 2) {
	  /* We should not be awake for more than two consecutive
	     power cycles without having heard a packet, so we turn off
	     the radio. */
	  waiting_for_packet = 0;
	  TIMETABLE_TIMESTAMP(xmac_timetable, "off waiting");
	  off();
	}
      }

#if WITH_TIMESYNCH
#define NUM_SLOTS 8
      should_be = ((timesynch_rtimer_to_time(RTIMER_TIME(t)) +
		    xmac_config.off_time) &
		   ~(xmac_config.off_time + xmac_config.on_time - 1)) +
	(rimeaddr_node_addr.u8[0] % NUM_SLOTS *
	 ((xmac_config.off_time + xmac_config.on_time) / NUM_SLOTS));

      should_be = timesynch_time_to_rtimer(should_be);

      if(should_be - RTIMER_TIME(t) > xmac_config.off_time) {
	adjust = xmac_config.off_time / 2;
      } else {
	adjust = should_be - RTIMER_TIME(t);
      }
      r = rtimer_set(t, RTIMER_TIME(t) + adjust, 1,
		     (void (*)(struct rtimer *, void *))powercycle, ptr);
#else /* WITH_TIMESYNCH */
      r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.off_time, 1,
		     (void (*)(struct rtimer *, void *))powercycle, ptr);
#endif /* WITH_TIMESYNCH */
      if(r) {
	PRINTF("xmac: 1 could not set rtimer %d\n", r);
      }
      PT_YIELD(&pt);
    }

    if(xmac_is_on &&
       we_are_sending == 0 &&
       waiting_for_packet == 0) {
      on();
    }
    r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.on_time, 1,
		   (void (*)(struct rtimer *, void *))powercycle, ptr);
    if(r) {
      PRINTF("xmac: 3 could not set rtimer %d\n", r);
    }

    PT_YIELD(&pt);
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  int strobes;
  struct xmac_hdr *hdr;
  int got_ack = 0;
  struct xmac_hdr msg;
  int len;

  TIMETABLE_TIMESTAMP(xmac_timetable, "send");
#if WITH_CHANNEL_CHECK
  /* Check if there are other strobes in the air. */
  waiting_for_packet = 1;
  on();
  t0 = RTIMER_NOW();
  while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_wait_time * 2)) {
    len = radio->read(&msg, sizeof(msg));
    if(len > 0) {
      someone_is_sending = 1;
    }
  }
  waiting_for_packet = 0;
  
  while(someone_is_sending); /* {printf("z");}*/

  TIMETABLE_TIMESTAMP(xmac_timetable, "send 2");
#endif /* WITH_CHANNEL_CHECK */
  
  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;

  off();

  
  rimebuf_hdralloc(sizeof(struct xmac_hdr));
  hdr = rimebuf_hdrptr();
  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
#if CHAMELEON
  rimeaddr_copy(&hdr->receiver, packattr_aget(PACKATTR_RECEIVER));
#else
  rimeaddr_copy(&hdr->receiver, &uc_receiver);
#endif
  rimebuf_compact();

  t0 = RTIMER_NOW();
  strobes = 0;

  BB_SET(XMAC_RECEIVER, hdr->receiver.u16[0]);
  
  LEDS_ON(LEDS_BLUE);

  /* Send a train of strobes until the receiver answers with an ACK. */

  /* Turn on the radio to listen for the strobe ACK. XXX for
     broadcasts, don't turn radio on at all. */
  on();

  got_ack = 0;
  for(strobes = 0;
      got_ack == 0 &&
	RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_time);
      strobes++) {

    t = RTIMER_NOW();

    rimeaddr_copy(&msg.sender, &rimeaddr_node_addr);
#if CHAMELEON
    rimeaddr_copy(&msg.receiver, packattr_aget(PACKATTR_RECEIVER));
#else
    rimeaddr_copy(&msg.receiver, &uc_receiver);
#endif

    TIMETABLE_TIMESTAMP(xmac_timetable, "send strobe");
    /* Send the strobe packet. */
    radio->send((const u8_t *)&msg, sizeof(struct xmac_hdr));
    CPRINTF("+");

    while(got_ack == 0 &&
	  RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time)) {
      /* See if we got an ACK */
      len = radio->read((u8_t *)&msg, sizeof(struct xmac_hdr));
      if(len > 0) {
	CPRINTF("_");
	if(rimeaddr_cmp(&msg.sender, &rimeaddr_node_addr) &&
	   rimeaddr_cmp(&msg.receiver, &rimeaddr_node_addr)) {
	  TIMETABLE_TIMESTAMP(xmac_timetable, "send ack received");
	  CPRINTF("@");
	  /* We got an ACK from the receiver, so we can immediately send
	     the packet. */
	  got_ack = 1;
	}
      }
    }

    /* XXX: turn off radio if we haven't heard an ACK within a
       specified time interval. */

    /*    if(got_ack == 0) {
      off();
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time));
      on();
      }*/
  }

  if(got_ack /* XXX && needs_ack */) {
    TIMETABLE_TIMESTAMP(xmac_timetable, "send got ack");
    on(); /* Wait for possible ACK packet */
  } else {
    on(); /* shell ping don't seem to work with off() here, so we'll
	     keep it on() for a while. */
  }

  if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null) || got_ack) {
    TIMETABLE_TIMESTAMP(xmac_timetable, "send packet");
    radio->send(rimebuf_hdrptr(), rimebuf_totlen());
    CPRINTF("#");
  }

  PRINTF("xmac: send (strobes=%u,len=%u,%s), done\n", strobes,
	 rimebuf_totlen(), got_ack ? "ack" : "no ack");

  BB_SET(XMAC_STROBES, strobes);
  if(got_ack) {
    BB_INC(XMAC_SEND_WITH_ACK, 1);
  } else {
    BB_INC(XMAC_SEND_WITH_NOACK, 1);
  }

  /*  printf("Strobe %d got_ack %d\n", strobes, got_ack);*/

  we_are_sending = 0;
  TIMETABLE_TIMESTAMP(xmac_timetable, "send we_are_sending = 0");
  LEDS_OFF(LEDS_BLUE);
  return 1;

}
/*---------------------------------------------------------------------------*/
static struct queuebuf *queued_packet;
static int
qsend_packet(void)
{
  if(someone_is_sending) {
    PRINTF("xmac: should queue packet, now just dropping %d %d %d %d.\n",
	   waiting_for_packet, someone_is_sending, we_are_sending, radio_is_on);
    if(queued_packet != NULL) {
      RIMESTATS_ADD(sendingdrop);
      return 0;
    } else {
#if WITH_QUEUE
      queued_packet = queuebuf_new_from_rimebuf();
      return 1;
#else
      RIMESTATS_ADD(sendingdrop);
      return 0;
#endif
    }
  } else {
    PRINTF("xmac: send immediately.\n");
    return send_packet();
  }

}
/*---------------------------------------------------------------------------*/
static void
input_packet(const struct radio_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&xmac_driver);
  }
}
/*---------------------------------------------------------------------------*/
static int
read_packet(void)
{
  struct xmac_hdr *hdr;
  u8_t len;

  rimebuf_clear();

  len = radio->read(rimebuf_dataptr(), RIMEBUF_SIZE);

  if(len > 0) {
    rimebuf_set_datalen(len);
    hdr = rimebuf_dataptr();

    rimebuf_hdrreduce(sizeof(struct xmac_hdr));

    if(rimebuf_totlen() == 0) {
      CPRINTF(".");
      /* There is no data in the packet so it has to be a strobe. */
      someone_is_sending = 2;
      
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
	/* This is a strobe packet for us. */

	if(rimeaddr_cmp(&hdr->sender, &rimeaddr_node_addr)) {
	  /* If the sender address is our node address, the strobe is
	     a stray strobe ACK to us, which we ignore unless we are
	     currently sending a packet.  */
	  CPRINTF("&");
	  someone_is_sending = 0;
	} else {
	  struct xmac_hdr msg;
	  /* If the sender address is someone else, we should
	     acknowledge the strobe and wait for the packet. By using
	     the same address as both sender and receiver, we flag the
	     message is a strobe ack. */
	  TIMETABLE_TIMESTAMP(xmac_timetable, "read send ack");
	  rimeaddr_copy(&msg.receiver, &hdr->sender);
	  rimeaddr_copy(&msg.sender, &hdr->sender);
	  CPRINTF("!");
	  /* We turn on the radio in anticipation of the incoming
	     packet. */
	  someone_is_sending = 1;
	  waiting_for_packet = 1;
	  on();
	  radio->send((const u8_t *)&msg, sizeof(struct xmac_hdr));
	}
      } else if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
	/* If the receiver address is null, the strobe is sent to
	   prepare for an incoming broadcast packet. If this is the
	   case, we turn on the radio and wait for the incoming
	   broadcast packet. */
	waiting_for_packet = 1;
	on();
      }
      /* We are done processing the strobe and we therefore return
	 to the caller. */
      return RIME_OK;
    } else {
      CPRINTF("-");
      someone_is_sending = 0;
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr) ||
	 rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
	TIMETABLE_TIMESTAMP(xmac_timetable, "read got packet");
	/* This is a regular packet that is destined to us or to the
	   broadcast address. */
	
	/* We have received the final packet, so we can go back to being
	   asleep. */
	off();
	waiting_for_packet = 0;
	
	/* XXX should set timer to send queued packet later. */
	if(queued_packet != NULL) {
	  queuebuf_free(queued_packet);
	  queued_packet = NULL;
	}
	
	return rimebuf_totlen();
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
xmac_init(const struct radio_driver *d)
{
  timetable_clear(&xmac_timetable);
  radio_is_on = 0;
  waiting_for_packet = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);

  xmac_is_on = 1;
  radio = d;
  radio->set_receive_function(input_packet);

  BB_SET("xmac.state_addr", (int) &waiting_for_packet);
  BB_SET(XMAC_RECEIVER, 0);
  BB_SET(XMAC_STROBES, 0);
  BB_SET(XMAC_SEND_WITH_ACK, 0);
  BB_SET(XMAC_SEND_WITH_NOACK, 0);
  return &xmac_driver;
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  xmac_is_on = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(void)
{
  xmac_is_on = 0;
  return radio->off();
}
/*---------------------------------------------------------------------------*/
const struct mac_driver xmac_driver =
  {
    qsend_packet,
    read_packet,
    set_receive_function,
    turn_on,
    turn_off
  };
