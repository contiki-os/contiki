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
 * $Id: xmac.c,v 1.9 2007/11/12 22:29:37 adamdunkels Exp $
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
#include "dev/radio.h"
#include "lib/random.h"
/*#include "lib/bb.h"*/

#include "contiki-conf.h"

#if CHAMELEON
#include "net/chameleon/packattr.h"
#endif

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

/* struct powercycle { */
/*   struct pt pt; */
/*   struct rtimer rt; */
/*   rtimer_clock_t ontime, offtime; */
/*   int num_strobes; */
/* }; */

struct xmac_hdr {
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

#ifdef XMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (XMAC_CONF_ON_TIME)
#else
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 50)
#endif

#ifdef XMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (XMAC_CONF_OFF_TIME)
#else
#define DEFAULT_OFF_TIME ((RTIMER_ARCH_SECOND / 5) - DEFAULT_ON_TIME)
#endif

#define DEFAULT_STROBE_WAIT_TIME (DEFAULT_ON_TIME) / 2
struct xmac_config xmac_config = {
  DEFAULT_ON_TIME, DEFAULT_OFF_TIME,
  DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
  DEFAULT_STROBE_WAIT_TIME
};

#include <stdio.h>
static struct rtimer rt;
static struct pt pt;

static volatile unsigned char should_be_awake = 0;
static volatile unsigned char someone_is_sending = 0;
static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;
static volatile unsigned char long_off = 0;

static const struct radio_driver *radio;

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE

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
#define LEDS_ON(x)
#define LEDS_OFF(x)
#define LEDS_TOGGLE(x)
#define PRINTF(...)
#endif

