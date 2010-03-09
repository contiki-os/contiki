/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: contikimac.c,v 1.10 2010/03/09 20:37:02 adamdunkels Exp $
 */

/**
 * \file
 *         The Contiki power-saving MAC protocol (ContikiMAC)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "net/netstack.h"
#include "dev/leds.h"
#include "dev/radio.h"
#include "dev/watchdog.h"
#include "lib/random.h"
#include "net/mac/contikimac.h"
#include "net/rime.h"
#include "sys/compower.h"
#include "sys/pt.h"
#include "sys/rtimer.h"

#include "contiki-conf.h"

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

#include <string.h>

#ifndef WITH_ACK_OPTIMIZATION
#define WITH_ACK_OPTIMIZATION        0
#endif
#ifndef WITH_PHASE_OPTIMIZATION
#define WITH_PHASE_OPTIMIZATION      1
#endif
#ifndef WITH_STREAMING
#define WITH_STREAMING               1
#endif

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

#define DISPATCH          0
#define TYPE_ANNOUNCEMENT 0x12

#ifdef CONTIKIMAC_CONF_CYCLE_TIME
#define CYCLE_TIME (CONTIKIMAC_CONF_CYCLE_TIME)
#else
#define CYCLE_TIME (RTIMER_ARCH_SECOND / MAC_CHANNEL_CHECK_RATE)
#endif

#define CCA_COUNT_MAX 2
#define CCA_CHECK_TIME                     RTIMER_ARCH_SECOND / 8192
#define CCA_SLEEP_TIME                     RTIMER_ARCH_SECOND / 2000 + CCA_CHECK_TIME
#define CHECK_TIME (CCA_COUNT_MAX * (CCA_CHECK_TIME + CCA_SLEEP_TIME))

#define INTER_PACKET_INTERVAL              RTIMER_ARCH_SECOND / 2500
#define AFTER_ACK_DETECTECT_WAIT_TIME      RTIMER_ARCH_SECOND / 1500

#define LISTEN_TIME_AFTER_PACKET_DETECTED  RTIMER_ARCH_SECOND / 100

#define SHORTEST_PACKET_SIZE            18

/* The cycle time for announcements. */
#define ANNOUNCEMENT_PERIOD 4 * CLOCK_SECOND

/* The time before sending an announcement within one announcement
   cycle. */
#define ANNOUNCEMENT_TIME (random_rand() % (ANNOUNCEMENT_PERIOD))

#define STROBE_WAIT_TIME (RTIMER_ARCH_SECOND / 600)

#define STROBE_TIME (CYCLE_TIME + 8 * CHECK_TIME)

#define ACK_LEN 3

#include <stdio.h>
static struct rtimer rt;
static struct pt pt;

static volatile uint8_t contikimac_is_on = 0;

static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif

#if CONTIKIMAC_CONF_ANNOUNCEMENTS
/* Timers for keeping track of when to send announcements. */
static struct ctimer announcement_cycle_ctimer, announcement_ctimer;

static int announcement_radio_txpower;
#endif /* CONTIKIMAC_CONF_ANNOUNCEMENTS */

/* Flag that is used to keep track of whether or not we are listening
   for announcements from neighbors. */
static uint8_t is_listening;

#if CONTIKIMAC_CONF_COMPOWER
static struct compower_activity current_packet;
#endif /* CONTIKIMAC_CONF_COMPOWER */

#if WITH_PHASE_OPTIMIZATION

#include "net/mac/phase.h"

#define MAX_PHASE_NEIGHBORS 30

PHASE_LIST(phase_list, MAX_PHASE_NEIGHBORS);

#endif /* WITH_PHASE_OPTIMIZATION */

static uint8_t is_streaming;
static rimeaddr_t is_streaming_to, is_streaming_to_too;
static rtimer_clock_t stream_until;

#define DEFAULT_STREAM_TIME (RTIMER_ARCH_SECOND)

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

static volatile uint8_t packet_indication_flag;
static volatile uint16_t packet_indicated_but_not_received,
  packet_indicated_but_not_received_clear_third_time;

