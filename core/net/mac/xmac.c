/* XXX: TODO: turn radio off when a packet not destined for us is heard. */

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
 * $Id: xmac.c,v 1.8 2007/10/23 21:27:57 adamdunkels Exp $
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

#include "contiki-conf.h"

#ifdef CHAMELEON
#include "net/chameleon/packattr.h"
#endif

#define WITH_RECEIVER 1
#define WITH_QUEUE 0

/*
#undef XMAC_CONF_ON_TIME
#undef XMAC_CONF_OFF_TIME

#define XMAC_CONF_ON_TIME RTIMER_ARCH_SECOND / 100
#define XMAC_CONF_OFF_TIME RTIMER_ARCH_SECOND / 10
*/

#if WITH_RECEIVER
extern
#else
static
#endif
rimeaddr_t uc_receiver;

struct powercycle {
  struct pt pt;
  struct rtimer rt;
  rtimer_clock_t ontime, offtime;
  int num_strobes;
};

struct xmac_hdr {
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

static struct rtimer rt;
static struct pt pt;

#ifdef XMAC_CONF_ON_TIME
#define ON_TIME (XMAC_CONF_ON_TIME)
#else
#define ON_TIME (RTIMER_ARCH_SECOND / 100)
#endif

#ifdef XMAC_CONF_OFF_TIME
#define OFF_TIME (XMAC_CONF_OFF_TIME)
#else
#define OFF_TIME (ON_TIME * 10)
#endif

#define STROBE_WAIT_TIME (ON_TIME) / 2

static volatile unsigned char should_be_awake = 0;
static volatile unsigned char someone_is_sending = 0;
static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;
static volatile unsigned char long_off = 0;

static const struct radio_driver *radio;

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#define LEDS_TOGGLE(x) leds_toggle(x)
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
    if(should_be_awake == 0) {
      radio->off();
      radio_is_on = 0;
      LEDS_OFF(LEDS_RED);
    } else {
      should_be_awake++;
      if(should_be_awake > 2) {
	should_be_awake = 0;
      }
    }
    r = rtimer_set(t, RTIMER_TIME(t) + OFF_TIME, 1,
		   (void (*)(struct rtimer *, void *))powercycle, ptr);
    if(r) {
      PRINTF("xmac: 1 could not set rtimer %d\n", r);
    }
/*     t2 = rtimer_arch_now(); */
/*     printf("xmac 1 %d\n", t2 - t1); */
    PT_YIELD(&pt);

    if(we_are_sending) {
      PRINTF("xmac: we are sending 1, stopping timer\n");
      PT_YIELD(&pt);
    }

/*     t1 = rtimer_arch_now(); */
    if(radio_is_on == 0) {
      radio->on();
      radio_is_on = 1;
      LEDS_ON(LEDS_RED);
    }
    r = rtimer_set(t, RTIMER_TIME(t) + ON_TIME, 1,
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
      
      /* XXX should wait for a complete packet that is not destined to
	 us to swisch past us. */
      radio->off();
      radio_is_on = 0;
      LEDS_OFF(LEDS_RED);
      if(rtimer_set(t, RTIMER_TIME(t) + OFF_TIME, 1,
		    (void (*)(struct rtimer *, void *))powercycle, ptr)) {
	PRINTF("xmac: could not set long off rtimer\n");
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
  rtimer_clock_t t0, t1;
  int strobes;
  struct xmac_hdr *hdr;
  int got_ack = 0;
  struct xmac_hdr msg;
  rtimer_clock_t t;
  int len;

  we_are_sending = 1;

  radio->on();
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

  do {

    t = RTIMER_NOW();
    
    LEDS_TOGGLE(LEDS_GREEN);
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

    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t + STROBE_WAIT_TIME)) {
      printf("now %d then %d wait %d\n",
	     RTIMER_NOW(), t, STROBE_WAIT_TIME);
      /* See if we got an ACK */
      /*      printf("befor read\n");*/
      len = radio->read((u8_t *)&msg, sizeof(struct xmac_hdr));
      /*      printf("after read len %d\n", len);*/
      if(len > 0) {
	PRINTF("xmac len %d sender %d.%d receiver %d.%d node %d.%d.\n", len,
		msg.sender.u8[0],msg.sender.u8[1],
		msg.receiver.u8[0],msg.receiver.u8[1],
		rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
	if(rimeaddr_cmp(&msg.receiver, &rimeaddr_node_addr)) {
	  
	  /* We got an ACK from the receiver, so we can immediately send
	     the packet. */
	  got_ack = 1;
	  break;
	} else {
	  RIMESTATS_ADD(sendingdrop);
	}
      }
    }
    
    /*    PRINTF("xmac Strobe %d got_ack %d\n", strobes, got_ack);*/
    ++strobes;
    if(got_ack) {
      break;
    }
  } while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + OFF_TIME + ON_TIME));
  
  radio->send(rimebuf_hdrptr(), rimebuf_totlen());

  PRINTF("xmac: send, done\n");

  /*  printf("Strobe %d got_ack %d\n", strobes, got_ack);*/

  LEDS_OFF(LEDS_GREEN);
  PT_INIT(&pt);
  if(rtimer_set(&rt, RTIMER_NOW() + ON_TIME, 1,
		(void (*)(struct rtimer *, void *))powercycle, NULL)) {
    PRINTF("xmac: could not set rtimer after send\n");
  }
  we_are_sending = 0;
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
      RIMESTATS_ADD(contentiondrop);
      return 0;
    } else {
#if WITH_QUEUE
      queued_packet = queuebuf_new_from_rimebuf();
      return 1;
#else
      RIMESTATS_ADD(contentiondrop);
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
    someone_is_sending = 1;
    rimebuf_set_datalen(len);
    hdr = rimebuf_dataptr();
    
    rimebuf_hdrreduce(sizeof(struct xmac_hdr));
    if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
      should_be_awake = 1;
      PRINTF("xmac: for us\n");
      
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
      someone_is_sending = 1;
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
/*---------------------------------------------------------------------------*/
const struct mac_driver *
xmac_init(const struct radio_driver *d)
{
  radio_is_on = 0;
  should_be_awake = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + OFF_TIME, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);

  radio = d;
  radio->set_receive_function(input);
  return &xmac_driver;
}
/*---------------------------------------------------------------------------*/
