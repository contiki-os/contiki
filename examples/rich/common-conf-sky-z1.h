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
 */
/**
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __COMMON_CONF_SKY_Z1_H__
#define __COMMON_CONF_SKY_Z1_H__

#if WITH_TSCH_SECURITY
/* We need longer slots to accommodate crypto operations */
#undef TSCH_CONF_DEFAULT_TIMESLOT_LENGTH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 65000
#endif /* WITH_TSCH_SECURITY */

 /* CoAP */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS 2

#undef COAP_CONF_MAX_OBSERVEES
#define COAP_CONF_MAX_OBSERVEES 2

#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE      64

/* Queues */
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 4

#undef TSCH_QUEUE_CONF_NUM_PER_NEIGHBOR
#define TSCH_QUEUE_CONF_NUM_PER_NEIGHBOR 4

/* The routing table size */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  0

/* Network config */
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG     1
#undef UIP_CONF_BUFFER_SIZE
//#define UIP_CONF_BUFFER_SIZE   (REST_MAX_CHUNK_SIZE + UIP_LLH_LEN + UIP_IPUDPH_LEN + COAP_MAX_HEADER_SIZE)
#define UIP_CONF_BUFFER_SIZE   (REST_MAX_CHUNK_SIZE + 0 + 48 + 70)
#undef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS       1

/* The neighbor table size */
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 4

/* The routing table size */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  4

/* Radio */

/* Needed for TSCH */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED 0

#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS 1

/* NullRDC: check for autoack in S/W */
#undef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_CONF_802154_AUTOACK 1

#endif /* __COMMON_CONF_SKY_Z1_H__ */