volatile uint16_t
packet_indicated_but_not_received_clear_second_time;

struct cca_checks_stats {
  volatile char check[CCA_COUNT_MAX];
};

#define LATEST_CCA_CHECKS_NUM 16
static volatile struct cca_checks_stats cca_checks[LATEST_CCA_CHECKS_NUM];
static volatile struct cca_checks_stats *latest_cca_check = &cca_checks[LATEST_CCA_CHECKS_NUM - 1];

/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(contikimac_is_on && radio_is_on == 0) {
    radio_is_on = 1;
    NETSTACK_RADIO.on();
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(contikimac_is_on && radio_is_on != 0 && is_listening == 0 && is_streaming == 0) {
    radio_is_on = 0;
    NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
static char powercycle(struct rtimer *t, void *ptr);
static void
schedule_powercycle(struct rtimer *t, rtimer_clock_t time)
{
  int r;

  if(contikimac_is_on) {

    if(RTIMER_CLOCK_LT(RTIMER_TIME(t) + time, RTIMER_NOW())) {
      time = RTIMER_NOW() - RTIMER_TIME(t);
    }
    
    while(RTIMER_TIME(t) + time == RTIMER_NOW() ||
          RTIMER_TIME(t) + time == RTIMER_NOW() + 1) {
      ++time;
    }
    
#if NURTIMER
    r = rtimer_reschedule(t, time, 1);
#else
    r = rtimer_set(t, RTIMER_TIME(t) + time, 1,
                   (void (*)(struct rtimer *, void *))powercycle, NULL);
#endif
    if(r != RTIMER_OK) {
      printf("schedule_powercycle: could not set rtimer\n");
    }
  }
}
static void
powercycle_turn_radio_off(void)
{
  if(we_are_sending == 0) {
    off();
  }
}
static void
powercycle_turn_radio_on(void)
{
  if(we_are_sending == 0) {
    on();
  }
}
static char
powercycle(struct rtimer *t, void *ptr)
{
  rtimer_clock_t start;

  if(is_streaming) {
    start = RTIMER_NOW();
#if NURTIMER
    if(!RTIMER_CLOCK_LT(start, RTIMER_NOW(), stream_until))
#else
    if(!RTIMER_CLOCK_LT(RTIMER_NOW(), stream_until))
#endif
    {
      is_streaming = 0;
      rimeaddr_copy(&is_streaming_to, &rimeaddr_null);
      rimeaddr_copy(&is_streaming_to_too, &rimeaddr_null);
    }
  }
  
  PT_BEGIN(&pt);

  while(1) {
    static uint8_t packet_seen;
    static rtimer_clock_t t0, cycle_start;
    static uint8_t count;
    static uint8_t clear_second_time, clear_third_time;

    cycle_start = RTIMER_NOW();

    for(count = 1; count < LATEST_CCA_CHECKS_NUM; ++count) {
      cca_checks[count - 1] = cca_checks[count];
    }

    latest_cca_check->check[0] =
      latest_cca_check->check[1] = 2;
    
    packet_seen = 0;
    clear_second_time = clear_third_time = 0;
    if(we_are_sending == 0) {
      latest_cca_check->check[0] =
        latest_cca_check->check[1] = -1;
      for(count = 0; count < CCA_COUNT_MAX; ++count) {
        t0 = RTIMER_NOW();
        powercycle_turn_radio_on();
#if NURTIMER
        while(RTIMER_CLOCK_LT(t0, RTIMER_NOW(), t0 + CCA_CHECK_TIME));
#else
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CCA_CHECK_TIME));
#endif
        if(NETSTACK_RADIO.channel_clear() == 0) {
          latest_cca_check->check[count] = 0;
          packet_indication_flag = 1;
          packet_seen = 1;
          break;
        } else {
          latest_cca_check->check[count] = 1;
        }
        powercycle_turn_radio_off();
        schedule_powercycle(t, CCA_SLEEP_TIME + CCA_CHECK_TIME);
        PT_YIELD(&pt);
      }

#if 1
      /* If there were a packet in the air, turn radio on */
      if(packet_seen) {
        static rtimer_clock_t start;
        static uint8_t silence_periods, periods;

        start = RTIMER_NOW();

        periods = silence_periods = 0;
        while(we_are_sending == 0 && radio_is_on &&
              RTIMER_CLOCK_LT(RTIMER_NOW(), (start + LISTEN_TIME_AFTER_PACKET_DETECTED))) {

          /* Check for two consecutive periods of non-activity. If we
             see two such periods, we turn the radio off. Also, if a
             packet has been successfully received (as indicated by
             the NETSTACK_RADIO.pending_packet() function), we stop
             listening. */
          if(NETSTACK_RADIO.channel_clear()) {
            ++silence_periods;
          } else {
            silence_periods = 0;
          }

          ++periods;
          
          if(NETSTACK_RADIO.receiving_packet()) {
            silence_periods = 0;
          }
          if(silence_periods > 5) {
            latest_cca_check->check[count] = -silence_periods;
            leds_on(LEDS_RED);
            powercycle_turn_radio_off();
#if CONTIKIMAC_CONF_COMPOWER
            compower_accumulate(&compower_idle_activity);
#endif /* CONTIKIMAC_CONF_COMPOWER */
            leds_off(LEDS_RED);
            break;
          }
          if(periods > 10 && !(NETSTACK_RADIO.receiving_packet() ||
                               NETSTACK_RADIO.pending_packet())) {
            latest_cca_check->check[count] = periods;
            leds_on(LEDS_GREEN);
            powercycle_turn_radio_off();
#if CONTIKIMAC_CONF_COMPOWER
            compower_accumulate(&compower_idle_activity);
#endif /* CONTIKIMAC_CONF_COMPOWER */

            leds_off(LEDS_GREEN);
            break;
          }
          if(NETSTACK_RADIO.pending_packet()) {
            break;
          }

          schedule_powercycle(t, CCA_CHECK_TIME + CCA_SLEEP_TIME);
          leds_on(LEDS_BLUE);
          PT_YIELD(&pt);
          leds_off(LEDS_BLUE);
        }
        if(radio_is_on && !(NETSTACK_RADIO.receiving_packet() &&
                            NETSTACK_RADIO.pending_packet())) {
          latest_cca_check->check[count] = -78;
          leds_on(LEDS_RED + LEDS_GREEN);
          powercycle_turn_radio_off();
#if CONTIKIMAC_CONF_COMPOWER
          compower_accumulate(&compower_idle_activity);
#endif /* CONTIKIMAC_CONF_COMPOWER */
          leds_off(LEDS_RED + LEDS_GREEN);
        }

      } else {
#if CONTIKIMAC_CONF_COMPOWER
        compower_accumulate(&compower_idle_activity);
#endif /* CONTIKIMAC_CONF_COMPOWER */
      }
#else /* 0 */
      if(packet_seen) {
        schedule_powercycle(t, LISTEN_TIME);
        PT_YIELD(&pt);
        if(!(NETSTACK_RADIO.receiving_packet() ||
             NETSTACK_RADIO.pending_packet())) {
          powercycle_turn_radio_off();
#if CONTIKIMAC_CONF_COMPOWER
          compower_accumulate(&compower_idle_activity);
#endif /* CONTIKIMAC_CONF_COMPOWER */
        }
      }
#endif /* 0 */

      /* If the packet indication flag is still set, it means that
         there was never a packet received by the radio. We increase
         the packet_indicated_but_not_received counter. */
      if(packet_indication_flag) {
        packet_indicated_but_not_received += packet_indication_flag;
        packet_indication_flag = 0;
      }
    }
    if(RTIMER_NOW() - cycle_start < CYCLE_TIME) {
      schedule_powercycle(t, CYCLE_TIME - (RTIMER_NOW() - cycle_start));
      PT_YIELD(&pt);
    }
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
#if CONTIKIMAC_CONF_ANNOUNCEMENTS
static int
parse_announcements(const rimeaddr_t * from)
{
  /* Parse incoming announcements */
  struct announcement_msg adata;
  int i;

  memcpy(&adata, packetbuf_dataptr(),
         MIN(packetbuf_datalen(), sizeof(adata)));

  /*  printf("%d.%d: probe from %d.%d with %d announcements\n",
     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
     from->u8[0], from->u8[1], adata->num); */
  /*  for(i = 0; i < packetbuf_datalen(); ++i) {
     printf("%02x ", ((uint8_t *)packetbuf_dataptr())[i]);
     }
     printf("\n"); */

  for(i = 0; i < adata.num; ++i) {
    /*   printf("%d.%d: announcement %d: %d\n",
       rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
       adata->data[i].id,
       adata->data[i].value); */

    announcement_heard(from, adata.data[i].id, adata.data[i].value);
  }
  return i;
}
/*---------------------------------------------------------------------------*/
static int
format_announcement(char *hdr)
{
  struct announcement_msg adata;
  struct announcement *a;

  /* Construct the announcements */
  /*  adata = (struct announcement_msg *)hdr; */

  adata.num = 0;
  for(a = announcement_list();
      a != NULL && adata.num < ANNOUNCEMENT_MAX; a = a->next) {
    adata.data[adata.num].id = a->id;
    adata.data[adata.num].value = a->value;
    adata.num++;
  }

  memcpy(hdr, &adata, sizeof(struct announcement_msg));

  if(adata.num > 0) {
    return ANNOUNCEMENT_MSG_HEADERLEN +
      sizeof(struct announcement_data) * adata.num;
  } else {
    return 0;
  }
}
#endif /* CONTIKIMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
static int
send_packet(mac_callback_t mac_callback, void *mac_callback_ptr)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  rtimer_clock_t encounter_time = 0;
  int strobes;
  int got_strobe_ack = 0;
  int len;
  int is_broadcast = 0;
  int is_reliable = 0;
  uint8_t collisions;
  int transmit_len;
  int i;

  if(packetbuf_totlen() == 0) {
    PRINTF("contikimac: send_packet data len 0\n");
    return MAC_TX_ERR_FATAL;
  }
     

#if WITH_PHASE_OPTIMIZATION
#if WITH_ACK_OPTIMIZATION
  /* Wait until the receiver is expected to be awake */
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) !=
     PACKETBUF_ATTR_PACKET_TYPE_ACK && is_streaming == 0) {

    if(phase_wait(&phase_list, packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                  CYCLE_TIME, 6 * CHECK_TIME,
                  mac_callback, mac_callback_ptr) == PHASE_DEFERRED) {
      return MAC_TX_DEFERRED;
    }
  }
