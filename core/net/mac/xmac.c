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
 * $Id: xmac.c,v 1.35 2009/08/20 18:59:17 adamdunkels Exp $
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
#include "dev/watchdog.h"
#include "lib/random.h"

#include "sys/compower.h"

#include "contiki-conf.h"

#include <string.h>

#define WITH_CHANNEL_CHECK           0    /* Seems to work badly when enabled */
#define WITH_TIMESYNCH               0
#define WITH_QUEUE                   0
#define WITH_ACK_OPTIMIZATION        0
#define WITH_RANDOM_WAIT_BEFORE_SEND 0

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

/* The maximum number of announcements in a single announcement
   message - may need to be increased in the future. */
#define ANNOUNCEMENT_MAX 10

/* The structure of the announcement messages. */
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[ANNOUNCEMENT_MAX];
};

/* The length of the header of the announcement message, i.e., the
   "num" field in the struct. */
#define ANNOUNCEMENT_MSG_HEADERLEN (sizeof (uint16_t))

#define TYPE_STROBE       0
#define TYPE_DATA         1
#define TYPE_ANNOUNCEMENT 2
#define TYPE_STROBE_ACK   3

struct xmac_hdr {
  uint16_t type;
  rimeaddr_t sender;
  rimeaddr_t receiver;
};

#ifdef XMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (XMAC_CONF_ON_TIME)
#else
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 200)
#endif

#ifdef XMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (XMAC_CONF_OFF_TIME)
#else
#define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / 2 - DEFAULT_ON_TIME)
#endif

/* The cycle time for announcements. */
#define ANNOUNCEMENT_PERIOD 4 * CLOCK_SECOND

/* The time before sending an announcement within one announcement
   cycle. */
#define ANNOUNCEMENT_TIME (random_rand() % (ANNOUNCEMENT_PERIOD))

#define DEFAULT_STROBE_WAIT_TIME (7 * DEFAULT_ON_TIME / 8)

