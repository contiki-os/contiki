/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 * \file
 *         IEEE 802.15.4 TSCH MAC implementation.
 *         Does not use any RDC layer. Should be used with nordc.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         Beshr Al Nahas <beshr@sics.se>
 *
 */

#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/mac/framer-802154.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-slot-operation.h"
#include "net/mac/tsch/tsch-queue.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-log.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-security.h"
#include "lib/random.h"

#if FRAME802154_VERSION < FRAME802154_IEEE802154E_2012
#error TSCH: FRAME802154_VERSION must be at least FRAME802154_IEEE802154E_2012
#endif

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/ip/uip-debug.h"

/* Use to collect link statistics even on Keep-Alive, even though they were
 * not sent from an upper layer and don't have a valid packet_sent callback */
#ifndef TSCH_LINK_NEIGHBOR_CALLBACK
void uip_ds6_link_neighbor_callback(int status, int numtx);
#define TSCH_LINK_NEIGHBOR_CALLBACK(dest, status, num) uip_ds6_link_neighbor_callback(status, num)
#endif /* TSCH_LINK_NEIGHBOR_CALLBACK */

/* 802.15.4 duplicate frame detection */
struct seqno {
  linkaddr_t sender;
  uint8_t seqno;
};

/* Size of the sequence number history */
#ifdef NETSTACK_CONF_MAC_SEQNO_HISTORY
#define MAX_SEQNOS NETSTACK_CONF_MAC_SEQNO_HISTORY
#else /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
#define MAX_SEQNOS 8
#endif /* NETSTACK_CONF_MAC_SEQNO_HISTORY */

/* Seqno history */
static struct seqno received_seqnos[MAX_SEQNOS];

/* Let TSCH select a time source with no help of an upper layer.
 * We do so using statistics from incoming EBs */
#if TSCH_AUTOSELECT_TIME_SOURCE
int best_neighbor_eb_count;
struct eb_stat {
  int rx_count;
  int jp;
};
NBR_TABLE(struct eb_stat, eb_stats);
#endif /* TSCH_AUTOSELECT_TIME_SOURCE */

/* TSCH channel hopping sequence */
uint8_t tsch_hopping_sequence[TSCH_HOPPING_SEQUENCE_MAX_LEN];
struct asn_divisor_t tsch_hopping_sequence_length;

/* Default TSCH timeslot timing (in micro-second) */
static const uint16_t tsch_default_timing_us[tsch_ts_elements_count] = {
  TSCH_DEFAULT_TS_CCA_OFFSET,
  TSCH_DEFAULT_TS_CCA,
  TSCH_DEFAULT_TS_TX_OFFSET,
  TSCH_DEFAULT_TS_RX_OFFSET,
  TSCH_DEFAULT_TS_RX_ACK_DELAY,
  TSCH_DEFAULT_TS_TX_ACK_DELAY,
  TSCH_DEFAULT_TS_RX_WAIT,
  TSCH_DEFAULT_TS_ACK_WAIT,
  TSCH_DEFAULT_TS_RX_TX,
  TSCH_DEFAULT_TS_MAX_ACK,
  TSCH_DEFAULT_TS_MAX_TX,
  TSCH_DEFAULT_TS_TIMESLOT_LENGTH,
};
/* TSCH timeslot timing (in rtimer ticks) */
rtimer_clock_t tsch_timing[tsch_ts_elements_count];

#if LINKADDR_SIZE == 8
/* 802.15.4 broadcast MAC address  */
const linkaddr_t tsch_broadcast_address = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
/* Address used for the EB virtual neighbor queue */
const linkaddr_t tsch_eb_address = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
#else /* LINKADDR_SIZE == 8 */
const linkaddr_t tsch_broadcast_address = { { 0xff, 0xff } };
const linkaddr_t tsch_eb_address = { { 0, 0 } };
#endif /* LINKADDR_SIZE == 8 */

/* Is TSCH started? */
int tsch_is_started = 0;
/* Has TSCH initialization failed? */
int tsch_is_initialized = 0;
/* Are we coordinator of the TSCH network? */
int tsch_is_coordinator = 0;
/* Are we associated to a TSCH network? */
int tsch_is_associated = 0;
/* Is the PAN running link-layer security? */
int tsch_is_pan_secured = TSCH_SECURITY_ENABLED;
/* The current Absolute Slot Number (ASN) */
struct asn_t current_asn;
/* Device rank or join priority:
 * For PAN coordinator: 0 -- lower is better */
uint8_t tsch_join_priority;
/* The current TSCH sequence number, used for both data and EBs */
static uint8_t tsch_packet_seqno = 0;
/* Current period for EB output */
static clock_time_t tsch_current_eb_period;

/* timer for sending keepalive messages */
static struct ctimer keepalive_timer;