#else /* WITH_ACK_OPTIMIZATION */
  if(is_streaming == 0) {
    if(phase_wait(&phase_list, packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                  CYCLE_TIME, 6 * CHECK_TIME,
                  mac_callback, mac_callback_ptr) == PHASE_DEFERRED) {
      return MAC_TX_DEFERRED;
    }
  }
#endif /* WITH_ACK_OPTIMIZATION */
#endif /* WITH_PHASE_OPTIMIZATION */

  /* Make sure that the packet is longer or equal to the shorest
     packet length. */
  if(packetbuf_totlen() < SHORTEST_PACKET_SIZE) {
    PRINTF("contikimac: shorter than shortest (%d)\n", packetbuf_totlen());
    packetbuf_set_datalen(SHORTEST_PACKET_SIZE);
  }

  /* Create the MAC header for the data packet. */
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &rimeaddr_null)) {
    is_broadcast = 1;
    PRINTDEBUG("contikimac: send broadcast\n");
  } else {
#if UIP_CONF_IPV6
    PRINTDEBUG("contikimac: send unicast to %02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[2],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[3],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[4],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[5],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[6],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[7]);
#else
    PRINTDEBUG("contikimac: send unicast to %u.%u\n",
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
               packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1]);
#endif /* UIP_CONF_IPV6 */
  }
  is_reliable = packetbuf_attr(PACKETBUF_ATTR_RELIABLE) ||
    packetbuf_attr(PACKETBUF_ATTR_ERELIABLE);
  len = NETSTACK_FRAMER.create();
  if(len == 0) {
    /* Failed to send */
    PRINTF("contikimac: send failed, too large header\n");
    return MAC_TX_ERR_FATAL;
  }

  packetbuf_compact();

  NETSTACK_RADIO.prepare(packetbuf_hdrptr(), packetbuf_totlen());
  transmit_len = packetbuf_totlen();

  