struct xmac_config xmac_config = {
  DEFAULT_ON_TIME,
  DEFAULT_OFF_TIME,
  20 * DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
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

#if XMAC_CONF_ANNOUNCEMENTS
/* Timers for keeping track of when to send announcements. */
static struct ctimer announcement_cycle_ctimer, announcement_ctimer;

static int announcement_radio_txpower;
#endif /* XMAC_CONF_ANNOUNCEMENTS */

/* Flag that is used to keep track of whether or not we are listening
   for announcements from neighbors. */
static uint8_t is_listening;

static void (* receiver_callback)(const struct mac_driver *);

#if XMAC_CONF_COMPOWER
static struct compower_activity current_packet;
#endif /* XMAC_CONF_COMPOWER */

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
  if(xmac_is_on && radio_is_on == 0) {
    radio_is_on = 1;
    radio->on();
    LEDS_ON(LEDS_RED);
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(xmac_is_on && radio_is_on != 0 && is_listening == 0) {
    radio_is_on = 0;
    radio->off();
    LEDS_OFF(LEDS_RED);
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


  PT_BEGIN(&pt);

  while(1) {
    /* Only wait for some cycles to pass for someone to start sending */
    if(someone_is_sending > 0) {
      someone_is_sending--;
    }

    if(xmac_config.off_time > 0) {
      if(waiting_for_packet == 0) {
	if(we_are_sending == 0) {
	  off();
#if XMAC_CONF_COMPOWER
	  compower_accumulate(&compower_idle_activity);
#endif /* XMAC_CONF_COMPOWER */
	}
      } else {
	waiting_for_packet++;
	if(waiting_for_packet > 2) {
	  /* We should not be awake for more than two consecutive
	     power cycles without having heard a packet, so we turn off
	     the radio. */
	  waiting_for_packet = 0;
	  if(we_are_sending == 0) {
	    off();
	  }
#if XMAC_CONF_COMPOWER
	  compower_accumulate(&compower_idle_activity);
#endif /* XMAC_CONF_COMPOWER */
	}
      }

#if WITH_TIMESYNCH
#define NUM_SLOTS 16
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
      if(xmac_is_on) {
	r = rtimer_set(t, RTIMER_TIME(t) + adjust, 1,
		       (void (*)(struct rtimer *, void *))powercycle, ptr);
      }
#else /* WITH_TIMESYNCH */
      if(xmac_is_on) {
	r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.off_time, 1,
		       (void (*)(struct rtimer *, void *))powercycle, ptr);
      }
#endif /* WITH_TIMESYNCH */
      if(r) {
	PRINTF("xmac: 1 could not set rtimer %d\n", r);
      }
      PT_YIELD(&pt);
    }

    if(we_are_sending == 0 &&
       waiting_for_packet == 0) {
      on();
    }
    if(xmac_is_on) {
      r = rtimer_set(t, RTIMER_TIME(t) + xmac_config.on_time, 1,
		     (void (*)(struct rtimer *, void *))powercycle, ptr);
    }
    if(r) {
      PRINTF("xmac: 3 could not set rtimer %d\n", r);
    }

    PT_YIELD(&pt);
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
#if XMAC_CONF_ANNOUNCEMENTS
static int
parse_announcements(rimeaddr_t *from)
{
  /* Parse incoming announcements */
  struct announcement_msg *adata = packetbuf_dataptr();
  int i;
  
  /*  printf("%d.%d: probe from %d.%d with %d announcements\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], adata->num);*/
  /*  for(i = 0; i < packetbuf_datalen(); ++i) {
    printf("%02x ", ((uint8_t *)packetbuf_dataptr())[i]);
  }
  printf("\n");*/
  
  for(i = 0; i < adata->num; ++i) {
    /*   printf("%d.%d: announcement %d: %d\n",
	  rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	  adata->data[i].id,
	  adata->data[i].value);*/
    
    announcement_heard(from,
		       adata->data[i].id,
		       adata->data[i].value);
  }
  return i;
}
/*---------------------------------------------------------------------------*/
static int
format_announcement(char *hdr)
{
  struct announcement_msg *adata;
  struct announcement *a;
  
  /* Construct the announcements */
  adata = (struct announcement_msg *)hdr;
  
  adata->num = 0;
  for(a = announcement_list();
      a != NULL && adata->num < ANNOUNCEMENT_MAX;
      a = a->next) {
    adata->data[adata->num].id = a->id;
    adata->data[adata->num].value = a->value;
    adata->num++;
  }

  if(adata->num > 0) {
    return ANNOUNCEMENT_MSG_HEADERLEN +
      sizeof(struct announcement_data) * adata->num;
  } else {
    return 0;
  }
}
#endif /* XMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  int strobes;
  struct xmac_hdr hdr;
  int got_strobe_ack = 0;
  struct {
    struct xmac_hdr hdr;
    struct announcement_msg announcement;
  } strobe;
  int len;
  int is_broadcast = 0;

#if WITH_RANDOM_WAIT_BEFORE_SEND
  {
    rtimer_clock_t t = RTIMER_NOW() + (random_rand() % (xmac_config.on_time * 4));
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t));
  }
#endif /* WITH_RANDOM_WAIT_BEFORE_SEND */
  

#if WITH_CHANNEL_CHECK
  /* Check if there are other strobes in the air. */
  waiting_for_packet = 1;
  on();
  t0 = RTIMER_NOW();
  while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_wait_time * 2)) {
    len = radio->read(&strobe.hdr, sizeof(strobe.hdr));
    if(len > 0) {
      someone_is_sending = 1;
    }
  }
  waiting_for_packet = 0;
  
  while(someone_is_sending);

