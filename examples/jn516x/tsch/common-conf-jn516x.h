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

#ifndef __COMMON_CONF_JN516X_H__
#define __COMMON_CONF_JN516X_H__

/* Shall we restart after exception, or stall?
 * in production code we should restart, so set this to 0 */
#undef EXCEPTION_STALLS_SYSTEM
#define EXCEPTION_STALLS_SYSTEM 1

 /* CoAP */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS 4

#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE      256

/* Network config */
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG     1
#undef UIP_CONF_BUFFER_SIZE
//#define UIP_CONF_BUFFER_SIZE   (REST_MAX_CHUNK_SIZE + UIP_LLH_LEN + UIP_IPUDPH_LEN + COAP_MAX_HEADER_SIZE)
//#define UIP_CONF_BUFFER_SIZE   (REST_MAX_CHUNK_SIZE + 0 + 48 + 70)
#define UIP_CONF_BUFFER_SIZE     1280 /* ipv6 required minimum */
#undef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS       8

/* No IPv6 reassembly */
#undef UIP_CONF_IPV6_REASSEMBLY
#define UIP_CONF_IPV6_REASSEMBLY 0

/* Timeout for packet reassembly at the 6lowpan layer (should be < 60s) */
#undef SICSLOWPAN_CONF_MAXAGE
#define SICSLOWPAN_CONF_MAXAGE 10

/* Queues */
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 32

#undef TSCH_QUEUE_CONF_NUM_PER_NEIGHBOR
#define TSCH_QUEUE_CONF_NUM_PER_NEIGHBOR 32

#undef TSCH_CONF_DEQUEUED_ARRAY_SIZE
#define TSCH_CONF_DEQUEUED_ARRAY_SIZE 32

#undef TSCH_QUEUE_CONF_MAX_NEIGHBOR_QUEUES
#define TSCH_QUEUE_CONF_MAX_NEIGHBOR_QUEUES 8

/* The neighbor table size */
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 8

/* The routing table size */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES  28

/* Radio */

#undef ENABLE_COOJA_DEBUG
#define ENABLE_COOJA_DEBUG 0

/* max 3, min 0 */

#undef UART_HW_FLOW_CTRL
#define UART_HW_FLOW_CTRL 0

#undef UART_XONXOFF_FLOW_CTRL
#define UART_XONXOFF_FLOW_CTRL 1

#undef UART_BAUD_RATE
#define UART_BAUD_RATE UART_RATE_1000000

#endif /* __COMMON_CONF_JN516X_H__ */
