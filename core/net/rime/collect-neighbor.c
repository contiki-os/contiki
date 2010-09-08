/**
 * \addtogroup rimecollect_neighbor
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
 * $Id: collect-neighbor.c,v 1.4 2010/09/08 19:21:45 adamdunkels Exp $
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
#include "lib/memb.h"
#include "lib/list.h"

#include "net/rime/collect-neighbor.h"
#include "net/rime/collect.h"

#ifdef COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#define MAX_COLLECT_NEIGHBORS COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#else /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */
#define MAX_COLLECT_NEIGHBORS 8
#endif /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */

#define RTMETRIC_MAX COLLECT_MAX_DEPTH

MEMB(collect_neighbors_mem, struct collect_neighbor, MAX_COLLECT_NEIGHBORS);
LIST(collect_neighbors_list);

/*static struct collect_neighbor collect_neighbors[MAX_COLLECT_NEIGHBORS];*/

static struct ctimer t;

static int max_time = 2400;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
static void
periodic(void *ptr)
{  
  struct collect_neighbor *n, *next;

  /* Go through all collect_neighbors and remove old ones. */
  for(n = list_head(collect_neighbors_list); n != NULL; n = next) {
    next = NULL;
    /*  for(i = 0; i < MAX_COLLECT_NEIGHBORS; ++i) {*/
    if(!rimeaddr_cmp(&n->addr, &rimeaddr_null) &&
       n->time < max_time) {
      n->time++;
      if(n->time == max_time) {
	n->rtmetric = RTMETRIC_MAX;
	PRINTF("%d.%d: removing old collect_neighbor %d.%d\n",
	       rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	       n->addr.u8[0], n->addr.u8[1]);
	rimeaddr_copy(&n->addr, &rimeaddr_null);
	next = list_item_next(n);
	list_remove(collect_neighbors_list, n);
	memb_free(&collect_neighbors_mem, n);
      }
    }
    if(next == NULL) {
      next = list_item_next(n);
    }
  }
  /*  PRINTF("collect_neighbor periodic\n");*/
  ctimer_set(&t, CLOCK_SECOND, periodic, NULL);
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_init(void)
{
  static uint8_t initialized = 0;
  if(initialized == 0) {
    initialized = 1;
    memb_init(&collect_neighbors_mem);
    list_init(collect_neighbors_list);
    ctimer_set(&t, CLOCK_SECOND, periodic, NULL);
  }
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_find(const rimeaddr_t *addr)
{
  struct collect_neighbor *n;
  for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
    if(rimeaddr_cmp(&n->addr, addr)) {
      return n;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_update_rtmetric(struct collect_neighbor *n, uint8_t rtmetric)
{
  if(n != NULL) {
    PRINTF("%d.%d: collect_neighbor_update %d.%d rtmetric %d\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           n->addr.u8[0], n->addr.u8[1], rtmetric);
    n->rtmetric = rtmetric;
    n->time = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_timedout(struct collect_neighbor *n, uint8_t num_tx)
{
  collect_link_estimate_update_tx_fail(&n->le, num_tx);
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_tx(struct collect_neighbor *n, uint8_t num_tx)
{
  collect_link_estimate_update_tx(&n->le, num_tx);
  n->time = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_rx(struct collect_neighbor *n)
{
  collect_link_estimate_update_rx(&n->le);
  n->time = 0;
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_link_estimate(struct collect_neighbor *n)
{
  return collect_link_estimate(&n->le);
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_add(const rimeaddr_t *addr, uint8_t nrtmetric)
{
  uint16_t rtmetric;
  uint16_t le;
  struct collect_neighbor *n, *max;

  PRINTF("collect_neighbor_add: adding %d.%d\n", addr->u8[0], addr->u8[1]);
  
  /* Check if the collect_neighbor is already on the list. */
  for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
    if(rimeaddr_cmp(&n->addr, &rimeaddr_null) ||
       rimeaddr_cmp(&n->addr, addr)) {
      PRINTF("collect_neighbor_add: already on list %d.%d\n", addr->u8[0], addr->u8[1]);
      break;
    }
  }

  /* If the collect_neighbor was not on the list, we try to allocate memory
     for it. */
  if(n == NULL) {
    PRINTF("collect_neighbor_add: not on list, allocating %d.%d\n", addr->u8[0], addr->u8[1]);
    n = memb_alloc(&collect_neighbors_mem);
    if(n != NULL) {
      list_add(collect_neighbors_list, n);
    }
  }
  
  /* If we could not allocate memory, we try to recycle an old
     collect_neighbor */
  if(n == NULL) {
    PRINTF("collect_neighbor_add: not on list, not allocated, recycling %d.%d\n", addr->u8[0], addr->u8[1]);
   /* Find the first unused entry or the used entry with the highest
     rtmetric and highest link estimate. */
    rtmetric = 0;
    le = 0;
    max = NULL;

    for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
      if(!rimeaddr_cmp(&n->addr, &rimeaddr_null)) {
	if(n->rtmetric > rtmetric) {
	  rtmetric = n->rtmetric;
	  le = collect_neighbor_link_estimate(n);
	  max = n;
	} else if(n->rtmetric == rtmetric) {
	  if(collect_neighbor_link_estimate(n) > le) {
	    rtmetric = n->rtmetric;
	    le = collect_neighbor_link_estimate(n);
	    max = n;
	    /*	PRINTF("%d: found worst collect_neighbor %d with rtmetric %d, signal %d\n",
		node_id, collect_neighbors[n].nodeid, rtmetric, signal);*/
	  }
	}
      }
    }
    n = max;
  }


  /*  PRINTF("%d: adding collect_neighbor %d with rtmetric %d, signal %d at %d\n",
      node_id, collect_neighbors[n].nodeid, rtmetric, signal, n);*/
  if(n != NULL) {
    n->time = 0;
    rimeaddr_copy(&n->addr, addr);
    n->rtmetric = nrtmetric;
    collect_link_estimate_new(&n->le);
  }
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_remove(const rimeaddr_t *addr)
{
  struct collect_neighbor *n;

  for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
    if(rimeaddr_cmp(&n->addr, addr)) {
      PRINTF("%d.%d: removing %d.%d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     addr->u8[0], addr->u8[1]);
      rimeaddr_copy(&n->addr, &rimeaddr_null);
      n->rtmetric = RTMETRIC_MAX;
      list_remove(collect_neighbors_list, n);
      memb_free(&collect_neighbors_mem, n);
      return;
    }
  }
  /*  int i;

  for(i = 0; i < MAX_COLLECT_NEIGHBORS; ++i) {
    if(rimeaddr_cmp(&collect_neighbors[i].addr, addr)) {
      PRINTF("%d: removing %d @ %d\n", rimeaddr_node_addr.u16[0], addr->u16[0], i);
      rimeaddr_copy(&collect_neighbors[i].addr, &rimeaddr_null);
      collect_neighbors[i].rtmetric = RTMETRIC_MAX;
      return;
    }
    }*/
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_best(void)
{
  int found;
  /*  int lowest, best;*/
  struct collect_neighbor *n, *best;
  uint16_t rtmetric;

  rtmetric = RTMETRIC_MAX;
  best = NULL;
  found = 0;

  /*  PRINTF("%d: ", node_id);*/
  
  /* Find the lowest rtmetric. */
  for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
    PRINTF("collect_neighbor_best: checking %d.%d with rtmetric %d + %d (%d)\n",
           n->addr.u8[0], n->addr.u8[1],
           n->rtmetric, collect_neighbor_link_estimate(n),
           collect_neighbor_rtmetric(n));
    if(!rimeaddr_cmp(&n->addr, &rimeaddr_null) &&
       rtmetric > collect_neighbor_rtmetric(n)) {
      rtmetric = collect_neighbor_rtmetric(n);
      best = n;
    }
  }

  return best;
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_rtmetric(struct collect_neighbor *n)
{
  return n->rtmetric + collect_link_estimate(&n->le);
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_set_lifetime(int seconds)
{
  max_time = seconds;
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_num(void)
{
  PRINTF("collect_neighbor_num %d\n", list_length(collect_neighbors_list));
  return list_length(collect_neighbors_list);
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_get(int num)
{
  int i;
  struct collect_neighbor *n;

  PRINTF("collect_neighbor_get %d\n", num);
  
  i = 0;
  for(n = list_head(collect_neighbors_list); n != NULL; n = list_item_next(n)) {
    if(i == num) {
      PRINTF("collect_neighbor_get found %d.%d\n", n->addr.u8[0], n->addr.u8[1]);
      return n;
    }
    i++;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_purge(void)
{
  memb_init(&collect_neighbors_mem);
  list_init(collect_neighbors_list);
}
/*---------------------------------------------------------------------------*/
/** @} */