#if WITH_STREAMING
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_STREAM) {
    is_streaming = 1;
    if(rimeaddr_cmp(&is_streaming_to, &rimeaddr_null)) {
      rimeaddr_copy(&is_streaming_to,
                    packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    } else
      if(!rimeaddr_cmp
         (&is_streaming_to, packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
      rimeaddr_copy(&is_streaming_to_too,
                    packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    }
    stream_until = RTIMER_NOW() + DEFAULT_STREAM_TIME;
  }
#endif /* WITH_STREAMING */


  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;

  /* If we have a pending packet in the radio, we should not send now,
     because we will trash the received packet. Instead, we signal
     that we have a collision, which lets the packet be received. This
     packet will be retransmitted later by the MAC protocol
     instread. */
  if(NETSTACK_RADIO.receiving_packet() || NETSTACK_RADIO.pending_packet()) {
    we_are_sending = 0;
    PRINTF("contikimac: collision receiving %d, pending %d\n",
           NETSTACK_RADIO.receiving_packet(), NETSTACK_RADIO.pending_packet());
    return MAC_TX_COLLISION;
  }
  
  /* Switch off the radio to ensure that we didn't start sending while
     the radio was doing a channel check. */
  off();


  strobes = 0;

  /* Send a train of strobes until the receiver answers with an ACK. */
  collisions = 0;

  got_strobe_ack = 0;

  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) !=
     PACKETBUF_ATTR_PACKET_TYPE_ACK && is_streaming == 0) {
    /* Check if there are any transmissions by others. */
    for(i = 0; i < CCA_COUNT_MAX; ++i) {
      t0 = RTIMER_NOW();
      on();
#if NURTIMER
      while(RTIMER_CLOCK_LT(t0, RTIMER_NOW(), t0 + CCA_CHECK_TIME));
#else
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CCA_CHECK_TIME)) { }
#endif
      if(NETSTACK_RADIO.channel_clear() == 0) {
        collisions++;
        off();
        break;
      }
      off();
#if NURTIMER
      while(RTIMER_CLOCK_LT(t0, RTIMER_NOW(), t0 + CCA_SLEEP_TIME + CCA_CHECK_TIME));
#else
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CCA_SLEEP_TIME + CCA_CHECK_TIME)) { }
#endif
    }
  }

  if(collisions > 0) {
    we_are_sending = 0;
    off();
    PRINTF("contikimac: collisions before sending\n");
    return MAC_TX_COLLISION;
  }

  t0 = RTIMER_NOW();
  t = RTIMER_NOW();
