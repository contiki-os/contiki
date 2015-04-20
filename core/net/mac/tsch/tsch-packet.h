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
 *         TSCH packet format management
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __TSCH_PACKET_H__
#define __TSCH_PACKET_H__

#include "contiki.h"
#include "net/mac/tsch/tsch-private.h"

/* Return values for tsch_packet_parse_frame_type */
#define DO_ACK 2
#define IS_DATA 4
#define IS_ACK 8
#define IS_EB 16

/* Return values for tsch_packet_parse_sync_ack */
#define TSCH_ACK_OK 2
#define TSCH_ACK_HAS_SYNC_IE 4

/* Construct enhanced ACK packet and return ACK length */
int tsch_packet_make_sync_ack(int32_t drift, int nack,
    uint8_t *ackbuf, int ackbuf_len, linkaddr_t *dest_addr, uint8_t seqno);

/* Parse enhanced ACK packet, extract drift and nack */
int tsch_packet_parse_sync_ack(int32_t *drift, int *nack,
    uint8_t *ackbuf, int ackbuf_len, uint8_t seqno, int extract_sync_ie);

/* Create an EB packet */
int tsch_packet_make_eb(uint8_t* const buf, uint8_t buf_size, uint8_t seqno);

/* Extract addresses from raw packet */
int tsch_packet_extract_addresses(uint8_t *buf, uint8_t len, linkaddr_t *source_address, linkaddr_t *dest_address);

/* Parse EB and extract ASN and join priority */
uint8_t tsch_parse_eb(uint8_t *buf, uint8_t buf_len, linkaddr_t *source_address, struct asn_t *asn, uint8_t *join_priority);

/* Update ASN in EB packet */
int tsch_packet_update_eb(uint8_t *buf, uint8_t buf_len);

/* Extract 802.15.4 frame type from FCF least-significant byte */
uint8_t tsch_packet_parse_frame_type_from_fcf_lsb(uint8_t fcf_lsb);

/* Extract 802.15.4 frame type from a struct packet_input */
uint8_t tsch_packet_parse_frame_type(uint8_t *buf, uint8_t len, uint8_t *seqno);

#endif /* __TSCH_PACKET_H__ */
