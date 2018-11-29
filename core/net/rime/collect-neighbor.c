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
 */

/**
 * \file
 *         Radio neighborhood management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rimeneighbor
 * @{
 */

#include <limits.h>
#include <stdio.h>

#include "contiki.h"
#include "net/rime/collect-neighbor.h"
#include "net/rime/collect.h"

#ifdef COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#define MAX_COLLECT_NEIGHBORS COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#else /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */
#define MAX_COLLECT_NEIGHBORS 8
#endif /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */

#define RTMETRIC_MAX COLLECT_MAX_DEPTH

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
static struct ctimer collect_periodic_timer;
/*---------------------------------------------------------------------------*/
NBR_TABLE_GLOBAL(collect_nbr_t, collect_nbr_table);
/*---------------------------------------------------------------------------*/
static void
periodic(void *ptr)
{
  collect_nbr_t *n;

  /* Go through all collect_neighbors and increase their age. */
  for(n = nbr_table_head(collect_nbr_table);
      n != NULL;
      n = nbr_table_next(collect_nbr_table, n)) {
    n->age++;
    n->le_age++;
  }
  
  n = nbr_table_head(collect_nbr_table);
  while(n != NULL) {
    if(n->le_age == MAX_LE_AGE) {
      collect_link_estimate_new(&n->le);
      n->le_age = 0;
    }
    if(n->age == MAX_AGE) {
      collect_nbr_rm(n);
    }
    n = nbr_table_next(collect_nbr_table, n);
  }

  ctimer_set(&collect_periodic_timer, PERIODIC_INTERVAL,
             periodic, NULL);
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_init(void)
{
  nbr_table_register(collect_nbr_table, (nbr_table_callback *)collect_nbr_rm);
  ctimer_set(&collect_periodic_timer, CLOCK_SECOND, periodic, NULL);
}
/*---------------------------------------------------------------------------*/
collect_nbr_t *
collect_nbr_find(const linkaddr_t *addr)
{
  return nbr_table_get_from_lladdr(collect_nbr_table, addr);
}
/*---------------------------------------------------------------------------*/
int
collect_nbr_add(const linkaddr_t *addr, uint16_t nrtmetric)
{
  collect_nbr_t *n = NULL;

  if(addr == NULL) {
    PRINTF("collect_nbr_add: attempt to add NULL addr\n");
    return 0;
  }

  /* Check if the neighbor is already in the table */
  if(collect_nbr_find(addr) == NULL) {
    /* The neighbor is not already in the table.
     * Before we add a new neighbor we check if the table is full. If that is 
     * true we check if the routing metric of the new neighbor is better than 
     * the worst neighbor. If this occurs, we replace the worst neighbor with 
     * the new one. Otherwise, we do not add this new neighbor.
     *
     * This mechanism avoids using the default neighbor policy.
     */
    if(collect_nbr_num() >= MAX_COLLECT_NEIGHBORS) {
      collect_nbr_t *worst;
      worst = collect_nbr_worst();
      if(worst->rtmetric > nrtmetric) {
        /* The new neighbor is better than the worst one. We remove the worst
         * neighbor to make space for the new one.
         */
        collect_nbr_rm(worst);
      } else {
        /* The worst neighbor is better or equal to the new one in terms of 
         * the routing metric. We do not replace the neighbor and neither add 
         * the new one.
         */
        return 0;
      }
    }

    /* Now, we should have space to add the new neighbor */
    n = nbr_table_add_lladdr(
      collect_nbr_table,
      addr,
      NBR_TABLE_REASON_UNDEFINED,
      NULL);

    if(n != NULL) {
      n->age = 0;
      n->rtmetric = nrtmetric;
      collect_link_estimate_new(&n->le);
      n->le_age = 0;
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_rm(collect_nbr_t *n)
{
  nbr_table_remove(collect_nbr_table, n);
}
/*---------------------------------------------------------------------------*/
collect_nbr_t *
collect_nbr_best(void)
{
  collect_nbr_t *n;
  collect_nbr_t *best_nbr = NULL;
  uint16_t best_rtmetric = RTMETRIC_MAX;

  for(n = nbr_table_head(collect_nbr_table);
      n != NULL;
      n = nbr_table_next(collect_nbr_table, n)) {
    if(n->rtmetric < best_rtmetric) {
      best_nbr = n;
      best_rtmetric = n->rtmetric;
    }
  }

  return best_nbr;
}
/*---------------------------------------------------------------------------*/
collect_nbr_t *
collect_nbr_worst(void)
{
  collect_nbr_t *n;
  collect_nbr_t *worst_nbr = NULL;
  uint16_t worst_rtmetric = 0;

  for(n = nbr_table_head(collect_nbr_table);
      n != NULL;
      n = nbr_table_next(collect_nbr_table, n)) {
    if(n->rtmetric > worst_rtmetric) {
      worst_nbr = n;
      worst_rtmetric = n->rtmetric;
    }
  }
  
  return worst_nbr;
}
/*---------------------------------------------------------------------------*/
int
collect_nbr_num(void)
{
  collect_nbr_t *n;
  int num;

  num = 0;
  for(n = nbr_table_head(collect_nbr_table);
      n != NULL;
      n = nbr_table_next(collect_nbr_table, n)) {
    num++;
  }
  PRINTF("collect_nbr_num %d\n", num);
  return num;
}
/*---------------------------------------------------------------------------*/
collect_nbr_t *
collect_nbr_get(int num)
{
  collect_nbr_t *n;
  int i;

  i = 0;
  PRINTF("collect_nbr_get %d\n", num);
  for(n = nbr_table_head(collect_nbr_table);
      n != NULL;
      n = nbr_table_next(collect_nbr_table, n)) {
    if(i == num) {
#if DEBUG
      linkaddr_t *addr;
      addr = nbr_table_get_lladdr(collect_nbr_table, n);
      PRINTF("collect_nbr_get found %d.%d\n",
        addr->u8[0],
        addr->u8[1]);
#endif
      return n;
    }
    i++;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_purge(void)
{
  /* TO DO 
   * This function is only called from collect_purge, but collect_purge is 
   * not called from anywhere. 
   */
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_update_rtmetric(collect_nbr_t *n, uint16_t rtmetric)
{
  if(n != NULL) {
#if DEBUG
  	linkaddr_t *addr;
    addr = nbr_table_get_lladdr(collect_nbr_table, n);
    PRINTF("%d.%d: collect_nbr_update_rtmetric %d.%d rtmetric %d\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           addr->u8[0], addr->u8[1], rtmetric);
#endif
    n->rtmetric = rtmetric;
    n->age = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_tx_fail(collect_nbr_t *n, uint16_t num_tx)
{
  if(n == NULL) {
    return;
  }
  collect_link_estimate_update_tx_fail(&n->le, num_tx);
  n->le_age = 0;
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_tx(collect_nbr_t *n, uint16_t num_tx)
{
  if(n == NULL) {
    return;
  }
  collect_link_estimate_update_tx(&n->le, num_tx);
  n->le_age = 0;
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_rx(collect_nbr_t *n)
{
  if(n == NULL) {
    return;
  }
  collect_link_estimate_update_rx(&n->le);
  n->age = 0;
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_nbr_link_estimate(collect_nbr_t *n)
{
  if(n == NULL) {
    return 0;
  }
  if(collect_nbr_is_congested(n)) {
    /*
    linkaddr_t *addr;
    addr = nbr_table_get_lladdr(collect_nbr_table, n);
    printf("Congested %d.%d, sould return %d, returning %d\n",
           addr->u8[0], addr->u8[1],
           collect_link_estimate(&n->le),
           collect_link_estimate(&n->le) + CONGESTION_PENALTY);
    */
    return collect_link_estimate(&n->le) + CONGESTION_PENALTY;
  } else {
    return collect_link_estimate(&n->le);
  }
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_nbr_rtmetric_link_estimate(collect_nbr_t *n)
{
  if(n == NULL) {
    return 0;
  }
  return n->rtmetric + collect_link_estimate(&n->le);
}
/*---------------------------------------------------------------------------*/
uint16_t
collect_nbr_rtmetric(const collect_nbr_t *n)
{
  if(n == NULL) {
    return 0;
  }
  return n->rtmetric;
}
/*---------------------------------------------------------------------------*/
void
collect_nbr_set_congested(collect_nbr_t *n)
{
  if(n == NULL) {
    return;
  }
  timer_set(&n->congested_timer, EXPECTED_CONGESTION_DURATION);
}
/*---------------------------------------------------------------------------*/
int
collect_nbr_is_congested(collect_nbr_t *n)
{
  if(n == NULL) {
    return 0;
  }

  if(timer_expired(&n->congested_timer)) {
    return 0;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
