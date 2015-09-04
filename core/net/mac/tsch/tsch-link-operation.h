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

#ifndef __TSCH_LINK_OPERATION_H__
#define __TSCH_LINK_OPERATION_H__

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

struct input_packet {
  uint8_t payload[TSCH_MAX_PACKET_LEN];
  struct asn_t rx_asn;
  int len;
  uint16_t rssi;
};

extern struct ringbufindex dequeued_ringbuf;
extern struct tsch_packet *dequeued_array[TSCH_DEQUEUED_ARRAY_SIZE];
extern struct ringbufindex input_ringbuf;
extern struct input_packet input_array[TSCH_MAX_INCOMING_PACKETS];

void tsch_start_link_operation(void);
void tsch_leave_network();
/* Protothread for association */
PT_THREAD(tsch_associate(struct pt *pt));

#endif /* __TSCH_LINK_OPERATION_H__ */
