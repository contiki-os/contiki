/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: uaodv-rt.c,v 1.2 2006/08/09 16:13:39 bg- Exp $
 */

/**
 * \file
 *         Routing tables for the micro implementation of the AODV ad hoc routing protocol
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


#include "net/uaodv-rt.h"
#include "contiki-net.h"

#define NUM_RT_ENTRIES 8

/*
 * LRU (with respect to insertion time) list of route entries.
 */
LIST(route_table);
MEMB(route_mem, struct uaodv_rt_entry, NUM_RT_ENTRIES);

/*---------------------------------------------------------------------------*/
void
uaodv_rt_init(void)
{
  list_init(route_table);
  memb_init(&route_mem);
}
/*---------------------------------------------------------------------------*/
int
uaodv_rt_add(uip_ipaddr_t *dest, uip_ipaddr_t *nexthop,
	     u16_t hop_count, u32_t seqno)
{
  struct uaodv_rt_entry *e;

  /* Avoid inserting duplicate entries. */
  e = uaodv_rt_lookup(dest);
  if(e != NULL) {
    list_remove(route_table, e);    
  } else {
    /* Allocate a new entry or reuse the oldest. */
    e = memb_alloc(&route_mem);
    if(e == NULL) {
      e = list_chop(route_table); /* Remove oldest entry. */
    }
  }

  uip_ipaddr_copy(&e->dest, dest);
  uip_ipaddr_copy(&e->nexthop, nexthop);
  e->hop_count = hop_count;
  e->seqno = seqno;

  /* New entry goes first. */
  list_push(route_table, e);

  return 0;
}
/*---------------------------------------------------------------------------*/
struct uaodv_rt_entry *
uaodv_rt_lookup(uip_ipaddr_t *dest)
{
  struct uaodv_rt_entry *e;

  for(e = list_head(route_table); e != NULL; e = e->next) {
    /*    printf("uaodv_rt_lookup: comparing %d.%d.%d.%d with %d.%d.%d.%d\n",
	   uip_ipaddr1(dest),
	   uip_ipaddr2(dest),
	   uip_ipaddr3(dest),
	   uip_ipaddr4(dest),
	   uip_ipaddr1(&e->dest),
	   uip_ipaddr2(&e->dest),
	   uip_ipaddr3(&e->dest),
	   uip_ipaddr4(&e->dest));*/
	   
    if(uip_ipaddr_cmp(dest, &e->dest)) {
      return e;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
uaodv_rt_remove(struct uaodv_rt_entry *e)
{
  list_remove(route_table, e);
  memb_free(&route_mem, e);
}
/*---------------------------------------------------------------------------*/
void
uaodv_rt_flush_all(void)
{
  struct uaodv_rt_entry *e;

  while (1) {
    e = list_pop(route_table);
    if(e != NULL)
      memb_free(&route_mem, e);
    else
      break;
  }
}
