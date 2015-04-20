/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 *         Private TSCH definitions
 *         (meant for use by TSCH implementation files only)
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __TSCH_PRIVATE_H__
#define __TSCH_PRIVATE_H__

#include "contiki.h"
#include "net/linkaddr.h"
#include "net/mac/tsch/tsch-asn.h"

/* Initializes TSCH with a 6TiSCH minimal schedule */
#ifdef TSCH_CONF_WITH_MINIMAL_SCHEDULE
#define TSCH_WITH_MINIMAL_SCHEDULE TSCH_CONF_WITH_MINIMAL_SCHEDULE
#else
#define TSCH_WITH_MINIMAL_SCHEDULE 1
#endif

/* Max time before sending a unicast keep-alive message to the time source */
#ifdef TSCH_CONF_KEEPALIVE_TIMEOUT
#define TSCH_KEEPALIVE_TIMEOUT TSCH_CONF_KEEPALIVE_TIMEOUT
#else
/* Time to desynch assuming a drift of 40 PPM (80 PPM between two nodes) and guard time of +/-1ms: 12.5s. */
#define TSCH_KEEPALIVE_TIMEOUT (12 * CLOCK_SECOND)
#endif

/* Max time without synchronization before leaving the PAN */
#ifdef TSCH_CONF_DESYNC_THRESHOLD
#define TSCH_DESYNC_THRESHOLD TSCH_CONF_DESYNC_THRESHOLD
#else
#define TSCH_DESYNC_THRESHOLD (4 * TSCH_KEEPALIVE_TIMEOUT)
#endif

/* Min period between two consecutive EBs */
#ifdef TSCH_CONF_MIN_EB_PERIOD
#define TSCH_MIN_EB_PERIOD TSCH_CONF_MIN_EB_PERIOD
#else
#define TSCH_MIN_EB_PERIOD (4 * CLOCK_SECOND)
#endif

/* Max period between two consecutive EBs */
#ifdef TSCH_CONF_MAX_EB_PERIOD
#define TSCH_MAX_EB_PERIOD TSCH_CONF_MAX_EB_PERIOD
#else
#define TSCH_MAX_EB_PERIOD (60 * CLOCK_SECOND)
#endif

/* Max acceptable join priority */
#ifdef TSCH_CONF_MAX_JOIN_PRIORITY
#define TSCH_MAX_JOIN_PRIORITY TSCH_CONF_MAX_JOIN_PRIORITY
#else
#define TSCH_MAX_JOIN_PRIORITY 32
#endif

/* Rx guard time (TS_LONG_GT) */
#ifdef TSCH_CONF_GUARD_TIME
#define TSCH_GUARD_TIME TSCH_CONF_GUARD_TIME
#else
#define TSCH_GUARD_TIME 1000
#endif

/* Max number of links */
#ifdef TSCH_CONF_MAX_LINKS
#define TSCH_MAX_LINKS TSCH_CONF_MAX_LINKS
#else
#define TSCH_MAX_LINKS 32
#endif

/* TSCH MAC parameters */
#define MAC_MIN_BE 0
#define MAC_MAX_FRAME_RETRIES 8
#define MAC_MAX_BE 4

/* TSCH packet len */
#define TSCH_MAX_PACKET_LEN 127
#define TSCH_BASE_ACK_LEN 3
#define TSCH_SYNC_IE_LEN 4
#define TSCH_EACK_DEST_LEN (LINKADDR_SIZE+2) /* Dest PAN ID + dest MAC address */
/* Send enhanced ACK with Information Elements or Std ACK? */
#define TSCH_PACKET_WITH_SYNC_IE 1
#ifdef TSCH_CONF_PACKET_DEST_ADDR_IN_ACK
#define TSCH_PACKET_DEST_ADDR_IN_ACK TSCH_CONF_PACKET_DEST_ADDR_IN_ACK
#else
#define TSCH_PACKET_DEST_ADDR_IN_ACK 0
#endif
#define TSCH_ACK_LEN (TSCH_BASE_ACK_LEN \
                      + TSCH_PACKET_WITH_SYNC_IE*TSCH_SYNC_IE_LEN \
                      + TSCH_PACKET_DEST_ADDR_IN_ACK*TSCH_EACK_DEST_LEN)

