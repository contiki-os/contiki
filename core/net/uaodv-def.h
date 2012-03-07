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
 * $Id: uaodv-def.h,v 1.5 2007/05/28 16:33:19 bg- Exp $
 */

/**
 * \file
 *         Definitions for the micro implementation of the AODV ad hoc routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __UAODV_DEF_H__
#define __UAODV_DEF_H__

#include "net/uip.h"

#define NUM_PRECURSORS 4


#define UAODV_UDPPORT 654

#if 0
/* AODV routing table entry */
struct uaodv_rtentry {
  uip_ipaddr_t dest_addr;
  uip_ipaddr_t next_hop;
  uip_ipaddr_t precursors[NUM_PRECURSORS];
  uint32_t dest_seqno;  
  uint16_t lifetime;
  uint8_t dest_seqno_flag;
  uint8_t route_flags;
  uint8_t hop_count;
};
#endif

/* Generic AODV message */
struct uaodv_msg {
  uint8_t type;
};

/* AODV RREQ message */
#define UAODV_RREQ_TYPE     1
#define UAODV_RREQ_JOIN     (1 << 7)
#define UAODV_RREQ_REPAIR   (1 << 6)
#define UAODV_RREQ_GRATIOUS (1 << 5)
#define UAODV_RREQ_DESTONLY (1 << 4)
#define UAODV_RREQ_UNKSEQNO (1 << 3)

struct uaodv_msg_rreq {
  uint8_t type;
  uint8_t flags;
  uint8_t reserved;
  uint8_t hop_count;
  uint32_t rreq_id;
  uip_ipaddr_t dest_addr;
  uint32_t dest_seqno;
  uip_ipaddr_t orig_addr;
  uint32_t orig_seqno;
};

/* AODV RREP message */
#define UAODV_RREP_TYPE     2
#define UAODV_RREP_REPAIR  (1 << 7)
#define UAODV_RREP_ACK     (1 << 6)

struct uaodv_msg_rrep {
  uint8_t type;
  uint8_t flags;
  uint8_t prefix_sz;		/* prefix_sz:5 */
  uint8_t hop_count;
  uip_ipaddr_t dest_addr;
  uint32_t dest_seqno;
  uip_ipaddr_t orig_addr;
  uint32_t lifetime;
};

/* AODV RERR message */
#define UAODV_RERR_TYPE     3
#define UAODV_RERR_NODELETE (1 << 7)
#define UAODV_RERR_UNKNOWN  (1 << 6) /* Non standard extension /bg. */

struct uaodv_msg_rerr {
  uint8_t type;
  uint8_t flags;
  uint8_t reserved;
  uint8_t dest_count;
  struct {
    uip_ipaddr_t addr;
    uint32_t seqno;
  } unreach[1];
};

/* AODV RREP-ACK message */
#define UAODV_RREP_ACK_TYPE     4

struct uaodv_msg_rrep_ack {
  uint8_t type;
  uint8_t reserved;
};

#define RREP_HELLO_INTERVAL_EXT 1 /* Per RFC 3561. */
#define RREQ_BAD_HOP_EXT 101	/* Non standard extension /bg */

struct uaodv_extension {
  uint8_t type;
  uint8_t length;
  /* uint8_t value[length]; */
};

struct uaodv_bad_hop_ext {
  uint8_t type;
  uint8_t length;
  uint8_t unused1, unused2;
  uip_ipaddr_t addrs[1];
};

#endif /* __UAODV_DEF_H__ */
