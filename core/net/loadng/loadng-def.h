/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * $Id: loadng-def.h,v 1.5 2010/05/28 16:33:19 bg- Exp $
 */

/**
 * \file
 *         Definitions for the LOADng ad hoc routing protocol
 * \author 
 *         Chi-Anh La la@imag.fr         
 */

#ifndef __LOADNG_DEF_H__
#define __LOADNG_DEF_H__

#include "net/uip.h"
#define uip_create_linklocal_lln_routers_mcast(a) uip_ip6addr(a, 0xff02, 0, 0, 0, 0, 0, 0, 0x001b)
#define uip_create_linklocal_empty_addr(a) uip_ip6addr(a, 0, 0, 0, 0, 0, 0, 0, 0)
#define LOADNG_UDPPORT            6666
#define LOADNG_NET_TRAVERSAL_TIME 10
#define LOADNG_RREQ_RETRIES       0
#define LOADNG_RREQ_RATELIMIT     0
#define LOADNG_R_HOLD_TIME        0
#define LOADNG_MAX_DIST           20
#define LOADNG_RREP_ACK_TIMEOUT   10
#define LOADNG_BLACKLIST_TIME     10
#define LOADNG_RSSI_THRESHOLD    -65 // Ana measured value
/* Self multicast OPT to create a default route at all nodes (for sink only) */


#ifdef LOADNG_CONF_IS_SINK
#define LOADNG_IS_SINK LOADNG_CONF_IS_SINK
#else
#define LOADNG_IS_SINK 1
#endif

#ifdef LOADNG_CONF_IS_COORDINATOR() 
#define LOADNG_IS_COORDINATOR() LOADNG_CONF_IS_COORDINATOR()
#else
#define LOADNG_IS_COORDINATOR() 0
#endif

#ifdef LOADNG_CONF_IS_SKIP_LEAF
#define LOADNG_IS_SKIP_LEAF LOADNG_CONF_IS_SKIP_LEAF
#else
#define LOADNG_IS_SKIP_LEAF 0
#endif

#define LOADNG_RREP_ACK           0 
#define LOADNG_ADDR_LEN_IPV6      15
#define LOADNG_METRIC_HC          0 
#define LOADNG_WEAK_LINK          0
#define LOADNG_RSVD1              0
#define LOADNG_RSVD2              0 
#define LOADNG_DEFAULT_ROUTE_LIFETIME  65534

#ifdef LOADNG_CONF_RANDOM_WAIT
#define LOADNG_RANDOM_WAIT LOADNG_CONF_RANDOM_WAIT
#else
#define LOADNG_RANDOM_WAIT 1
#endif

/* Generic LOADng message */
struct loadng_msg {
	uint8_t type;
};


/* LOADng RREQ message */
#define LOADNG_RREQ_TYPE     0

struct loadng_msg_rreq {
	uint8_t type;
	uint8_t addr_len;
	uint16_t seqno;
	uint8_t metric;
	uint8_t route_cost;
	uip_ipaddr_t dest_addr;
	uip_ipaddr_t orig_addr;
};

/* LOADng RREP message */
#define LOADNG_RREP_TYPE     1

struct loadng_msg_rrep {
	uint8_t type;
	uint8_t addr_len;
	uint16_t seqno;
	uint8_t metric;
	uint8_t route_cost;
	uip_ipaddr_t dest_addr;
	uip_ipaddr_t orig_addr;
};

/* LOADng RREP-ACK message */
#define LOADNG_RACK_TYPE     2

struct loadng_msg_rack {
	uint8_t type;
	uint8_t addr_len;
	uip_ipaddr_t src_addr;
	uint16_t seqno;

};

/* LOADng RERR message */
#define LOADNG_RERR_TYPE     3

struct loadng_msg_rerr {
	uint8_t type;
	uint8_t addr_len;
	uip_ipaddr_t src_addr;
	uip_ipaddr_t dest_addr;

};


/* LOADng OPT message */
#define LOADNG_OPT_TYPE      4

struct loadng_msg_opt {
	uint8_t type;
	uint8_t addr_len;
	uint16_t seqno;
	int8_t rank;
        uint8_t metric;
	uip_ipaddr_t sink_addr;
};

/* LOADng QRY message */
#define LOADNG_QRY_TYPE      5

struct loadng_msg_qry {
	uint8_t type;
	uint8_t addr_len;
};

#endif /* __LOADNG_DEF_H__ */