/* Timeslot timing */

#ifndef TSCH_CONF_SLOT_DURATION
#define TSCH_CONF_SLOT_DURATION 10000
#endif

#if TSCH_CONF_SLOT_DURATION == 10000
#define TS_CCA_OFFSET         US_TO_RTIMERTICKS(1800)
#define TS_CCA                US_TO_RTIMERTICKS(128)
#define TS_TX_ACK_DELAY       US_TO_RTIMERTICKS(1000)
#define TS_TX_OFFSET          US_TO_RTIMERTICKS(2120)
#define TS_SLOT_DURATION      US_TO_RTIMERTICKS(10000)
#define TS_SHORT_GT           US_TO_RTIMERTICKS(200)
#elif TSCH_CONF_SLOT_DURATION == 15000
#define TS_CCA_OFFSET         US_TO_RTIMERTICKS(1800)
#define TS_CCA                US_TO_RTIMERTICKS(128)
#define TS_TX_ACK_DELAY       US_TO_RTIMERTICKS(4000)
#define TS_TX_OFFSET          US_TO_RTIMERTICKS(4000)
#define TS_SLOT_DURATION      US_TO_RTIMERTICKS(15000)
#define TS_SHORT_GT           US_TO_RTIMERTICKS(400)
#else
#error "TSCH: Unsupported slot duration"
#endif

#define TS_LONG_GT            US_TO_RTIMERTICKS(TSCH_GUARD_TIME)
#define TS_RX_TX              (TS_TX_OFFSET - tsCCA - tsCCAOffset)
#define TS_RW_WAIT            (2 * TS_LONG_GT)
#define TS_ACK_WAIT           (2 * TS_SHORT_GT)
#define TS_RX_ACK_DELAY       (TS_TX_ACK_DELAY - (tsAckWait / 2))

/* Calculate packet tx/rx duration in rtimer ticks based on sent
 * packet len in bytes with 802.15.4 250kbps data rate.
 * One byte = 32us. Add two bytes for CRC and one for len field */
#define TSCH_PACKET_DURATION(len) US_TO_RTIMERTICKS(32 * ((len) + 3))

/* Max time to wait for end of packet reception. Add a 350us margin. */
#define TSCH_DATA_MAX_DURATION ((unsigned)(TSCH_PACKET_DURATION(TSCH_MAX_PACKET_LEN) + US_TO_RTIMERTICKS(350)))
#define TSCH_ACK_MAX_DURATION  ((unsigned)(TSCH_PACKET_DURATION(TSCH_ACK_LEN) + US_TO_RTIMERTICKS(350)))

/* Convert rtimer ticks to clock and vice versa */
#define TSCH_CLOCK_TO_TICKS(c) (((c)*RTIMER_SECOND)/CLOCK_SECOND)
#define TSCH_CLOCK_TO_SLOTS(c) (TSCH_CLOCK_TO_TICKS(c)/TS_SLOT_DURATION)

/* 802.15.4 broadcast MAC address */
extern const linkaddr_t tsch_broadcast_address;
/* The address we use to identify EB queue */
extern const linkaddr_t tsch_eb_address;
/* The current Absolute Slot Number (ASN) */
extern struct asn_t current_asn;
extern uint8_t tsch_join_priority;
extern struct tsch_link *current_link;
/* Are we coordinator of the TSCH network? */
extern int tsch_is_coordinator;
/* Are we associated to a TSCH network? */
extern int tsch_is_associated;

/* Is TSCH locked? */
int tsch_is_locked();
/* Lock TSCH (no link operation) */
int tsch_get_lock();
/* Release TSCH lock */
void tsch_release_lock();
/* Returns a 802.15.4 channel from an ASN and channel offset */
uint8_t tsch_calculate_channel(struct asn_t *asn, uint8_t channel_offset);

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif /* MAX */

#endif /* __TSCH_PRIVATE_H__ */
