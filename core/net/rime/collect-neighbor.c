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
 * $Id: collect-neighbor.c,v 1.10 2011/01/10 15:08:52 adamdunkels Exp $
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

#define MAX_AGE                      180
#define MAX_LE_AGE                   10
#define PERIODIC_INTERVAL            CLOCK_SECOND * 60

#define EXPECTED_CONGESTION_DURATION CLOCK_SECOND * 240
#define CONGESTION_PENALTY           8 * COLLECT_LINK_ESTIMATE_UNIT

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
  struct collect_neighbor_list *neighbor_list;
  struct collect_neighbor *n;

  neighbor_list = ptr;

  /* Go through all collect_neighbors and increase their age. */
  for(n = list_head(neighbor_list->list); n != NULL; n = list_item_next(n)) {
    n->age++;
    n->le_age++;
  }
  for(n = list_head(neighbor_list->list); n != NULL; n = list_item_next(n)) {
    if(n->le_age == MAX_LE_AGE) {
      collect_link_estimate_new(&n->le);
      n->le_age = 0;
    }
    if(n->age == MAX_AGE) {
      memb_free(&collect_neighbors_mem, n);
      list_remove(neighbor_list->list, n);
      n = list_head(neighbor_list->list);
    }
  }
  ctimer_set(&neighbor_list->periodic, PERIODIC_INTERVAL,
             periodic, neighbor_list);
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_init(void)
{
  static uint8_t initialized = 0;
  if(initialized == 0) {
    initialized = 1;
    memb_init(&collect_neighbors_mem);
  }
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_list_new(struct collect_neighbor_list *neighbors_list)
{
  LIST_STRUCT_INIT(neighbors_list, list);
  list_init(neighbors_list->list);
  ctimer_set(&neighbors_list->periodic, CLOCK_SECOND, periodic, neighbors_list);
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_list_find(struct collect_neighbor_list *neighbors_list,
                           const rimeaddr_t *addr)
{
  struct collect_neighbor *n;
  for(n = list_head(neighbors_list->list); n != NULL; n = list_item_next(n)) {
    if(rimeaddr_cmp(&n->addr, addr)) {
      return n;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_list_add(struct collect_neighbor_list *neighbors_list,
                          const rimeaddr_t *addr, uint16_t nrtmetric)
{
  struct collect_neighbor *n;

  if(addr == NULL) {
    PRINTF("collect_neighbor_list_add: attempt to add NULL addr\n");
    return 0;
  }

  PRINTF("collect_neighbor_add: adding %d.%d\n", addr->u8[0], addr->u8[1]);

  /* Check if the collect_neighbor is already on the list. */
  for(n = list_head(neighbors_list->list); n != NULL; n = list_item_next(n)) {
    if(rimeaddr_cmp(&n->addr, addr)) {
      PRINTF("collect_neighbor_add: already on list %d.%d\n",
             addr->u8[0], addr->u8[1]);
      break;
    }
  }

  /* If the collect_neighbor was not on the list, we try to allocate memory
     for it. */
  if(n == NULL) {
    PRINTF("collect_neighbor_add: not on list, allocating %d.%d\n",
           addr->u8[0], addr->u8[1]);
    n = memb_alloc(&collect_neighbors_mem);
    if(n != NULL) {
      list_add(neighbors_list->list, n);
    }
  }

  /* If we could not allocate memory, we try to recycle an old
     neighbor. XXX Should also look for the one with the worst
     rtmetric (not link esimate). XXX Also make sure that we don't
     replace a neighbor with a neighbor that has a worse metric. */
  if(n == NULL) {
    uint16_t worst_rtmetric;
    struct collect_neighbor *worst_neighbor;

    /* Find the neighbor that has the highest rtmetric. This is the
       neighbor that we are least likely to be using in the
       future. But we also need to make sure that the neighbor we are
       currently adding is not worst than the one we would be
       replacing. If so, we don't put the new neighbor on the list. */
    worst_rtmetric = 0;
    worst_neighbor = NULL;

    for(n = list_head(neighbors_list->list);
        n != NULL; n = list_item_next(n)) {
      if(n->rtmetric > worst_rtmetric) {
        worst_neighbor = n;
        worst_rtmetric = n->rtmetric;
      }
    }

    /* Only add this new neighbor if its rtmetric is lower than the
       one it would replace. */
    if(nrtmetric < worst_rtmetric) {
      n = worst_neighbor;
    }
    if(n != NULL) {
      PRINTF("collect_neighbor_add: not on list, not allocated, recycling %d.%d\n",
             n->addr.u8[0], n->addr.u8[1]);
    }
  }

  if(n != NULL) {
    n->age = 0;
    rimeaddr_copy(&n->addr, addr);
    n->rtmetric = nrtmetric;
    collect_link_estimate_new(&n->le);
    n->le_age = 0;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
list_t
collect_neighbor_list(struct collect_neighbor_list *neighbors_list)
{
  return neighbors_list->list;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_list_remove(struct collect_neighbor_list *neighbors_list,
                             const rimeaddr_t *addr)
{
  struct collect_neighbor *n = collect_neighbor_list_find(neighbors_list, addr);

  if(n != NULL) {
    list_remove(neighbors_list->list, n);
    memb_free(&collect_neighbors_mem, n);
  }
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_list_best(struct collect_neighbor_list *neighbors_list)
{
  int found;
  struct collect_neighbor *n, *best;
  uint16_t rtmetric;

  rtmetric = RTMETRIC_MAX;
  best = NULL;
  found = 0;

  /*  PRINTF("%d: ", node_id);*/
  PRINTF("collect_neighbor_best: ");

  /* Find the neighbor with the lowest rtmetric + linkt estimate. */
  for(n = list_head(neighbors_list->list); n != NULL; n = list_item_next(n)) {
    PRINTF("%d.%d %d+%d=%d, ",
           n->addr.u8[0], n->addr.u8[1],
           n->rtmetric, collect_neighbor_link_estimate(n),
           collect_neighbor_rtmetric(n));
    if(collect_neighbor_rtmetric_link_estimate(n) < rtmetric) {
      rtmetric = collect_neighbor_rtmetric_link_estimate(n);
      best = n;
    }
  }
  PRINTF("\n");

  return best;
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_list_num(struct collect_neighbor_list *neighbors_list)
{
  PRINTF("collect_neighbor_num %d\n", list_length(neighbors_list->list));
  return list_length(neighbors_list->list);
}
/*---------------------------------------------------------------------------*/
struct collect_neighbor *
collect_neighbor_list_get(struct collect_neighbor_list *neighbors_list, int num)
{
  int i;
  struct collect_neighbor *n;

  PRINTF("collect_neighbor_get %d\n", num);

  i = 0;
  for(n = list_head(neighbors_list->list); n != NULL; n = list_item_next(n)) {
    if(i == num) {
      PRINTF("collect_neighbor_get found %d.%d\n",
             n->addr.u8[0], n->addr.u8[1]);
      return n;
    }
    i++;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_list_purge(struct collect_neighbor_list *neighbors_list)
{
  while(list_head(neighbors_list->list) != NULL) {
    memb_free(&collect_neighbors_mem, list_pop(neighbors_list->list));
  }
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_update_rtmetric(struct collect_neighbor *n, uint16_t rtmetric)
{
  if(n != NULL) {
    PRINTF("%d.%d: collect_neighbor_update %d.%d rtmetric %d\n",
           rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
           n->addr.u8[0], n->addr.u8[1], rtmetric);
    n->rtmetric = rtmetric;
    n->age = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_tx_fail(struct collect_neighbor *n, uint16_t num_tx)
{
  collect_link_estimate_update_tx_fail(&n->le, num_tx);
  n->le_age = 0;
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_tx(struct collect_neighbor *n, uint16_t num_tx)
{
  collect_link_estimate_update_tx(&n->le, num_tx);
  n->le_age = 0;
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_rx(struct collect_neighbor *n)
{
  collect_link_estimate_update_rx(&n->le);
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_neighbor_link_estimate(struct collect_neighbor *n)
{
  if(collect_neighbor_is_congested(n)) {
    /*    printf("Congested %d.%d, sould return %d, returning %d\n",
           n->addr.u8[0], n->addr.u8[1],
           collect_link_estimate(&n->le),
           collect_link_estimate(&n->le) + CONGESTION_PENALTY);*/
    return collect_link_estimate(&n->le) + CONGESTION_PENALTY;
  } else {
    return collect_link_estimate(&n->le);
  }
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_neighbor_rtmetric_link_estimate(struct collect_neighbor *n)
{
  return n->rtmetric + collect_link_estimate(&n->le);
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_neighbor_rtmetric(struct collect_neighbor *n)
{
  return n->rtmetric;
}
/*---------------------------------------------------------------------------*/
void
collect_neighbor_set_congested(struct collect_neighbor *n)
{
  timer_set(&n->congested_timer, EXPECTED_CONGESTION_DURATION);
}
/*---------------------------------------------------------------------------*/
int
collect_neighbor_is_congested(struct collect_neighbor *n)
{
  if(timer_expired(&n->congested_timer)) {
    return 0;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
