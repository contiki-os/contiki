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

/* Default IEEE 802.15.4e hopping sequences, obtained from https://gist.github.com/twatteyne/2e22ee3c1a802b685695 */
/* 16 channels, sequence length 16 */
#define TSCH_HOPPING_SEQUENCE_16_16 (uint8_t[]){16, 17, 23, 18, 26, 15, 25, 22, 19, 11, 12, 13, 24, 14, 20, 21}
/* 4 channels, sequence length 16 */
#define TSCH_HOPPING_SEQUENCE_4_16 (uint8_t[]){20, 26, 25, 26, 15, 15, 25, 20, 26, 15, 26, 25, 20, 15, 20, 25}
/* 4 channels, sequence length 4 */
#define TSCH_HOPPING_SEQUENCE_4_4 (uint8_t[]){15, 25, 26, 20}
/* 1 channel, sequence length 1 */
#define TSCH_HOPPING_SEQUENCE_1_1 (uint8_t[]){25}

/* Default hopping sequence, used in case hopping sequence ID == 0 */
#ifdef TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
#define TSCH_DEFAULT_HOPPING_SEQUENCE TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
#else
#define TSCH_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_16_16
#endif

/* Hopping sequence used for joining (scan channels) */
#ifdef TSCH_CONF_JOIN_HOPPING_SEQUENCE
#define TSCH_JOIN_HOPPING_SEQUENCE TSCH_DEFAULT_HOPPING_SEQUENCE
#else
#define TSCH_JOIN_HOPPING_SEQUENCE TSCH_DEFAULT_HOPPING_SEQUENCE
#endif

/* Maximum length of the TSCH channel hopping sequence.
 * Set to a larger value to support joining a network with a hopping sequence
 * longer than the default sequence TSCH_DEFAULT_HOPPING_SEQUENCE */
#ifdef TSCH_CONF_HOPPING_SEQUENCE_MAX_LEN
#define TSCH_HOPPING_SEQUENCE_MAX_LEN TSCH_CONF_HOPPING_SEQUENCE_MAX_LEN
#else
#define TSCH_HOPPING_SEQUENCE_MAX_LEN sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE)
#endif

/* Initializes TSCH with a 6TiSCH minimal schedule */
#ifdef TSCH_CONF_WITH_MINIMAL_SCHEDULE
#define TSCH_WITH_MINIMAL_SCHEDULE TSCH_CONF_WITH_MINIMAL_SCHEDULE
#else
#define TSCH_WITH_MINIMAL_SCHEDULE 1
#endif

/* 6TiSCH Minimal schedule slotframe length */
#ifdef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_DEFAULT_LENGTH TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#else
#define TSCH_SCHEDULE_DEFAULT_LENGTH 7
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

/* Period between two consecutive EBs */
#ifdef TSCH_CONF_EB_PERIOD
#define TSCH_EB_PERIOD TSCH_CONF_EB_PERIOD
#else
#define TSCH_EB_PERIOD (10 * CLOCK_SECOND)
#endif

/* Max acceptable join priority */
#ifdef TSCH_CONF_MAX_JOIN_PRIORITY
#define TSCH_MAX_JOIN_PRIORITY TSCH_CONF_MAX_JOIN_PRIORITY
#else
#define TSCH_MAX_JOIN_PRIORITY 32
#endif

/* Max number of links */
#ifdef TSCH_CONF_MAX_LINKS
#define TSCH_MAX_LINKS TSCH_CONF_MAX_LINKS
#else
#define TSCH_MAX_LINKS 32
#endif

/* TSCH MAC parameters */
#ifdef TSCH_CONF_MAC_MIN_BE
#define TSCH_MAC_MIN_BE TSCH_CONF_MAC_MIN_BE
#else
#define TSCH_MAC_MIN_BE 1
#endif
#ifdef TSCH_CONF_MAC_MAX_BE
#define TSCH_MAC_MAX_BE TSCH_CONF_MAC_MAX_BE
#else
#define TSCH_MAC_MAX_BE 7
#endif
#ifdef TSCH_CONF_MAC_MAX_FRAME_RETRIES
#define TSCH_MAC_MAX_FRAME_RETRIES TSCH_CONF_MAC_MAX_FRAME_RETRIES
#else
#define TSCH_MAC_MAX_FRAME_RETRIES 3
#endif

