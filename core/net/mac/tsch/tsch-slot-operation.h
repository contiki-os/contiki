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

#ifndef __TSCH_SLOT_OPERATION_H__
#define __TSCH_SLOT_OPERATION_H__

#include "contiki.h"
#include "lib/ringbufindex.h"
#include "net/mac/tsch/tsch-private.h"

/* Ringbuf for dequeued outgoing packets */
#ifdef TSCH_CONF_TSCH_DEQUEUED_ARRAY_SIZE
#define TSCH_DEQUEUED_ARRAY_SIZE TSCH_CONF_TSCH_DEQUEUED_ARRAY_SIZE
#else
#define TSCH_DEQUEUED_ARRAY_SIZE 16
#endif
#if TSCH_DEQUEUED_ARRAY_SIZE < QUEUEBUF_NUM
#error TSCH_DEQUEUED_ARRAY_SIZE must be greater than QUEUEBUF_NUM
#endif
#if (TSCH_DEQUEUED_ARRAY_SIZE & (TSCH_DEQUEUED_ARRAY_SIZE-1)) != 0
#error TSCH_QUEUE_NUM_PER_NEIGHBOR must be power of two
#endif

/* Ringbuf for incoming packets: must be power of two for atomic ringbuf operation */
#ifdef TSCH_CONF_MAX_INCOMING_PACKETS
#define TSCH_MAX_INCOMING_PACKETS TSCH_CONF_MAX_INCOMING_PACKETS
#else
#define TSCH_MAX_INCOMING_PACKETS 4
#endif
#if (TSCH_MAX_INCOMING_PACKETS & (TSCH_MAX_INCOMING_PACKETS-1)) != 0
#error TSCH_MAX_INCOMING_PACKETS must be power of two
#endif

/* Stores data about an incoming packet */
struct input_packet {
  uint8_t payload[TSCH_MAX_PACKET_LEN]; /* Packet payload */
  struct asn_t rx_asn; /* ASN when the packet was received */
  int len; /* Packet len */
  uint16_t rssi; /* RSSI for this packet */
};

/* A ringbuf storing outgoing packets after they were dequeued.
 * Will be processed layer by tsch_tx_process_pending */
extern struct ringbufindex dequeued_ringbuf;
extern struct tsch_packet *dequeued_array[TSCH_DEQUEUED_ARRAY_SIZE];
/* A ringbuf storing incoming packets.
 * Will be processed layer by tsch_rx_process_pending */
extern struct ringbufindex input_ringbuf;
extern struct input_packet input_array[TSCH_MAX_INCOMING_PACKETS];

/* Returns a 802.15.4 channel from an ASN and channel offset */
uint8_t tsch_calculate_channel(struct asn_t *asn, uint8_t channel_offset);

/* Is TSCH locked? */
int tsch_is_locked();
/* Lock TSCH (no link operation) */
int tsch_get_lock();
/* Release TSCH lock */
void tsch_release_lock();

/* Set global time before starting slot operation,
 * with a rtimer time and an ASN */
void tsch_slot_operation_sync(rtimer_clock_t next_slot_start,
    struct asn_t *next_slot_asn);
/* Start actual slot operation */
void tsch_slot_operation_start(void);

#endif /* __TSCH_SLOT_OPERATION_H__ */