/* TSCH processes and protothreads */
PT_THREAD(tsch_scan(struct pt *pt));
PROCESS(tsch_process, "TSCH: main process");
PROCESS(tsch_send_eb_process, "TSCH: send EB process");
PROCESS(tsch_pending_events_process, "TSCH: pending events process");

/* Other function prototypes */
static void packet_input(void);

/* Getters and setters */

/*---------------------------------------------------------------------------*/
void
tsch_set_coordinator(int enable)
{
  tsch_is_coordinator = enable;
  tsch_set_eb_period(TSCH_EB_PERIOD);
}
/*---------------------------------------------------------------------------*/
void
tsch_set_pan_secured(int enable)
{
  tsch_is_pan_secured = TSCH_SECURITY_ENABLED && enable;
}
/*---------------------------------------------------------------------------*/
void
tsch_set_join_priority(uint8_t jp)
{
  tsch_join_priority = jp;
}
/*---------------------------------------------------------------------------*/
void
tsch_set_eb_period(uint32_t period)
{
  tsch_current_eb_period = period;
}
/*---------------------------------------------------------------------------*/
static void
tsch_reset(void)
{
  int i;
  frame802154_set_pan_id(0xffff);
  /* First make sure pending packet callbacks are sent etc */
  process_post_synch(&tsch_pending_events_process, PROCESS_EVENT_POLL, NULL);
  /* Empty all neighbor queues */
  /* tsch_queue_flush_all(); */
  /* Remove unused neighbors */
  tsch_queue_free_unused_neighbors();
  tsch_queue_update_time_source(NULL);
  /* Initialize global variables */
  tsch_join_priority = 0xff;
  ASN_INIT(current_asn, 0, 0);
  current_link = NULL;
  /* Reset timeslot timing to defaults */
  for(i = 0; i < tsch_ts_elements_count; i++) {
    tsch_timing[i] = US_TO_RTIMERTICKS(tsch_default_timing_us[i]);
  }
#ifdef TSCH_CALLBACK_LEAVING_NETWORK
  TSCH_CALLBACK_LEAVING_NETWORK();
#endif
#if TSCH_AUTOSELECT_TIME_SOURCE
  best_neighbor_eb_count = 0;
  nbr_table_register(eb_stats, NULL);
  tsch_set_eb_period(TSCH_EB_PERIOD);
#endif
}

/* TSCH keep-alive functions */

/*---------------------------------------------------------------------------*/
/* Tx callback for keepalive messages */
static void
keepalive_packet_sent(void *ptr, int status, int transmissions)
{
#ifdef TSCH_LINK_NEIGHBOR_CALLBACK
  TSCH_LINK_NEIGHBOR_CALLBACK(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), status, transmissions);