#endif /* WITH_CHANNEL_CHECK */
  
  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;

  off();

  /* Create the X-MAC header for the data packet. We cannot do this
     in-place in the packet buffer, because we cannot be sure of the
     alignment of the header in the packet buffer. */
  hdr.type = TYPE_DATA;
  rimeaddr_copy(&hdr.sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr.receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  if(rimeaddr_cmp(&hdr.receiver, &rimeaddr_null)) {
    is_broadcast = 1;
  }

  /* Copy the X-MAC header to the header portion of the packet
     buffer. */
  packetbuf_hdralloc(sizeof(struct xmac_hdr));
  memcpy(packetbuf_hdrptr(), &hdr, sizeof(struct xmac_hdr));
  packetbuf_compact();

  t0 = RTIMER_NOW();
  strobes = 0;

  LEDS_ON(LEDS_BLUE);

  /* Send a train of strobes until the receiver answers with an ACK. */

  /* Turn on the radio to listen for the strobe ACK. */
  if(!is_broadcast) {
    on();
  }

  watchdog_stop();
  got_strobe_ack = 0;
  for(strobes = 0;
      got_strobe_ack == 0 &&
	RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_time);
      strobes++) {

    t = RTIMER_NOW();

    strobe.hdr.type = TYPE_STROBE;
    rimeaddr_copy(&strobe.hdr.sender, &rimeaddr_node_addr);
    rimeaddr_copy(&strobe.hdr.receiver, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));

    /* Send the strobe packet. */
    radio->send((const uint8_t *)&strobe, sizeof(struct xmac_hdr));

    while(got_strobe_ack == 0 &&
	  RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time)) {
      /* See if we got an ACK */
      if(!is_broadcast) {
	len = radio->read((uint8_t *)&strobe, sizeof(struct xmac_hdr));
	if(len > 0) {
	  if(rimeaddr_cmp(&strobe.hdr.sender, &rimeaddr_node_addr) &&
	     rimeaddr_cmp(&strobe.hdr.receiver, &rimeaddr_node_addr)) {
	    /* We got an ACK from the receiver, so we can immediately send
	       the packet. */
	    got_strobe_ack = 1;
	  }
	}
      }
    }

    /* XXX: turn off radio if we haven't heard an ACK within a
       specified time interval. */

    /*    if(got_strobe_ack == 0) {
      off();
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time));
      on();
      }*/
  }

  /* If we have received the strobe ACK, and we are sending a packet
     that will need an upper layer ACK (as signified by the
     PACKETBUF_ATTR_RELIABLE packet attribute), we keep the radio on. */
  if(got_strobe_ack && (packetbuf_attr(PACKETBUF_ATTR_RELIABLE) ||
			packetbuf_attr(PACKETBUF_ATTR_ERELIABLE))) {

#if WITH_ACK_OPTIMIZATION
    on(); /* Wait for ACK packet */
    waiting_for_packet = 1;
#else /* WITH_ACK_OPTIMIZATION */
    off();
#endif /* WITH_ACK_OPTIMIZATION */

  } else {

    off(); /* shell ping don't seem to work with off() here, so we'll
	     keep it on() for a while. */
  }

  /* Send the data packet. */
  if(is_broadcast || got_strobe_ack) {

    radio->send(packetbuf_hdrptr(), packetbuf_totlen());
  }
  watchdog_start();

  PRINTF("xmac: send (strobes=%u,len=%u,%s), done\n", strobes,
	 packetbuf_totlen(), got_strobe_ack ? "ack" : "no ack");

#if XMAC_CONF_COMPOWER
  /* Accumulate the power consumption for the packet transmission. */
  compower_accumulate(&current_packet);

  /* Convert the accumulated power consumption for the transmitted
     packet to packet attributes so that the higher levels can keep
     track of the amount of energy spent on transmitting the
     packet. */
  compower_attrconv(&current_packet);

  /* Clear the accumulated power consumption so that it is ready for
     the next packet. */
  compower_clear(&current_packet);
#endif /* XMAC_CONF_COMPOWER */
  
  we_are_sending = 0;

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
      queued_packet = queuebuf_new_from_packetbuf();
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
  uint8_t len;

  packetbuf_clear();

  len = radio->read(packetbuf_dataptr(), PACKETBUF_SIZE);
  
  if(len > 0) {
    packetbuf_set_datalen(len);
    hdr = packetbuf_dataptr();

    packetbuf_hdrreduce(sizeof(struct xmac_hdr));

    if(hdr->type == TYPE_STROBE) {
      /* There is no data in the packet so it has to be a strobe. */
      someone_is_sending = 2;
      
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
	/* This is a strobe packet for us. */

	if(rimeaddr_cmp(&hdr->sender, &rimeaddr_node_addr)) {
	  /* If the sender address is our node address, the strobe is
	     a stray strobe ACK to us, which we ignore unless we are
	     currently sending a packet.  */
	  someone_is_sending = 0;
	} else {
	  struct xmac_hdr msg;
	  /* If the sender address is someone else, we should
	     acknowledge the strobe and wait for the packet. By using
	     the same address as both sender and receiver, we flag the
	     message is a strobe ack. */
	  msg.type = TYPE_STROBE_ACK;
	  rimeaddr_copy(&msg.receiver, &hdr->sender);
	  rimeaddr_copy(&msg.sender, &hdr->sender);
	  /* We turn on the radio in anticipation of the incoming
	     packet. */
	  someone_is_sending = 1;
	  waiting_for_packet = 1;
	  on();
	  radio->send((const uint8_t *)&msg, sizeof(struct xmac_hdr));
	}
      } else if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
	/* If the receiver address is null, the strobe is sent to
	   prepare for an incoming broadcast packet. If this is the
	   case, we turn on the radio and wait for the incoming
	   broadcast packet. */
	waiting_for_packet = 1;
	on();
      }

      /* Check for annoucements in the strobe */
      /*      if(packetbuf_datalen() > 0) {
	parse_announcements(&hdr->sender);
	}*/
      /* We are done processing the strobe and we therefore return
	 to the caller. */
      return RIME_OK;
    } else if(hdr->type == TYPE_DATA) {
      someone_is_sending = 0;
      if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr) ||
	 rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
	/* This is a regular packet that is destined to us or to the
	   broadcast address. */
	
	/* We have received the final packet, so we can go back to being
	   asleep. */
	off();

	/* Set sender and receiver packet attributes */
 	if(!rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
	  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &hdr->receiver);
	}
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &hdr->sender);
	