static void (* receiver_callback)(const struct mac_driver *);
/*---------------------------------------------------------------------------*/
static void
set_receive_function(void (* recv)(const struct mac_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
static char
powercycle(struct rtimer *t, void *ptr)
{
  int r;

  PT_BEGIN(&pt);

  while(1) {
/*     rtimer_clock_t t1, t2; */

/*     t1 = rtimer_arch_now(); */
    if(we_are_sending) {
      PRINTF("xmac: we are sending 1, stopping timer\n");
      PT_YIELD(&pt);
    }

    /* Only wait for some cycles to pass for someone to start sending */
    if(someone_is_sending > 0) {
      someone_is_sending--;
    }

    if(xmac_config.off_time > 0) {
      if(should_be_awake == 0) {
	radio->off();
	LEDS_OFF(LEDS_RED);
	radio_is_on = 0;
      } else {
	should_be_awake++;
	if(should_be_awake > 2) {
	  should_be_awake = 0;
	}
      }
      r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.off_time, 1,
		     (void (*)(struct rtimer *, void *))powercycle, ptr);
      if(r) {
	PRINTF("xmac: 1 could not set rtimer %d\n", r);
      }
/*     t2 = rtimer_arch_now(); */
/*     printf("xmac 1 %d\n", t2 - t1); */
      PT_YIELD(&pt);
    }

    if(we_are_sending) {
      PRINTF("xmac: we are sending 1, stopping timer\n");
      PT_YIELD(&pt);
    }

/*     t1 = rtimer_arch_now(); */
    if(radio_is_on == 0) {
      radio->on();
      LEDS_ON(LEDS_RED);
      radio_is_on = 1;
    }
    r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.on_time, 1,
		   (void (*)(struct rtimer *, void *))powercycle, ptr);
    if(r) {
      PRINTF("xmac: 2 could not set rtimer %d\n", r);
    }
/*     t2 = rtimer_arch_now(); */
/*     printf("xmac 2 %d\n", t2 - t1); */

    PT_YIELD(&pt);

    if(we_are_sending) {
      PRINTF("xmac: we are sending 2, stopping timer\n");
      PT_YIELD(&pt);
    }

    if(long_off) {

      if(xmac_config.off_time > 0) {
	/* XXX should wait for a complete packet that is not destined to
	   us to swisch past us. */
	radio->off();
	LEDS_OFF(LEDS_RED);
	radio_is_on = 0;
	if(rtimer_set(t, RTIMER_TIME(t) + xmac_config.off_time, 1,
		      (void (*)(struct rtimer *, void *))powercycle, ptr)) {
	  PRINTF("xmac: could not set long off rtimer\n");
	}
      }
      long_off = 0;
      someone_is_sending = 0;
    }

  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
static int
send(void)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  int strobes;
  struct xmac_hdr *hdr;
  int got_ack = 0;
  struct xmac_hdr msg;
  int len;

  we_are_sending = 1;

  radio->on();
  LEDS_ON(LEDS_RED);
  radio_is_on = 1;

  /*  printf("xmac_send\n");*/
#if !CHAMELEON
  PRINTF("xmac: send() len %d to %d.%d\n", rimebuf_totlen(),
	 uc_receiver.u8[0], uc_receiver.u8[1]);
#endif

  rimebuf_hdralloc(sizeof(struct xmac_hdr));
  hdr = rimebuf_hdrptr();
  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
#if CHAMELEON
  rimeaddr_copy(&hdr->receiver, packattr_aget(PACKATTR_RECEIVER));
#else
  rimeaddr_copy(&hdr->receiver, &uc_receiver);
#endif
  rimebuf_compact();

  /*  should_be_awake = 1;
      while(!radio_is_on) {}*/

  t0 = RTIMER_NOW();
  strobes = 0;

  BB_SET(XMAC_RECEIVER, hdr->receiver.u16[0]);

  LEDS_ON(LEDS_YELLOW);
  do {

    if(strobes & 1) {
      LEDS_OFF(LEDS_GREEN);
    } else {
      LEDS_ON(LEDS_GREEN);
    }

    t = RTIMER_NOW();

    rimeaddr_copy(&msg.sender, &rimeaddr_node_addr);
#if CHAMELEON
    rimeaddr_copy(&msg.receiver, packattr_aget(PACKATTR_RECEIVER));
#else
    rimeaddr_copy(&msg.receiver, &uc_receiver);
#endif

    /*    printf("sender %d.%d receiver %d.%d node %d.%d.\n",
	   msg.sender.u8[0],msg.sender.u8[1],
	   msg.receiver.u8[0],msg.receiver.u8[1],
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);*/
    radio->send((const u8_t *)&msg, sizeof(struct xmac_hdr));

    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time)) {
      PRINTF("xmac: now %u then %u wait %d\n",
	     RTIMER_NOW(), t, xmac_config.strobe_wait_time);
      /* See if we got an ACK */
      /*      printf("before read\n");*/
      len = radio->read((u8_t *)&msg, sizeof(struct xmac_hdr));

      /*      printf("after read len %d\n", len);*/
      if(len > 0) {
	PRINTF("xmac: len %d sender %d.%d receiver %d.%d node %d.%d.\n", len,
		msg.sender.u8[0],msg.sender.u8[1],
		msg.receiver.u8[0],msg.receiver.u8[1],
		rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
/* 	if(rimeaddr_cmp(&msg.receiver, &rimeaddr_node_addr)) { */
	if(rimeaddr_cmp(&msg.sender, &rimeaddr_node_addr) &&
	   rimeaddr_cmp(&msg.receiver, &rimeaddr_node_addr)) {

	  /* We got an ACK from the receiver, so we can immediately send
	     the packet. */
	  got_ack = 1;
	  break;

/* 	} else if(rimeaddr_cmp(&msg.sender, &hdr->receiver)) { */
/* 	  /\* We did not get an ACK from the receiver, but the receiver */
/* 	     is active so we can send the packet after a short random */
/* 	     duration. *\/ */
/* 	  clock_delay(random_rand() & 0x1ff); */
/* 	  got_ack = 1; */
/* 	  break; */

	} else {
	  /*	  RIMESTATS_ADD(sendingdrop);*/
	}
      }
    }

    /*    PRINTF("xmac Strobe %d got_ack %d\n", strobes, got_ack);*/
    ++strobes;
    if(got_ack) {
      break;
    }
  } while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_time));
  LEDS_OFF(LEDS_YELLOW);

  radio->send(rimebuf_hdrptr(), rimebuf_totlen());

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
  LEDS_OFF(LEDS_GREEN);
  PT_INIT(&pt);
  if((len = rtimer_set(&rt, RTIMER_NOW() + xmac_config.on_time, 1,
		(void (*)(struct rtimer *, void *))powercycle, NULL)) != 0) {
    PRINTF("xmac: could not set rtimer after send (%d)\n", len);
  }
