/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
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
 * \file
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "node-id.h"
#include "orchestra.h"
#include "toplogy.h"
#if CONTIKI_TARGET_SKY || CONTIKI_TARGET_Z1
#include "cc2420.h"
#endif
#include <string.h>
#include <stdio.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

struct toplogy_link {
  linkaddr_t node;
  linkaddr_t parent;
};

/* List of ID<->MAC mapping used for different deployments */
static const struct toplogy_link topology[] = {
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x1}},{{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x2}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x1}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x3}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x1}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x4}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x1}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x5}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x2}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x6}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x2}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x7}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x3}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x8}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x3}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x9}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x4}}},
    {{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0xa}},{{0xc1,0x0c,0x00,0x00,0x00,0x00,0x00,0x4}}},
};

#define NODE_COUNT (sizeof(topology)/sizeof(struct toplogy_link))

/*---------------------------------------------------------------------------*/
unsigned
toplogy_orchestra_hash(const void *addr)
{
  int i;
  for(i=0; i<NODE_COUNT; i++) {
    if(linkaddr_cmp(addr, &topology[i].node)) {
      return i;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const linkaddr_t *
toplogy_hardcoded_parent(const linkaddr_t *addr)
{
  int i;
  for(i=0; i<NODE_COUNT; i++) {
    if(linkaddr_cmp(addr, &topology[i].node)) {
      return &topology[i].parent;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void *
toplogy_probing_func(void *vdag)
{
  /* Returns the next probing target. This implementation probes the current
   * preferred parent if we have not transmitted to it for RPL_PROBING_EXPIRATION_TIME.
   * Otherwise, it picks at random between:
   * (1) selecting the best parent (lowest rank) not updated for RPL_PROBING_EXPIRATION_TIME
   * (2) selecting the least recently updated parent
   */

  rpl_dag_t *dag = (rpl_dag_t *)vdag;
  rpl_parent_t *p;
  rpl_parent_t *probing_target = NULL;
  rpl_rank_t probing_target_rank = INFINITE_RANK;
  clock_time_t probing_target_age = 0;
  clock_time_t clock_now = clock_time();
  rpl_parent_t *hardcoded_parent = rpl_get_parent((uip_lladdr_t *)toplogy_hardcoded_parent(&linkaddr_node_addr));

  if(dag == NULL ||
      dag->instance == NULL ||
      dag->preferred_parent == NULL) {
    return NULL;
  }

  /* Our preferred parent needs probing */
  if(clock_now - dag->preferred_parent->last_tx_time > RPL_PROBING_EXPIRATION_TIME) {
    return dag->preferred_parent;
  }

  /* Our hardcoded preferred parent needs probing */
  if(hardcoded_parent != NULL
      && clock_now - hardcoded_parent->last_tx_time > RPL_PROBING_EXPIRATION_TIME) {
    return hardcoded_parent;
  }

  /* With 50% probability: probe best parent not updated for RPL_PROBING_EXPIRATION_TIME */
  if(random_rand() % 2 == 0) {
    p = nbr_table_head(rpl_parents);
    while(p != NULL) {
      if(p->dag == dag && (clock_now - p->last_tx_time > RPL_PROBING_EXPIRATION_TIME)) {
        /* p is in our dag and needs probing */
        rpl_rank_t p_rank = dag->instance->of->calculate_rank(p, 0);
        if(probing_target == NULL
            || p_rank < probing_target_rank) {
          probing_target = p;
          probing_target_rank = p_rank;
        }
      }
      p = nbr_table_next(rpl_parents, p);
    }
  }

  /* If we still do not have a probing target: pick the least recently updated parent */
  if(probing_target == NULL) {
    p = nbr_table_head(rpl_parents);
    while(p != NULL) {
      if(p->dag == dag) {
        if(probing_target == NULL
            || clock_now - p->last_tx_time > probing_target_age) {
          probing_target = p;
          probing_target_age = clock_now - p->last_tx_time;
        }
      }
      p = nbr_table_next(rpl_parents, p);
    }
  }

  return probing_target;
}
