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
 * $Id: neighbor.c,v 1.3 2007/03/15 19:43:07 adamdunkels Exp $
 */

/**
 * \file
 *         Radio neighborhood management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <limits.h>

#include "contiki.h"
#include "net/rime/neighbor.h"

#define MAX_NEIGHBORS 5

#define HOPCOUNT_MAX 32

static struct neighbor neighbors[MAX_NEIGHBORS];
/*---------------------------------------------------------------------------*/
void
neighbor_periodic(int max_time)
{
  int i;
  
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
       neighbors[i].time < max_time) {
      neighbors[i].time++;
      if(neighbors[i].time == max_time) {
	neighbors[i].hopcount = HOPCOUNT_MAX;
	/*	printf("%d: removing old neighbor %d\n", node_id, neighbors[i].nodeid);*/
	rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_init(void)
{
  int i;

  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
  }
}
/*---------------------------------------------------------------------------*/
struct neighbor *
neighbor_find(rimeaddr_t *addr)
{
  int i;
  
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&neighbors[i].addr, addr)) {
      return &neighbors[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
neighbor_update(struct neighbor *n, u8_t hopcount, u16_t signal)
{
  if(n != NULL) {
    n->hopcount = hopcount;
    n->signal = signal;
    n->time = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_add(rimeaddr_t *addr, u8_t nhopcount, u16_t nsignal)
{
  int i, n;
  u8_t hopcount;
  u16_t signal;

  /* Find the first unused entry or the used entry with the highest
     hopcount and lowest signal strength. */
  hopcount = 0;
  signal = USHRT_MAX;

  n = 0;
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) ||
       rimeaddr_cmp(&neighbors[i].addr, addr)) {
      n = i;
      break;
    }
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null)) {
      if(neighbors[i].hopcount > hopcount) {
	hopcount = neighbors[i].hopcount;
	signal = neighbors[i].signal;
	n = i;
      } else if(neighbors[i].hopcount == hopcount) {
	if(neighbors[i].signal < signal) {
	  hopcount = neighbors[i].hopcount;
	  signal = neighbors[i].signal;
	  n = i;
	  /*	printf("%d: found worst neighbor %d with hopcount %d, signal %d\n",
		node_id, neighbors[n].nodeid, hopcount, signal);*/
	}
      }
    }
  }


  /*  printf("%d: adding neighbor %d with hopcount %d, signal %d at %d\n",
      node_id, neighbors[n].nodeid, hopcount, signal, n);*/

  neighbors[n].time = 0;
  rimeaddr_copy(&neighbors[i].addr, addr);
  neighbors[n].hopcount = nhopcount;
  neighbors[n].signal = nsignal;
}
/*---------------------------------------------------------------------------*/
void
neighbor_remove(rimeaddr_t *addr)
{
  int i;

  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&neighbors[i].addr, addr)) {
      printf("%d: removing %d @ %d\n", rimeaddr_node_addr.u16, addr->u16, i);
      rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
      neighbors[i].hopcount = HOPCOUNT_MAX;
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
struct neighbor *
neighbor_best(void)
{
  int i, found;
  int lowest, best;
  u8_t hopcount;
  u16_t signal;

  hopcount = HOPCOUNT_MAX;
  lowest = 0;
  found = 0;

  /*  printf("%d: ", node_id);*/
  
  /* Find the lowest hopcount. */
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    /*  printf("%d:%d ", neighbors[i].nodeid, neighbors[i].hopcount);*/
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
       hopcount > neighbors[i].hopcount) {
      hopcount = neighbors[i].hopcount;
      lowest = i;
      found = 1;
    }
  }
  /*  printf("\n");*/

  /* Find the neighbor with highest signal strength of the ones that
     have the lowest hopcount. */
  if(found) {
    signal = 0;
    best = lowest;
    for(i = 0; i < MAX_NEIGHBORS; ++i) {
      if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
	 hopcount == neighbors[i].hopcount &&
	 neighbors[i].signal > signal) {
	signal = neighbors[i].signal;
	best = i;
      }
    }
    return &neighbors[best];
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