/*   we_are_sending = 0; */
  return 1;

}
/*---------------------------------------------------------------------------*/
static struct queuebuf *queued_packet;
static int
qsend(void)
{
  if(someone_is_sending) {
    PRINTF("xmac: should queue packet, now just dropping %d %d %d %d.\n",
	   should_be_awake, someone_is_sending, we_are_sending, radio_is_on);
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
    return send();
  }

}
/*---------------------------------------------------------------------------*/
static void
input(const struct radio_driver *d)
{
  if(receiver_callback) {
    receiver_callback(&xmac_driver);
  }
}
/*---------------------------------------------------------------------------*/
static int
read(void)
{
  struct xmac_hdr *hdr;
  u8_t len;

  rimebuf_clear();

  PRINTF("xmac: input ");
  len = radio->read(rimebuf_dataptr(), RIMEBUF_SIZE);
  PRINTF("%d bytes\n", len);

  if(len > 0) {
    someone_is_sending = 2;
    rimebuf_set_datalen(len);
    hdr = rimebuf_dataptr();

    rimebuf_hdrreduce(sizeof(struct xmac_hdr));
    if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
      should_be_awake = 1;
      PRINTF("xmac: for us\n");

      /* Only send strobe ack if the packet itself is not a strobe ack
	 to us. */
      if(!rimeaddr_cmp(&hdr->sender, &rimeaddr_node_addr) &&
	 rimebuf_totlen() == 0) {
	/* XXX Send an ACK and wait for packet . */
	PRINTF("xmac: got sender %d.%d receiver %d.%d\n",
	       hdr->sender.u8[0],hdr->sender.u8[1],
	       hdr->receiver.u8[0],hdr->receiver.u8[1]);
	while(!radio_is_on) {}

	{
	  struct xmac_hdr msg;
	  rimeaddr_copy(&msg.receiver, &hdr->sender);
	  /* Using the same address as both sender and receiver will
	     flag the message as a strobe ack. */
	  rimeaddr_copy(&msg.sender, &hdr->sender);
	  PRINTF("xmac: sending sender %d.%d receiver %d.%d\n",
		 msg.sender.u8[0],msg.sender.u8[1],
		 msg.receiver.u8[0],msg.receiver.u8[1]);
	  radio->send((const u8_t *)&msg, sizeof(struct xmac_hdr));
	}
      }
    } else if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
      /* Wait for the actual packet to arrive. */
      should_be_awake = 1;
      PRINTF("xmac: broadcast from %d.%d\n",
	     hdr->sender.u8[0],hdr->sender.u8[1]);
      /*      PRINTF("xmac: broadcast\n");*/
    } else {
      /* Go back to sleep. XXX should not turn on the radio again
	 until this packet has passed.  */
      should_be_awake = 0;
      long_off = 1;
      /* XXX set timer to turn someone_is_sending off again and send queued packet. */
      /*      PRINTF("xmac: not for us\n");*/
    }

    if(rimebuf_totlen() > 0) {

      /* We have received the final packet, so we can go back to being
	 asleep. */
      someone_is_sending = 0;
      should_be_awake = 0;
      PRINTF("xmac: final packet received\n");

      /* XXX should set timer to send queued packet later. */
      if(queued_packet != NULL) {
	queuebuf_free(queued_packet);
	queued_packet = NULL;
      }

      return rimebuf_totlen();
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
xmac_init(const struct radio_driver *d)
{
  radio_is_on = 0;
  should_be_awake = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);

  radio = d;
  radio->set_receive_function(input);

  BB_SET("xmac.state_addr", (int) &should_be_awake);
  BB_SET(XMAC_RECEIVER, 0);
  BB_SET(XMAC_STROBES, 0);
  BB_SET(XMAC_SEND_WITH_ACK, 0);
  BB_SET(XMAC_SEND_WITH_NOACK, 0);
  return &xmac_driver;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return radio->on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  return radio->off();
}
/*---------------------------------------------------------------------------*/
const struct mac_driver xmac_driver =
  {
    qsend,
    read,
    set_receive_function,
    on,
    off
  };
