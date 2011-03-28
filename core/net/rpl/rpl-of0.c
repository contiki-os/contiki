/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */
/**
 * \file
 *         An implementation of RPL's objective function 0.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl-private.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "net/neighbor-info.h"

static void reset(rpl_dag_t *);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);
static void update_metric_container(rpl_dag_t *);

rpl_of_t rpl_of0 = {
  reset,
  NULL,
  best_parent,
  calculate_rank,
  update_metric_container,
  0
};

#define DEFAULT_RANK_INCREMENT  DEFAULT_MIN_HOPRANKINC

#define MIN_DIFFERENCE (NEIGHBOR_INFO_ETX_DIVISOR + NEIGHBOR_INFO_ETX_DIVISOR / 2)

static void
reset(rpl_dag_t *dag)
{
  PRINTF("RPL: Resetting OF0\n");
}

static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
  rpl_rank_t increment;
  if(base_rank == 0) {
    if(p == NULL) {
      return INFINITE_RANK;
    }
    base_rank = p->rank;
  }

  increment = p != NULL ? p->dag->min_hoprankinc : DEFAULT_RANK_INCREMENT;

  if((rpl_rank_t)(base_rank + increment) < base_rank) {
    PRINTF("RPL: OF0 rank %d incremented to infinite rank due to wrapping\n",
        base_rank);
    return INFINITE_RANK;
  }
  return base_rank + increment;

}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_rank_t r1, r2;
  rpl_dag_t *dag;
  
  PRINTF("RPL: Comparing parent ");
  PRINT6ADDR(&p1->addr);
  PRINTF(" (confidence %d, rank %d) with parent ",
        p1->link_metric, p1->rank);
  PRINT6ADDR(&p2->addr);
  PRINTF(" (confidence %d, rank %d)\n",
        p2->link_metric, p2->rank);


  r1 = DAG_RANK(p1->rank, (rpl_dag_t *)p1->dag) * NEIGHBOR_INFO_ETX_DIVISOR +
    p1->link_metric;
  r2 = DAG_RANK(p2->rank, (rpl_dag_t *)p1->dag) * NEIGHBOR_INFO_ETX_DIVISOR +
    p2->link_metric;
  /* Compare two parents by looking both and their rank and at the ETX
     for that parent. We choose the parent that has the most
     favourable combination. */

  dag = (rpl_dag_t *)p1->dag; /* Both parents must be in the same DAG. */
  if(r1 < r2 + MIN_DIFFERENCE &&
     r1 > r2 - MIN_DIFFERENCE) {
    return dag->preferred_parent;
  } else if(r1 < r2) {
    return p1;
  } else {
    return p2;
  }
}

static void
update_metric_container(rpl_dag_t *dag)
{
  dag->mc.type = RPL_DAG_MC_NONE;
}
