/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
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
 *         IEEE 802.15.4e-specific frame creation and parsing
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef FRAME_802154E_H
#define FRAME_802154E_H

#include "contiki.h"
#include "net/mac/tsch/tsch-private.h"

/* The information elements that we currently support */
struct ieee802154_ies {
  /* Header IEs */
  int16_t ie_time_correction;
  uint8_t ie_is_nack;
  /* Payload MLME */
  uint16_t ie_mlme_len;
  /* Payload Short MLME IEs */
  uint8_t ie_tsch_synchronization_offset;
  struct asn_t ie_asn;
  uint8_t ie_join_priority;
  uint8_t ie_tsch_timeslot_id;
  struct tsch_timeslot_timing_t timeslot_timing;
  /* Payload Long MLME IEs */
  uint8_t ie_channel_hopping_sequence_id;
};

/** Insert various Information Elements **/
/* Header IE. ACK/NACK time correction. Used in enhanced ACKs */
int frame80215e_create_ie_ack_nack_time_correction(uint8_t *buf, int len,
    struct ieee802154_ies *ies);
/* Payload IE. MLME. Used to nest sub-IEs */
int frame80215e_create_ie_mlme(uint8_t *buf, int len,
    struct ieee802154_ies *ies);
/* MLME sub-IE. TSCH synchronization. Used in EBs: ASN and join priority */
int frame80215e_create_ie_tsch_synchronization(uint8_t *buf, int len,
    struct ieee802154_ies *ies);
/* MLME sub-IE. TSCH slotframe and link. Used in EBs: initial schedule */
int frame80215e_create_ie_tsch_slotframe_and_link(uint8_t *buf, int len,
    struct ieee802154_ies *ies);
/* MLME sub-IE. TSCH timeslot. Used in EBs: timeslot template (timing) */
int frame80215e_create_ie_tsch_timeslot(uint8_t *buf, int len,
    struct ieee802154_ies *ies);
/* MLME sub-IE. TSCH channel hopping sequence. Used in EBs: hopping sequence */
int frame80215e_create_ie_tsch_channel_hopping_sequence(uint8_t *buf, int len,
    struct ieee802154_ies *ies);

/* Parse all Information Elements of a frame */
int frame802154e_parse_information_elements(uint8_t *buf, uint8_t buf_size, struct ieee802154_ies *ies);

#endif /* FRAME_802154E_H */