#if NURTIMER
  for(strobes = 0, collisions = 0;
      got_strobe_ack == 0 && collisions == 0 &&
      RTIMER_CLOCK_LT(t0, RTIMER_NOW(), t0 + STROBE_TIME); strobes++) {
#else
  for(strobes = 0, collisions = 0;
      got_strobe_ack == 0 && collisions == 0 &&
      RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + STROBE_TIME); strobes++) {
#endif

    /* Let the watchdog know we are still alive. */
    watchdog_periodic();
    
    len = 0;

    t = RTIMER_NOW();
    
    
    /*if(is_broadcast) {
      NETSTACK_RADIO.transmit(transmit_len);
      off();b
      } else*/ {
      rtimer_clock_t wt;
      rtimer_clock_t now = RTIMER_NOW();

      leds_on(LEDS_RED);
      NETSTACK_RADIO.transmit(transmit_len);
      leds_off(LEDS_RED);
      if(!is_broadcast) {
        on();
      }
      /* Turn off the radio for a while to let the other side
         respond. We don't need to keep our radio on when we know
         that the other side needs some time to produce a reply. */
      wt = RTIMER_NOW();
#if NURTIMER
      while(RTIMER_CLOCK_LT
            (wt, RTIMER_NOW(), wt + INTER_PACKET_INTERVAL));
#else
      while(RTIMER_CLOCK_LT
            (RTIMER_NOW(), wt + INTER_PACKET_INTERVAL)) { }
#endif
      if(!is_broadcast && (NETSTACK_RADIO.receiving_packet() ||
                           NETSTACK_RADIO.pending_packet() ||
                           NETSTACK_RADIO.channel_clear() == 0)) {
        uint8_t ackbuf[ACK_LEN];
        wt = RTIMER_NOW();
#if NURTIMER
        while(RTIMER_CLOCK_LT
              (wt, RTIMER_NOW(), wt + AFTER_ACK_DETECTECT_WAIT_TIME));
#else
        while(RTIMER_CLOCK_LT
              (RTIMER_NOW(), wt + AFTER_ACK_DETECTECT_WAIT_TIME)) { }
#endif
        len = NETSTACK_RADIO.read(ackbuf, ACK_LEN);
        if(len == ACK_LEN) {
          got_strobe_ack = 1;
          encounter_time = now;
          packet_indication_flag = 0;
        } else {
          PRINTF("contikimac: collisions while sending\n");
          packet_indication_flag = 0;
          collisions++;
        }
      } else {
      }
    }
  }

