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
 *
 */
/**
 * \file
 *         The minrank-hysteresis objective function (OCP 1).
 *
 *         This implementation uses the estimated number of 
 *         transmissions (ETX) as the additive routing metric.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl-private.h"
#include "net/neighbor-info.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

static void reset(rpl_dag_t *);
static void parent_state_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);

rpl_of_t rpl_of_etx = {
  reset,
  parent_state_callback,
  best_parent,
  calculate_rank,
  1
};

/* Reject parents that have a higher link metric than the following. */
#define MAX_LINK_METRIC			10

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST			100

/* An initial guess of the link metric. */
#define INITIAL_LINK_METRIC		3

/*
 * The rank must differ more than 1/PARENT_SWITCH_THRESHOLD_DIV in order
 * to switch preferred parent.
 */
#define PARENT_SWITCH_THRESHOLD_DIV	2

#define MAX_DIFFERENCE(dag)				\
	((dag)->min_hoprankinc / PARENT_SWITCH_THRESHOLD_DIV)

static rpl_rank_t min_path_cost = INFINITE_RANK;

static void
reset(rpl_dag_t *dag)
{
  min_path_cost = INFINITE_RANK;
}

static void
parent_state_callback(rpl_parent_t *parent, int known, int etx)
{
  if(!known) {
    if(RPL_PARENT_COUNT(parent->dag) == 1) {
      /* Our last parent has disappeared, set the path ETX to INFINITE_RANK. */
      min_path_cost = INFINITE_RANK;
    }
  }
}

static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
  rpl_rank_t new_rank;
  rpl_rank_t rank_increase;

  if(p == NULL) {
    if(base_rank == 0) {
      return INFINITE_RANK;
    }
    rank_increase = INITIAL_LINK_METRIC * DEFAULT_MIN_HOPRANKINC;
  } else {
    if(p->etx == 0) {
      p->etx = INITIAL_LINK_METRIC * ETX_DIVISOR;
    }
    rank_increase = (p->etx * p->dag->min_hoprankinc) / ETX_DIVISOR;
    if(base_rank == 0) {
      base_rank = p->rank;
    }
  }

  PRINTF("RPL: MRHOF calculate rank, base rank = %u, rank_increase = %u\n",
	 (unsigned)base_rank, rank_increase);

  if(base_rank < min_path_cost) {
    min_path_cost = base_rank;
    PRINTF("RPL: min_path_cost updated to %u\n", min_path_cost);
  }

  if(INFINITE_RANK - base_rank < rank_increase) {
    /* Reached the maximum rank. */
    new_rank = INFINITE_RANK;
  } else {
   /* Calculate the rank based on the new rank information from DIO or
      stored otherwise. */
    new_rank = base_rank + rank_increase;
  }

  PRINTF("RPL: Path cost %u\n", (unsigned)new_rank);

  return new_rank;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_dag_t *dag;
  rpl_rank_t p1_rank;
  rpl_rank_t p2_rank;

  dag = (rpl_dag_t *)p1->dag; /* Both parents must be in the same DAG. */

  p1_rank = calculate_rank(p1, 0);
  p2_rank = calculate_rank(p2, 0);

  /* Maintain stability of the preferred parent in case of similar ranks. */
  if(p1_rank < p2_rank + MAX_DIFFERENCE(dag) &&
     p1_rank > p2_rank - MAX_DIFFERENCE(dag)) {
    PRINTF("RPL: MRHOF hysteresis: %u <= %u <= %u\n",
           p2_rank - MAX_DIFFERENCE(dag),
           p1_rank,
           p2_rank + MAX_DIFFERENCE(dag));
    if(p1 == dag->preferred_parent) {
      return p1;
    } else if(p2 == dag->preferred_parent) {
      return p2;
    }
  }

  if(p1_rank < p2_rank) {
    return p1;
  }

  return p2;
}