/* TSCH packet len */
#define TSCH_MAX_PACKET_LEN 127
#define TSCH_BASE_ACK_LEN 3
#define TSCH_SYNC_IE_LEN 4
#define TSCH_EACK_DEST_LEN (LINKADDR_SIZE+2) /* Dest PAN ID + dest MAC address */
#ifdef TSCH_CONF_PACKET_DEST_ADDR_IN_ACK
#define TSCH_PACKET_DEST_ADDR_IN_ACK TSCH_CONF_PACKET_DEST_ADDR_IN_ACK
#else
#define TSCH_PACKET_DEST_ADDR_IN_ACK 0
#endif
#define TSCH_MAX_ACK_LEN (TSCH_BASE_ACK_LEN \
                          + TSCH_EACK_DEST_LEN \
                          + TSCH_SYNC_IE_LEN)

/* Timeslot timing */

#ifndef TSCH_CONF_DEFAULT_TIMESLOT_LENGTH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 10000
#endif

#if TSCH_CONF_DEFAULT_TIMESLOT_LENGTH == 10000
/* Default timeslot timing as per IEEE 802.15.4e */

#define TSCH_DEFAULT_TS_CCA_OFFSET         1800
#define TSCH_DEFAULT_TS_CCA                128
#define TSCH_DEFAULT_TS_TX_OFFSET          2120
/* By standard, TS_RX_OFFSET is 1120us by default. To have the guard time
 * equally spent before and after the expected reception, use
 * (TS_TX_OFFSET - (TS_RX_WAIT / 2)) instead */
#define TSCH_DEFAULT_TS_RX_OFFSET          1120
#define TSCH_DEFAULT_TS_RX_ACK_DELAY       800
#define TSCH_DEFAULT_TS_TX_ACK_DELAY       1000
#define TSCH_DEFAULT_TS_RX_WAIT            2200
#define TSCH_DEFAULT_TS_ACK_WAIT           400
#define TSCH_DEFAULT_TS_RX_TX              192
#define TSCH_DEFAULT_TS_MAX_ACK            2400
#define TSCH_DEFAULT_TS_MAX_TX             4256
#define TSCH_DEFAULT_TS_TIMESLOT_LENGTH    10000

#elif TSCH_CONF_DEFAULT_TIMESLOT_LENGTH == 15000
/* Default timeslot timing for platfroms requiring 15ms slots */

#define TSCH_DEFAULT_TS_CCA_OFFSET         1800
#define TSCH_DEFAULT_TS_CCA                128
#define TSCH_DEFAULT_TS_TX_OFFSET          4000
#define TSCH_DEFAULT_TS_RX_OFFSET          2900
#define TSCH_DEFAULT_TS_RX_ACK_DELAY       3600
#define TSCH_DEFAULT_TS_TX_ACK_DELAY       4000
#define TSCH_DEFAULT_TS_RX_WAIT            2200
#define TSCH_DEFAULT_TS_ACK_WAIT           800
#define TSCH_DEFAULT_TS_RX_TX              2072
#define TSCH_DEFAULT_TS_MAX_ACK            2400
#define TSCH_DEFAULT_TS_MAX_TX             4256
#define TSCH_DEFAULT_TS_TIMESLOT_LENGTH    15000

#else
#error "TSCH: Unsupported default timeslot length"
#endif

/* Calculate packet tx/rx duration in rtimer ticks based on sent
 * packet len in bytes with 802.15.4 250kbps data rate.
 * One byte = 32us. Add two bytes for CRC and one for len field */
#define TSCH_PACKET_DURATION(len) US_TO_RTIMERTICKS(32 * ((len) + 3))

/* Convert rtimer ticks to clock and vice versa */
#define TSCH_CLOCK_TO_TICKS(c) (((c)*RTIMER_SECOND)/CLOCK_SECOND)
#define TSCH_CLOCK_TO_SLOTS(c, timeslot_length) (TSCH_CLOCK_TO_TICKS(c)/timeslot_length)

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

#endif /* __TSCH_PRIVATE_H__ */
