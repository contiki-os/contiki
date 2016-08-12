/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 *         Simon Duquennoy <simonduq@sics.se>
 *         Beshr Al Nahas <beshr@sics.se>
 */

#ifndef __TSCH_PRIVATE_H__
#define __TSCH_PRIVATE_H__

/********** Includes **********/

#include "contiki.h"
#include "net/linkaddr.h"
#include "net/mac/tsch/tsch-asn.h"
#include "net/mac/tsch/tsch-conf.h"

/************ Types ***********/

/* TSCH timeslot timing elements. Used to index timeslot timing
 * of different units, such as rtimer tick or micro-second */
enum tsch_timeslot_timing_elements {
  tsch_ts_cca_offset,
  tsch_ts_cca,
  tsch_ts_tx_offset,
  tsch_ts_rx_offset,
  tsch_ts_rx_ack_delay,
  tsch_ts_tx_ack_delay,
  tsch_ts_rx_wait,
  tsch_ts_ack_wait,
  tsch_ts_rx_tx,
  tsch_ts_max_ack,
  tsch_ts_max_tx,
  tsch_ts_timeslot_length,
  tsch_ts_elements_count, /* Not a timing element */
};

/***** External Variables *****/

/* 802.15.4 broadcast MAC address */
extern const linkaddr_t tsch_broadcast_address;
/* The address we use to identify EB queue */
extern const linkaddr_t tsch_eb_address;
/* The current Absolute Slot Number (ASN) */
extern struct asn_t current_asn;
extern uint8_t tsch_join_priority;
extern struct tsch_link *current_link;
/* TSCH channel hopping sequence */
extern uint8_t tsch_hopping_sequence[TSCH_HOPPING_SEQUENCE_MAX_LEN];
extern struct asn_divisor_t tsch_hopping_sequence_length;
/* TSCH timeslot timing (in rtimer ticks) */
extern rtimer_clock_t tsch_timing[tsch_ts_elements_count];

/* TSCH processes */
PROCESS_NAME(tsch_process);
PROCESS_NAME(tsch_send_eb_process);
PROCESS_NAME(tsch_pending_events_process);

/********** Functions *********/

/* Set TSCH to send a keepalive message after TSCH_KEEPALIVE_TIMEOUT */
void tsch_schedule_keepalive(void);
/* Leave the TSCH network */
void tsch_disassociate(void);

/************ Macros **********/

/* Calculate packet tx/rx duration in rtimer ticks based on sent
 * packet len in bytes with 802.15.4 250kbps data rate.
 * One byte = 32us. Add two bytes for CRC and one for len field */
#define TSCH_PACKET_DURATION(len) US_TO_RTIMERTICKS(32 * ((len) + 3))

/* Convert rtimer ticks to clock and vice versa */
#define TSCH_CLOCK_TO_TICKS(c) (((c) * RTIMER_SECOND) / CLOCK_SECOND)
#define TSCH_CLOCK_TO_SLOTS(c, timeslot_length) (TSCH_CLOCK_TO_TICKS(c) / timeslot_length)

/* Wait for a condition with timeout t0+offset. */
#define BUSYWAIT_UNTIL_ABS(cond, t0, offset) \
  while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), (t0) + (offset))) ;

#endif /* __TSCH_PRIVATE_H__ */