#endif
  PRINTF("TSCH: KA sent to %u, st %d-%d\n",
         TSCH_LOG_ID_FROM_LINKADDR(packetbuf_addr(PACKETBUF_ADDR_RECEIVER)), status, transmissions);
  tsch_schedule_keepalive();
}
/*---------------------------------------------------------------------------*/
/* Prepare and send a keepalive message */
static void
keepalive_send()
{
  if(tsch_is_associated) {
    struct tsch_neighbor *n = tsch_queue_get_time_source();
    /* Simply send an empty packet */
    packetbuf_clear();
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &n->addr);
    NETSTACK_LLSEC.send(keepalive_packet_sent, NULL);
    PRINTF("TSCH: sending KA to %u\n",
           TSCH_LOG_ID_FROM_LINKADDR(&n->addr));
  }
}
/*---------------------------------------------------------------------------*/
/* Set ctimer to send a keepalive message after expiration of TSCH_KEEPALIVE_TIMEOUT */
void
tsch_schedule_keepalive()
{
  /* Pick a delay in the range [TSCH_KEEPALIVE_TIMEOUT*0.9, TSCH_KEEPALIVE_TIMEOUT[ */
  if(!tsch_is_coordinator && tsch_is_associated) {
    unsigned long delay = (TSCH_KEEPALIVE_TIMEOUT - TSCH_KEEPALIVE_TIMEOUT / 10)
      + random_rand() % (TSCH_KEEPALIVE_TIMEOUT / 10);
    ctimer_set(&keepalive_timer, delay, keepalive_send, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static void
eb_input(struct input_packet *current_input)
{
  /* PRINTF("TSCH: EB received\n"); */
  frame802154_t frame;
  /* Verify incoming EB (does its ASN match our Rx time?),
   * and update our join priority. */
  struct ieee802154_ies eb_ies;

  if(tsch_packet_parse_eb(current_input->payload, current_input->len,
                          &frame, &eb_ies, NULL, 1)) {
    /* PAN ID check and authentication done at rx time */

#if TSCH_AUTOSELECT_TIME_SOURCE
    if(!tsch_is_coordinator) {
      /* Maintain EB received counter for every neighbor */
      struct eb_stat *stat = (struct eb_stat *)nbr_table_get_from_lladdr(eb_stats, &frame.src_addr);
      if(stat == NULL) {
        stat = (struct eb_stat *)nbr_table_add_lladdr(eb_stats, &frame.src_addr);
      }
      if(stat != NULL) {
        stat->rx_count++;
        stat->jp = eb_ies.join_priority;
        best_neighbor_eb_count = MAX(best_neighbor_eb_count, stat->rx_count);
      }
      /* Select best time source */
      struct eb_stat *best_stat = NULL;
      stat = nbr_table_head(eb_stats);
      while(stat != NULL) {
        /* Is neighbor eligible as a time source? */
        if(stat->rx_count > best_neighbor_eb_count / 2) {
          if(best_stat == NULL ||
             stat->jp < best_stat->jp) {
            best_stat = stat;
          }
        }
        stat = nbr_table_next(eb_stats, stat);
      }
      /* Update time source */
      if(best_stat != NULL) {
        tsch_queue_update_time_source(nbr_table_get_lladdr(eb_stats, best_stat));
        tsch_join_priority = best_stat->jp + 1;
      }
    }
#endif

    struct tsch_neighbor *n = tsch_queue_get_time_source();
    /* Did the EB come from our time source? */
    if(n != NULL && linkaddr_cmp((linkaddr_t *)&frame.src_addr, &n->addr)) {
      /* Check for ASN drift */
      int32_t asn_diff = ASN_DIFF(current_input->rx_asn, eb_ies.ie_asn);
      if(asn_diff != 0) {
        /* We disagree with our time source's ASN -- leave the network */
        PRINTF("TSCH:! ASN drifted by %ld, leaving the network\n", asn_diff);
        tsch_disassociate();
      }

      if(eb_ies.ie_join_priority >= TSCH_MAX_JOIN_PRIORITY) {
        /* Join priority unacceptable. Leave network. */
        PRINTF("TSCH:! EB JP too high %u, leaving the network\n",
               eb_ies.ie_join_priority);
        tsch_disassociate();
      } else {
#if TSCH_AUTOSELECT_TIME_SOURCE
        /* Update join priority */
        if(tsch_join_priority != eb_ies.ie_join_priority + 1) {
          PRINTF("TSCH: update JP from EB %u -> %u\n",
                 tsch_join_priority, eb_ies.ie_join_priority + 1);
          tsch_join_priority = eb_ies.ie_join_priority + 1;
        }
#endif /* TSCH_AUTOSELECT_TIME_SOURCE */
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Process pending input packet(s) */
static void
tsch_rx_process_pending()
{
  int16_t input_index;
  /* Loop on accessing (without removing) a pending input packet */
  while((input_index = ringbufindex_peek_get(&input_ringbuf)) != -1) {
    struct input_packet *current_input = &input_array[input_index];
    frame802154_t frame;
    uint8_t ret = frame802154_parse(current_input->payload, current_input->len, &frame);
    int is_data = ret && frame.fcf.frame_type == FRAME802154_DATAFRAME;
    int is_eb = ret
      && frame.fcf.frame_version == FRAME802154_IEEE802154E_2012
      && frame.fcf.frame_type == FRAME802154_BEACONFRAME;

    if(is_data) {
      /* Skip EBs and other control messages */
      /* Copy to packetbuf for processing */
      packetbuf_copyfrom(current_input->payload, current_input->len);
      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, current_input->rssi);
    }

    /* Remove input from ringbuf */
    ringbufindex_get(&input_ringbuf);

    if(is_data) {
      /* Pass to upper layers */
      packet_input();
    } else if(is_eb) {
      eb_input(current_input);
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Pass sent packets to upper layer */
static void
tsch_tx_process_pending()
{
  int16_t dequeued_index;
  /* Loop on accessing (without removing) a pending input packet */
  while((dequeued_index = ringbufindex_peek_get(&dequeued_ringbuf)) != -1) {
    struct tsch_packet *p = dequeued_array[dequeued_index];
    /* Put packet into packetbuf for packet_sent callback */
    queuebuf_to_packetbuf(p->qb);
    /* Call packet_sent callback */
    mac_call_sent_callback(p->sent, p->ptr, p->ret, p->transmissions);
    /* Free packet queuebuf */
    tsch_queue_free_packet(p);
    /* Free all unused neighbors */
    tsch_queue_free_unused_neighbors();
    /* Remove dequeued packet from ringbuf */
    ringbufindex_get(&dequeued_ringbuf);
  }
}
/*---------------------------------------------------------------------------*/
/* Setup TSCH as a coordinator */
static void
tsch_start_coordinator(void)
{
  frame802154_set_pan_id(IEEE802154_PANID);
  /* Initialize hopping sequence as default */
  memcpy(tsch_hopping_sequence, TSCH_DEFAULT_HOPPING_SEQUENCE, sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
  ASN_DIVISOR_INIT(tsch_hopping_sequence_length, sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
#if TSCH_SCHEDULE_WITH_6TISCH_MINIMAL
  tsch_schedule_create_minimal();
#endif

  tsch_is_associated = 1;
  tsch_join_priority = 0;

  PRINTF("TSCH: starting as coordinator, PAN ID %x, asn-%x.%lx\n",
      frame802154_get_pan_id(), current_asn.ms1b, current_asn.ls4b);

  /* Start slot operation */
  tsch_slot_operation_sync(RTIMER_NOW(), &current_asn);
}
/*---------------------------------------------------------------------------*/
/* Leave the TSCH network */
void
tsch_disassociate(void)
{
  if(tsch_is_associated == 1) {
    tsch_is_associated = 0;
    process_post(&tsch_process, PROCESS_EVENT_POLL, NULL);
    PRINTF("TSCH: leaving the network\n");
  }
}
/*---------------------------------------------------------------------------*/
/* Attempt to associate to a network form an incoming EB */
static int
tsch_associate(const struct input_packet *input_eb, rtimer_clock_t timestamp)
{
  frame802154_t frame;
  struct ieee802154_ies ies;
  uint8_t hdrlen;
  int i;

  if(input_eb == NULL || tsch_packet_parse_eb(input_eb->payload, input_eb->len,
                                              &frame, &ies, &hdrlen, 0) == 0) {
    PRINTF("TSCH:! failed to parse EB (len %u)\n", input_eb->len);
    return 0;
  }

  current_asn = ies.ie_asn;
  tsch_join_priority = ies.ie_join_priority + 1;

#if TSCH_JOIN_SECURED_ONLY
  if(frame.fcf.security_enabled == 0) {
    PRINTF("TSCH:! parse_eb: EB is not secured\n");
    return 0;
  }
#endif /* TSCH_JOIN_SECURED_ONLY */
  
#if TSCH_SECURITY_ENABLED
  if(!tsch_security_parse_frame(input_eb->payload, hdrlen,
      input_eb->len - hdrlen - tsch_security_mic_len(&frame),
      &frame, (linkaddr_t*)&frame.src_addr, &current_asn)) {
    PRINTF("TSCH:! parse_eb: failed to authenticate\n");
    return 0;
  }
#endif /* TSCH_SECURITY_ENABLED */

#if !TSCH_SECURITY_ENABLED
  if(frame.fcf.security_enabled == 1) {
    PRINTF("TSCH:! parse_eb: we do not support security, but EB is secured\n");
    return 0;
  }
#endif /* !TSCH_SECURITY_ENABLED */

#if TSCH_JOIN_MY_PANID_ONLY
  /* Check if the EB comes from the PAN ID we expect */
  if(frame.src_pid != IEEE802154_PANID) {
    PRINTF("TSCH:! parse_eb: PAN ID %x != %x\n", frame.src_pid, IEEE802154_PANID);
    return 0;
  }
#endif /* TSCH_JOIN_MY_PANID_ONLY */

  /* There was no join priority (or 0xff) in the EB, do not join */
  if(ies.ie_join_priority == 0xff) {
    PRINTF("TSCH:! parse_eb: no join priority\n");
    return 0;
  }

  /* TSCH timeslot timing */
  for(i = 0; i < tsch_ts_elements_count; i++) {
    if(ies.ie_tsch_timeslot_id == 0) {
      tsch_timing[i] = US_TO_RTIMERTICKS(tsch_default_timing_us[i]);
    } else {
      tsch_timing[i] = US_TO_RTIMERTICKS(ies.ie_tsch_timeslot[i]);
    }
  }

  /* TSCH hopping sequence */
  if(ies.ie_channel_hopping_sequence_id == 0) {
    memcpy(tsch_hopping_sequence, TSCH_DEFAULT_HOPPING_SEQUENCE, sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
    ASN_DIVISOR_INIT(tsch_hopping_sequence_length, sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
  } else {
    if(ies.ie_hopping_sequence_len <= sizeof(tsch_hopping_sequence)) {
      memcpy(tsch_hopping_sequence, ies.ie_hopping_sequence_list, ies.ie_hopping_sequence_len);
      ASN_DIVISOR_INIT(tsch_hopping_sequence_length, ies.ie_hopping_sequence_len);
    } else {
      PRINTF("TSCH:! parse_eb: hopping sequence too long (%u)\n", ies.ie_hopping_sequence_len);
      return 0;
    }
  }

#if TSCH_CHECK_TIME_AT_ASSOCIATION > 0
  /* Divide by 4k and multiply again to avoid integer overflow */
  uint32_t expected_asn = 4096 * TSCH_CLOCK_TO_SLOTS(clock_time() / 4096, tsch_timing_timeslot_length); /* Expected ASN based on our current time*/
  int32_t asn_threshold = TSCH_CHECK_TIME_AT_ASSOCIATION * 60ul * TSCH_CLOCK_TO_SLOTS(CLOCK_SECOND, tsch_timing_timeslot_length);
  int32_t asn_diff = (int32_t)current_asn.ls4b - expected_asn;
  if(asn_diff > asn_threshold) {
    PRINTF("TSCH:! EB ASN rejected %lx %lx %ld\n",
           current_asn.ls4b, expected_asn, asn_diff);
    return 0;
  }
#endif

#if TSCH_INIT_SCHEDULE_FROM_EB
  /* Create schedule */
  if(ies.ie_tsch_slotframe_and_link.num_slotframes == 0) {
#if TSCH_SCHEDULE_WITH_6TISCH_MINIMAL
    PRINTF("TSCH: parse_eb: no schedule, setting up minimal schedule\n");
    tsch_schedule_create_minimal();
#else
    PRINTF("TSCH: parse_eb: no schedule\n");
#endif
  } else {
    /* First, empty current schedule */
    tsch_schedule_remove_all_slotframes();
    /* We support only 0 or 1 slotframe in this IE */
    int num_links = ies.ie_tsch_slotframe_and_link.num_links;
    if(num_links <= FRAME802154E_IE_MAX_LINKS) {
      int i;
      struct tsch_slotframe *sf = tsch_schedule_add_slotframe(
          ies.ie_tsch_slotframe_and_link.slotframe_handle,
          ies.ie_tsch_slotframe_and_link.slotframe_size);
      for(i = 0; i < num_links; i++) {
        tsch_schedule_add_link(sf,
            ies.ie_tsch_slotframe_and_link.links[i].link_options,
            LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
            ies.ie_tsch_slotframe_and_link.links[i].timeslot, ies.ie_tsch_slotframe_and_link.links[i].channel_offset);
      }
    } else {
      PRINTF("TSCH:! parse_eb: too many links in schedule (%u)\n", num_links);
      return 0;
    }
  }
#endif /* TSCH_INIT_SCHEDULE_FROM_EB */

  if(tsch_join_priority < TSCH_MAX_JOIN_PRIORITY) {
    struct tsch_neighbor *n;

    /* Add coordinator to list of neighbors, lock the entry */
    n = tsch_queue_add_nbr((linkaddr_t *)&frame.src_addr);

    if(n != NULL) {
      tsch_queue_update_time_source((linkaddr_t *)&frame.src_addr);

#ifdef TSCH_CALLBACK_JOINING_NETWORK
      TSCH_CALLBACK_JOINING_NETWORK();
#endif

      /* Set PANID */
      frame802154_set_pan_id(frame.src_pid);

      /* Synchronize on EB */
      tsch_slot_operation_sync(timestamp - tsch_timing[tsch_ts_tx_offset], &current_asn);

      /* Update global flags */
      tsch_is_associated = 1;
      tsch_is_pan_secured = frame.fcf.security_enabled;

      /* Association done, schedule keepalive messages */
      tsch_schedule_keepalive();

      PRINTF("TSCH: association done, sec %u, PAN ID %x, asn-%x.%lx, jp %u, timeslot id %u, hopping id %u, slotframe len %u with %u links, from ",
             tsch_is_pan_secured,
             frame.src_pid,
             current_asn.ms1b, current_asn.ls4b, tsch_join_priority,
             ies.ie_tsch_timeslot_id,
             ies.ie_channel_hopping_sequence_id,
             ies.ie_tsch_slotframe_and_link.slotframe_size,
             ies.ie_tsch_slotframe_and_link.num_links);
      PRINTLLADDR((const uip_lladdr_t *)&frame.src_addr);
      PRINTF("\n");

      return 1;
    }
  }
  PRINTF("TSCH:! did not associate.\n");
  return 0;
}

/* Processes and protothreads used by TSCH */

/*---------------------------------------------------------------------------*/
/* Scanning protothread, called by tsch_process:
 * Listen to different channels, and when receiving an EB,
 * attempt to associate.
 */
PT_THREAD(tsch_scan(struct pt *pt))
{
  PT_BEGIN(pt);

  static struct input_packet input_eb;
  static struct etimer scan_timer;

  ASN_INIT(current_asn, 0, 0);

  etimer_set(&scan_timer, CLOCK_SECOND / TSCH_ASSOCIATION_POLL_FREQUENCY);

  while(!tsch_is_associated && !tsch_is_coordinator) {
    /* Hop to any channel offset */
    static int current_channel = 0;
    /* Time when we started scanning on current_channel */
    static clock_time_t current_channel_since = 0;

    /* We are not coordinator, try to associate */
    rtimer_clock_t t0;
    int is_packet_pending = 0;
    clock_time_t now_seconds = clock_seconds();

    /* Switch to a (new) channel for scanning */
    if(current_channel == 0 || now_seconds != current_channel_since) {
      /* Pick a channel at random in TSCH_JOIN_HOPPING_SEQUENCE */
      uint8_t scan_channel = TSCH_JOIN_HOPPING_SEQUENCE[
          random_rand() % sizeof(TSCH_JOIN_HOPPING_SEQUENCE)];
      if(current_channel != scan_channel) {
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, scan_channel);
        current_channel = scan_channel;
        PRINTF("TSCH: scanning on channel %u\n", scan_channel);
      }
      current_channel_since = now_seconds;
    }

    /* Turn radio on and wait for EB */
    NETSTACK_RADIO.on();

    is_packet_pending = NETSTACK_RADIO.pending_packet();
    if(!is_packet_pending && NETSTACK_RADIO.receiving_packet()) {
      /* If we are currently receiving a packet, wait until end of reception */
      t0 = RTIMER_NOW();
      BUSYWAIT_UNTIL_ABS((is_packet_pending = NETSTACK_RADIO.pending_packet()), t0, RTIMER_SECOND / 100);
    }

    if(is_packet_pending) {
      /* Save packet timestamp */
      NETSTACK_RADIO.get_object(RADIO_PARAM_LAST_PACKET_TIMESTAMP, &t0, sizeof(rtimer_clock_t));

      /* Read packet */
      input_eb.len = NETSTACK_RADIO.read(input_eb.payload, TSCH_PACKET_MAX_LEN);

      /* Parse EB and attempt to associate */
      PRINTF("TSCH: association: received packet (%u bytes) on channel %u\n", input_eb.len, current_channel);

      tsch_associate(&input_eb, t0);
    }

    if(tsch_is_associated) {
      /* End of association, turn the radio off */
      NETSTACK_RADIO.off();
    } else if(!tsch_is_coordinator) {
      /* Go back to scanning */
      etimer_reset(&scan_timer);
      PT_WAIT_UNTIL(pt, etimer_expired(&scan_timer));
    }
  }

  PT_END(pt);
}

/*---------------------------------------------------------------------------*/
/* The main TSCH process */
PROCESS_THREAD(tsch_process, ev, data)
{
  static struct pt scan_pt;

  PROCESS_BEGIN();

  while(1) {

    while(!tsch_is_associated) {
      if(tsch_is_coordinator) {
        /* We are coordinator, start operating now */
        tsch_start_coordinator();
      } else {
        /* Start scanning, will attempt to join when receiving an EB */
        PROCESS_PT_SPAWN(&scan_pt, tsch_scan(&scan_pt));
      }
    }

    /* We are part of a TSCH network, start slot operation */
    tsch_slot_operation_start();

    /* Yield our main process. Slot operation will re-schedule itself
     * as long as we are associated */
    PROCESS_YIELD_UNTIL(!tsch_is_associated);

    /* Will need to re-synchronize */
    tsch_reset();
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* A periodic process to send TSCH Enhanced Beacons (EB) */
PROCESS_THREAD(tsch_send_eb_process, ev, data)
{
  static struct etimer eb_timer;

  PROCESS_BEGIN();

  /* Wait until association */
  etimer_set(&eb_timer, CLOCK_SECOND / 10);
  while(!tsch_is_associated) {
    PROCESS_WAIT_UNTIL(etimer_expired(&eb_timer));
    etimer_reset(&eb_timer);
  }

  /* Set an initial delay except for coordinator, which should send an EB asap */
  if(!tsch_is_coordinator) {
    etimer_set(&eb_timer, random_rand() % TSCH_EB_PERIOD);
    PROCESS_WAIT_UNTIL(etimer_expired(&eb_timer));
  }

  while(1) {
    unsigned long delay;

    if(tsch_is_associated && tsch_current_eb_period > 0) {
      /* Enqueue EB only if there isn't already one in queue */
      if(tsch_queue_packet_count(&tsch_eb_address) == 0) {
        int eb_len;
        uint8_t hdr_len = 0;
        uint8_t tsch_sync_ie_offset;
        /* Prepare the EB packet and schedule it to be sent */
        packetbuf_clear();
        /* We don't use seqno 0 */
        if(++tsch_packet_seqno == 0) {
          tsch_packet_seqno++;
        }
        packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_BEACONFRAME);
        packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, tsch_packet_seqno);
#if TSCH_SECURITY_ENABLED
        if(tsch_is_pan_secured) {
          /* Set security level, key id and index */
          packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL, TSCH_SECURITY_KEY_SEC_LEVEL_EB);
          packetbuf_set_attr(PACKETBUF_ATTR_KEY_ID_MODE, FRAME802154_1_BYTE_KEY_ID_MODE); /* Use 1-byte key index */
          packetbuf_set_attr(PACKETBUF_ATTR_KEY_INDEX, TSCH_SECURITY_KEY_INDEX_EB);
        }
#endif /* TSCH_SECURITY_ENABLED */
        eb_len = tsch_packet_create_eb(packetbuf_dataptr(), PACKETBUF_SIZE,
            tsch_packet_seqno, &hdr_len, &tsch_sync_ie_offset);
        if(eb_len != 0) {
          struct tsch_packet *p;
          packetbuf_set_datalen(eb_len);
          /* Enqueue EB packet */
          if(!(p = tsch_queue_add_packet(&tsch_eb_address, NULL, NULL))) {
            PRINTF("TSCH:! could not enqueue EB packet\n");
          } else {
            PRINTF("TSCH: enqueue EB packet %u %u\n", eb_len, hdr_len);
            p->tsch_sync_ie_offset = tsch_sync_ie_offset;
            p->header_len = hdr_len;
          }
        }
      }
    }
    if(tsch_current_eb_period > 0) {
      /* Next EB transmission with a random delay
       * within [tsch_current_eb_period*0.75, tsch_current_eb_period[ */
      delay = (tsch_current_eb_period - tsch_current_eb_period / 4)
        + random_rand() % (tsch_current_eb_period / 4);
    } else {
      delay = TSCH_EB_PERIOD;
    }
    etimer_set(&eb_timer, delay);
    PROCESS_WAIT_UNTIL(etimer_expired(&eb_timer));
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* A process that is polled from interrupt and calls tx/rx input
 * callbacks, outputs pending logs. */
PROCESS_THREAD(tsch_pending_events_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    tsch_rx_process_pending();
    tsch_tx_process_pending();
    tsch_log_process_pending();
  }
  PROCESS_END();
}

/* Functions from the Contiki MAC layer driver interface */

/*---------------------------------------------------------------------------*/
static void
tsch_init(void)
{
  radio_value_t radio_rx_mode;
  radio_value_t radio_tx_mode;
  rtimer_clock_t t;

  /* Radio Rx mode */
  if(NETSTACK_RADIO.get_value(RADIO_PARAM_RX_MODE, &radio_rx_mode) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support getting RADIO_PARAM_RX_MODE. Abort init.\n");
    return;
  }
  /* Disable radio in frame filtering */
  radio_rx_mode &= ~RADIO_RX_MODE_ADDRESS_FILTER;
  /* Unset autoack */
  radio_rx_mode &= ~RADIO_RX_MODE_AUTOACK;
  /* Set radio in poll mode */
  radio_rx_mode |= RADIO_RX_MODE_POLL_MODE;
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, radio_rx_mode) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support setting required RADIO_PARAM_RX_MODE. Abort init.\n");
    return;
  }

  /* Radio Tx mode */
  if(NETSTACK_RADIO.get_value(RADIO_PARAM_TX_MODE, &radio_tx_mode) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support getting RADIO_PARAM_TX_MODE. Abort init.\n");
    return;
  }
  /* Unset CCA */
  radio_tx_mode &= ~RADIO_TX_MODE_SEND_ON_CCA;
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, radio_tx_mode) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support setting required RADIO_PARAM_TX_MODE. Abort init.\n");
    return;
  }
  /* Test setting channel */
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, TSCH_DEFAULT_HOPPING_SEQUENCE[0]) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support setting channel. Abort init.\n");
    return;
  }
  /* Test getting timestamp */
  if(NETSTACK_RADIO.get_object(RADIO_PARAM_LAST_PACKET_TIMESTAMP, &t, sizeof(rtimer_clock_t)) != RADIO_RESULT_OK) {
    printf("TSCH:! radio does not support getting last packet timestamp. Abort init.\n");
    return;
  }
  /* Check max hopping sequence length vs default sequence length */
  if(TSCH_HOPPING_SEQUENCE_MAX_LEN < sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE)) {
    printf("TSCH:! TSCH_HOPPING_SEQUENCE_MAX_LEN < sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE). Abort init.\n");
  }

  /* Init TSCH sub-modules */
  tsch_reset();
  tsch_queue_init();
  tsch_schedule_init();
  tsch_log_init();
  ringbufindex_init(&input_ringbuf, TSCH_MAX_INCOMING_PACKETS);
  ringbufindex_init(&dequeued_ringbuf, TSCH_DEQUEUED_ARRAY_SIZE);

  tsch_is_initialized = 1;

#if TSCH_AUTOSTART
  /* Start TSCH operation.
   * If TSCH_AUTOSTART is not set, one needs to call NETSTACK_MAC.on() to start TSCH. */
  NETSTACK_MAC.on();
#endif /* TSCH_AUTOSTART */
}
/*---------------------------------------------------------------------------*/
/* Function send for TSCH-MAC, puts the packet in packetbuf in the MAC queue */
static void
send_packet(mac_callback_t sent, void *ptr)
{
  int ret = MAC_TX_DEFERRED;
  int packet_count_before;
  int hdr_len = 0;
  const linkaddr_t *addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(!tsch_is_associated) {
    if(!tsch_is_initialized) {
      PRINTF("TSCH:! not initialized (see earlier logs), drop outgoing packet\n");
    } else {
      PRINTF("TSCH:! not associated, drop outgoing packet\n");
    }
    ret = MAC_TX_ERR;
    mac_call_sent_callback(sent, ptr, ret, 1);
    return;
  }

  /* PACKETBUF_ATTR_MAC_SEQNO cannot be zero, due to a pecuilarity
         in framer-802154.c. */
  if(++tsch_packet_seqno == 0) {
    tsch_packet_seqno++;
  }

  /* Ask for ACK if we are sending anything other than broadcast */
  if(!linkaddr_cmp(addr, &linkaddr_null)) {
    packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);
  } else {
    /* Broadcast packets shall be added to broadcast queue
     * The broadcast address in Contiki is linkaddr_null which is equal
     * to tsch_eb_address */
    addr = &tsch_broadcast_address;
  }

  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, tsch_packet_seqno);

#if TSCH_SECURITY_ENABLED
  if(tsch_is_pan_secured) {
    /* Set security level, key id and index */
    packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL, TSCH_SECURITY_KEY_SEC_LEVEL_OTHER);
    packetbuf_set_attr(PACKETBUF_ATTR_KEY_ID_MODE, FRAME802154_1_BYTE_KEY_ID_MODE); /* Use 1-byte key index */
    packetbuf_set_attr(PACKETBUF_ATTR_KEY_INDEX, TSCH_SECURITY_KEY_INDEX_OTHER);
  }
#endif /* TSCH_SECURITY_ENABLED */

  packet_count_before = tsch_queue_packet_count(addr);

  if((hdr_len = NETSTACK_FRAMER.create()) < 0) {
    PRINTF("TSCH:! can't send packet due to framer error\n");
    ret = MAC_TX_ERR;
  } else {
    struct tsch_packet *p;
    /* Enqueue packet */
    p = tsch_queue_add_packet(addr, sent, ptr);
    if(p == NULL) {
      PRINTF("TSCH:! can't send packet !tsch_queue_add_packet\n");
      ret = MAC_TX_ERR;
    } else {
      p->header_len = hdr_len;
      PRINTF("TSCH: send packet to %u with seqno %u, queue %u %u, len %u %u\n",
             TSCH_LOG_ID_FROM_LINKADDR(addr), tsch_packet_seqno,
             packet_count_before,
             tsch_queue_packet_count(addr),
             p->header_len,
             queuebuf_datalen(p->qb));
    }
  }
  if(ret != MAC_TX_DEFERRED) {
    mac_call_sent_callback(sent, ptr, ret, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  int frame_parsed = 1;

  frame_parsed = NETSTACK_FRAMER.parse();

  if(frame_parsed < 0) {
    PRINTF("TSCH:! failed to parse %u\n", packetbuf_datalen());
  } else {
    int duplicate = 0;

    /* Seqno of 0xffff means no seqno */
    if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) != 0xffff) {
      /* Check for duplicate packet by comparing the sequence number
         of the incoming packet with the last few ones we saw. */
      int i;
      for(i = 0; i < MAX_SEQNOS; ++i) {
        if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) == received_seqnos[i].seqno &&
           linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
                        &received_seqnos[i].sender)) {
          /* Drop the packet. */
          PRINTF("TSCH:! drop dup ll from %u seqno %u\n",
                 TSCH_LOG_ID_FROM_LINKADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER)),
                 packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
          duplicate = 1;
        }
      }
      if(!duplicate) {
        for(i = MAX_SEQNOS - 1; i > 0; --i) {
          memcpy(&received_seqnos[i], &received_seqnos[i - 1],
                 sizeof(struct seqno));
        }
        received_seqnos[0].seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
        linkaddr_copy(&received_seqnos[0].sender,
                      packetbuf_addr(PACKETBUF_ADDR_SENDER));
      }
    }

    if(!duplicate) {
      PRINTF("TSCH: received from %u with seqno %u\n",
             TSCH_LOG_ID_FROM_LINKADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER)),
             packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
      NETSTACK_LLSEC.input();
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  if(tsch_is_initialized == 1 && tsch_is_started == 0) {
    tsch_is_started = 1;
    /* Process tx/rx callback and log messages whenever polled */
    process_start(&tsch_pending_events_process, NULL);
    /* periodically send TSCH EBs */
    process_start(&tsch_send_eb_process, NULL);
    /* try to associate to a network or start one if setup as coordinator */
    process_start(&tsch_process, NULL);
    PRINTF("TSCH: starting as %s\n", tsch_is_coordinator ? "coordinator" : "node");
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver tschmac_driver = {
  "TSCH",
  tsch_init,
  send_packet,
  packet_input,
  turn_on,
  turn_off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