#if XMAC_CONF_COMPOWER
	/* Accumulate the power consumption for the packet reception. */
	compower_accumulate(&current_packet);
	/* Convert the accumulated power consumption for the received
	   packet to packet attributes so that the higher levels can
	   keep track of the amount of energy spent on receiving the
	   packet. */
	compower_attrconv(&current_packet);
	
	/* Clear the accumulated power consumption so that it is ready
	   for the next packet. */
	compower_clear(&current_packet);
#endif /* XMAC_CONF_COMPOWER */
	
	waiting_for_packet = 0;
	
	/* XXX should set timer to send queued packet later. */
	if(queued_packet != NULL) {
	  queuebuf_free(queued_packet);
	  queued_packet = NULL;
	}
	
	return packetbuf_totlen();
      }
#if XMAC_CONF_ANNOUNCEMENTS
    } else if(hdr->type == TYPE_ANNOUNCEMENT) {
      parse_announcements(&hdr->sender);
#endif /* XMAC_CONF_ANNOUNCEMENTS */
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
#if XMAC_CONF_ANNOUNCEMENTS
static void
send_announcement(void *ptr)
{
  struct xmac_hdr *hdr;
  int announcement_len;
  
  /* Set up the probe header. */
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(struct xmac_hdr));
  hdr = packetbuf_dataptr();
  hdr->type = TYPE_ANNOUNCEMENT;
  rimeaddr_copy(&hdr->sender, &rimeaddr_node_addr);
  rimeaddr_copy(&hdr->receiver, &rimeaddr_null);

  announcement_len = format_announcement((char *)hdr +
					 sizeof(struct xmac_hdr));

  if(announcement_len > 0) {
    packetbuf_set_datalen(sizeof(struct xmac_hdr) + announcement_len);

    packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER, announcement_radio_txpower);
    radio->send(packetbuf_hdrptr(), packetbuf_totlen());
  }
}
/*---------------------------------------------------------------------------*/
static void
cycle_announcement(void *ptr)
{
  ctimer_set(&announcement_ctimer, ANNOUNCEMENT_TIME,
	     send_announcement, NULL);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_PERIOD,
	     cycle_announcement, NULL);
  if(is_listening > 0) {
    is_listening--;
    /*    printf("is_listening %d\n", is_listening);*/
  }
}
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods + 1;
}
#endif /* XMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
xmac_set_announcement_radio_txpower(int txpower)
{
#if XMAC_CONF_ANNOUNCEMENTS
  announcement_radio_txpower = txpower;
#endif /* XMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
xmac_init(const struct radio_driver *d)
{
  radio_is_on = 0;
  waiting_for_packet = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);

  xmac_is_on = 1;
  radio = d;
  radio->set_receive_function(input_packet);

#if XMAC_CONF_ANNOUNCEMENTS
  announcement_register_listen_callback(listen_callback);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_TIME,
	     cycle_announcement, NULL);
#endif /* XMAC_CONF_ANNOUNCEMENTS */
  return &xmac_driver;
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  xmac_is_on = 1;
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  xmac_is_on = 0;
  if(keep_radio_on) {
    return radio->on();
  } else {
    return radio->off();
  }
}
/*---------------------------------------------------------------------------*/
const struct mac_driver xmac_driver =
  {
    "X-MAC",
    xmac_init,
    qsend_packet,
    read_packet,
    set_receive_function,
    turn_on,
    turn_off
  };