#if WITH_ACK_OPTIMIZATION
  /* If we have received the strobe ACK, and we are sending a packet
     that will need an upper layer ACK (as signified by the
     PACKETBUF_ATTR_RELIABLE packet attribute), we keep the radio on. */
  if(got_strobe_ack && is_reliable) {
    on();                       /* Wait for ACK packet */
  } else {
    off();
  }
#else /* WITH_ACK_OPTIMIZATION */
  off();
#endif /* WITH_ACK_OPTIMIZATION */

#if WITH_PHASE_OPTIMIZATION
#if WITH_ACK_OPTIMIZATION
  if(got_strobe_ack && packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) !=
     PACKETBUF_ATTR_PACKET_TYPE_ACK && is_streaming == 0) {
    phase_register(&phase_list, packetbuf_addr(PACKETBUF_ADDR_RECEIVER), encounter_time);
  }
#else /* WITH_ACK_OPTIMIZATION */
  if(got_strobe_ack && is_streaming == 0) {
    phase_register(&phase_list, packetbuf_addr(PACKETBUF_ADDR_RECEIVER), encounter_time);
  }
#endif /* WITH_ACK_OPTIMIZATION */
#endif /* WITH_PHASE_OPTIMIZATION */

  PRINTF("contikimac: send (strobes=%u,len=%u,%s), done\n", strobes,
         packetbuf_totlen(), got_strobe_ack ? "ack" : "no ack");

