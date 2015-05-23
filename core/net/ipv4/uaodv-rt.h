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
 */

/**
 * \file
 *         Routing tables for the micro implementation of the AODV ad hoc routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef UAODV_RT_H_
#define UAODV_RT_H_

#include "contiki-net.h"

struct uaodv_rt_entry {
  struct uaodv_rt_entry *next;
  uip_ipaddr_t dest;
  uip_ipaddr_t nexthop;
  uint32_t hseqno;			/* In host byte order! */
  uint8_t hop_count;
  uint8_t is_bad;			/* Only one bit is used. */
};

struct uaodv_rt_entry *
uaodv_rt_add(uip_ipaddr_t *dest, uip_ipaddr_t *nexthop,
	     unsigned hop_count, const uint32_t *seqno);
struct uaodv_rt_entry *uaodv_rt_lookup_any(uip_ipaddr_t *dest);
struct uaodv_rt_entry *uaodv_rt_lookup(uip_ipaddr_t *dest);
void uaodv_rt_remove(struct uaodv_rt_entry *e);
void uaodv_rt_lru(struct uaodv_rt_entry *e);
void uaodv_rt_flush_all(void);

#endif /* UAODV_RT_H_ */
