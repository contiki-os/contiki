/**
 * \addtogroup rimeneighbor
 * @{
 */

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
 * $Id: neighbor.c,v 1.13 2007/12/09 15:44:21 adamdunkels Exp $
 */

/**
 * \file
 *         Radio neighborhood management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <limits.h>
#include <stdio.h>

#include "contiki.h"
#include "net/rime/neighbor.h"
#include "net/rime/ctimer.h"
#include "net/rime/collect.h"

#define MAX_NEIGHBORS 5

#define RTMETRIC_MAX COLLECT_MAX_DEPTH

static struct neighbor neighbors[MAX_NEIGHBORS];

static struct ctimer t;

static int max_time = 30;
/*---------------------------------------------------------------------------*/
static void
periodic(void *ptr)
{
  int i;

  /* Go through all neighbors and remove old ones. */
  
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
       neighbors[i].time < max_time) {
      neighbors[i].time++;
      if(neighbors[i].time == max_time) {
	neighbors[i].rtmetric = RTMETRIC_MAX;
	/*	printf("%d: removing old neighbor %d\n", node_id, neighbors[i].nodeid);*/
	rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
      }
    }
  }
  /*  printf("neighbor periodic\n");*/
  ctimer_set(&t, CLOCK_SECOND, periodic, NULL);
}
/*---------------------------------------------------------------------------*/
void
neighbor_init(void)
{
  int i;

  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
  }
  
  ctimer_set(&t, CLOCK_SECOND, periodic, NULL);
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
neighbor_update(struct neighbor *n, u8_t rtmetric)
{
  if(n != NULL) {
    n->rtmetric = rtmetric;
    n->time = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_timedout_etx(struct neighbor *n, u8_t etx)
{
  if(n != NULL) {
    n->etxs[n->etxptr] = etx;
    n->etxptr = (n->etxptr + 1) % NEIGHBOR_NUM_ETXS;
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_update_etx(struct neighbor *n, u8_t etx)
{
  if(n != NULL) {
    n->etxs[n->etxptr] = etx;
    n->etxptr = (n->etxptr + 1) % NEIGHBOR_NUM_ETXS;
    n->time = 0;
  }
}
/*---------------------------------------------------------------------------*/
uint8_t
neighbor_etx(struct neighbor *n)
{
  int i, etx;

  etx = 0;
  for(i = 0; i < NEIGHBOR_NUM_ETXS; ++i) {
    etx += n->etxs[i];
  }
  return NEIGHBOR_ETX_SCALE * etx / NEIGHBOR_NUM_ETXS;
}
/*---------------------------------------------------------------------------*/
void
neighbor_add(rimeaddr_t *addr, u8_t nrtmetric, u8_t netx)
{
  int i, n;
  u8_t rtmetric;
  u8_t etx;

  /* Find the first unused entry or the used entry with the highest
     rtmetric and highest etx. */
  rtmetric = 0;
  etx = 0;

  n = 0;
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) ||
       rimeaddr_cmp(&neighbors[i].addr, addr)) {
      n = i;
      break;
    }
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null)) {
      if(neighbors[i].rtmetric > rtmetric) {
	rtmetric = neighbors[i].rtmetric;
	etx = neighbor_etx(&neighbors[i]);
	n = i;
      } else if(neighbors[i].rtmetric == rtmetric) {
	if(neighbor_etx(&neighbors[i]) > etx) {
	  rtmetric = neighbors[i].rtmetric;
	  etx = neighbor_etx(&neighbors[i]);
	  n = i;
	  /*	printf("%d: found worst neighbor %d with rtmetric %d, signal %d\n",
		node_id, neighbors[n].nodeid, rtmetric, signal);*/
	}
      }
    }
  }


  /*  printf("%d: adding neighbor %d with rtmetric %d, signal %d at %d\n",
      node_id, neighbors[n].nodeid, rtmetric, signal, n);*/

  neighbors[n].time = 0;
  rimeaddr_copy(&neighbors[i].addr, addr);
  neighbors[n].rtmetric = nrtmetric;
  for(i = 0; i < NEIGHBOR_NUM_ETXS; ++i) {
    neighbors[n].etxs[i] = netx;
  }
  neighbors[n].etxptr = 0;
}
/*---------------------------------------------------------------------------*/
void
neighbor_remove(rimeaddr_t *addr)
{
  int i;

  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&neighbors[i].addr, addr)) {
      printf("%d: removing %d @ %d\n", rimeaddr_node_addr.u16[0], addr->u16[0], i);
      rimeaddr_copy(&neighbors[i].addr, &rimeaddr_null);
      neighbors[i].rtmetric = RTMETRIC_MAX;
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
  u8_t rtmetric;
  u8_t etx;

  rtmetric = RTMETRIC_MAX;
  lowest = 0;
  found = 0;

  /*  printf("%d: ", node_id);*/
  
  /* Find the lowest rtmetric. */
  for(i = 0; i < MAX_NEIGHBORS; ++i) {
    /*  printf("%d:%d ", neighbors[i].nodeid, neighbors[i].rtmetric);*/
    if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
       rtmetric > neighbors[i].rtmetric) {
      rtmetric = neighbors[i].rtmetric;
      lowest = i;
      found = 1;
    }
  }
  /*  printf("\n");*/

  /* Find the neighbor with lowest etx of the ones that
     have the lowest rtmetric. */
  if(found) {
    etx = 0;
    best = lowest;
    for(i = 0; i < MAX_NEIGHBORS; ++i) {
      if(!rimeaddr_cmp(&neighbors[i].addr, &rimeaddr_null) &&
	 rtmetric == neighbors[i].rtmetric &&
	 neighbor_etx(&neighbors[i]) < etx) {
	etx = neighbor_etx(&neighbors[i]);
	best = i;
      }
    }
    return &neighbors[best];
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
neighbor_set_lifetime(int seconds)
{
  max_time = seconds;
}
/*---------------------------------------------------------------------------*/
int
neighbor_num(void)
{
  return MAX_NEIGHBORS;
}
/*---------------------------------------------------------------------------*/
struct neighbor *
neighbor_get(int num)
{
  return &neighbors[num];
}
/*---------------------------------------------------------------------------*/
/** @} */