#if CONTIKIMAC_CONF_COMPOWER
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
#endif /* CONTIKIMAC_CONF_COMPOWER */

  we_are_sending = 0;

  if(collisions > 0) {
    return MAC_TX_COLLISION;
  }
  
  if(!is_broadcast && !got_strobe_ack) {
    return MAC_TX_NOACK;
  } else {
    return MAC_TX_OK;
  }
}
/*---------------------------------------------------------------------------*/
static void
qsend_packet(mac_callback_t sent, void *ptr)
{
  int ret = send_packet(sent, ptr);
  if(ret != MAC_TX_DEFERRED) {
    mac_call_sent_callback(sent, ptr, ret, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  /* We have received the packet, so we can go back to being
     asleep. */
  off();

  packet_indication_flag = 0;

  if(packetbuf_totlen() > 0 && NETSTACK_FRAMER.parse()) {
    
    if(packetbuf_datalen() > 0 &&
       packetbuf_totlen() > 0 &&
       (rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                     &rimeaddr_node_addr) ||
        rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                     &rimeaddr_null))) {
      /* This is a regular packet that is destined to us or to the
         broadcast address. */

#if CONTIKIMAC_CONF_COMPOWER
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
#endif /* CONTIKIMAC_CONF_COMPOWER */

      PRINTDEBUG("contikimac: data(%u)\n", packetbuf_datalen());

      NETSTACK_MAC.input();
      return;
    } else {
      PRINTDEBUG("contikimac: data not for us\n");
    }
  } else {
    PRINTF("contikimac: failed to parse (%u)\n", packetbuf_totlen());
  }
}
/*---------------------------------------------------------------------------*/
#if CONTIKIMAC_CONF_ANNOUNCEMENTS
static void
send_announcement(void *ptr)
{
  struct contikimac_hdr *hdr;
  int announcement_len;

  /* Set up the probe header. */
  packetbuf_clear();
  hdr = packetbuf_dataptr();

  announcement_len = format_announcement((char *)hdr +
                                         sizeof(struct contikimac_hdr));

  if(announcement_len > 0) {
    packetbuf_set_datalen(sizeof(struct contikimac_hdr) + announcement_len);
    hdr->dispatch = DISPATCH;
    hdr->type = TYPE_ANNOUNCEMENT;

    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &rimeaddr_null);
    packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER,
                       announcement_radio_txpower);
    if(NETSTACK_FRAMER.create()) {
      we_are_sending = 1;
      NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
      we_are_sending = 0;
    }
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
    /*    printf("is_listening %d\n", is_listening); */
  }
}
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods + 1;
}
#endif /* CONTIKIMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
contikimac_set_announcement_radio_txpower(int txpower)
{
#if CONTIKIMAC_CONF_ANNOUNCEMENTS
  announcement_radio_txpower = txpower;
#endif /* CONTIKIMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  radio_is_on = 0;
  PT_INIT(&pt);
#if NURTIMER
  rtimer_setup(&rt, RTIMER_HARD,
               (void (*)(struct rtimer *, void *, int status))powercycle,
               NULL);
  rtimer_schedule(&rt, CYCLE_TIME, 1);
#else
  rtimer_set(&rt, RTIMER_NOW() + CYCLE_TIME, 1,
             (void (*)(struct rtimer *, void *))powercycle, NULL);
#endif

  contikimac_is_on = 1;

#if WITH_PHASE_OPTIMIZATION
  phase_init(&phase_list);
#endif /* WITH_PHASE_OPTIMIZATION */

#if CONTIKIMAC_CONF_ANNOUNCEMENTS
  announcement_register_listen_callback(listen_callback);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_TIME,
             cycle_announcement, NULL);
#endif /* CONTIKIMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  contikimac_is_on = 1;
#if NURTIMER
  rtimer_schedule(&rt, CYCLE_TIME, 1);
#else
  rtimer_set(&rt, RTIMER_NOW() + CYCLE_TIME, 1,
             (void (*)(struct rtimer *, void *))powercycle, NULL);
#endif

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  contikimac_is_on = 0;
  if(keep_radio_on) {
    return NETSTACK_RADIO.on();
  } else {
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
static unsigned short
duty_cycle(void)
{
  return (1ul * CLOCK_SECOND * CYCLE_TIME) / RTIMER_ARCH_SECOND;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver contikimac_driver = {
  "ContikiMAC",
  init,
  qsend_packet,
  input_packet,
  turn_on,
  turn_off,
  duty_cycle,
};
/*---------------------------------------------------------------------------*/
uint16_t
contikimac_debug_print(void)
{
  int i;
  for(i = 0; i < LATEST_CCA_CHECKS_NUM; ++i) {
    printf("%d: %d, %d\n", i, cca_checks[i].check[0], cca_checks[i].check[1]);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
