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
 * $Id: rpl-of-etx.c,v 1.7 2010/10/28 20:39:06 joxe Exp $
 */
/**
 * \file
 *         An implementation of RPL's objective function 1 (ETX).
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl.h"
#include "net/neighbor-info.h"

#define DEBUG DEBUG_ANNOTATE
#include "net/uip-debug.h"

static void reset(void *);
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

#define LINK_ETX_MIN			1
#define LINK_ETX_MAX			10
#define LINK_ETX_GUESS			3
#define PATH_ETX_MIN			1
#define PATH_ETX_MAX			200
#define PARENT_SWITCH_ETX_THRESHOLD	0.5

typedef uint16_t etx_t;

static etx_t min_path_etx = INFINITE_RANK;

static void
reset(void *dag)
{
  min_path_etx = INFINITE_RANK;
}

static void
parent_state_callback(rpl_parent_t *parent, int known, int etx)
{
  rpl_dag_t *dag;

  dag = (rpl_dag_t *)parent->dag;

  if(!known) {
    if(RPL_PARENT_COUNT(dag) == 1) {
      /* Our last parent has disappeared, set the path ETX to INFINITE_RANK. */
      min_path_etx = INFINITE_RANK;
    }
  }
}

static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
  rpl_dag_t *dag;
  rpl_rank_t new_rank;
  rpl_rank_t rank_increase;

  if(p == NULL) {
    if(base_rank == 0) {
      return INFINITE_RANK;
    }
    rank_increase = LINK_ETX_GUESS * DEFAULT_MIN_HOPRANKINC;
  } else {
    dag = (rpl_dag_t *)p->dag;
    if(p->local_confidence == 0) {
      p->local_confidence = LINK_ETX_GUESS * ETX_DIVISOR;
    }
    rank_increase = (p->local_confidence * dag->min_hoprankinc) / ETX_DIVISOR;
    if(base_rank == 0) {
      base_rank = p->rank;
    }
  }

  PRINTF("RPL: OF1 calculate rank, base rank = %u, rank_increase = %u\n",
	 (unsigned)base_rank, rank_increase);

  if(base_rank < min_path_etx) {
    min_path_etx = base_rank;
    PRINTF("RPL: min_path_etx updated to %u\n", min_path_etx);
  }

  if(INFINITE_RANK - base_rank < rank_increase) {
    /* Reached the maximum rank. */
    new_rank = INFINITE_RANK;
  } else {
   /* Calculate the rank based on the new rank information from DIO or
      stored otherwise. */
    new_rank = base_rank + rank_increase;
  }

  PRINTF("RPL: Path ETX %u\n", (unsigned)new_rank);

  return new_rank;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_dag_t *dag;
  rpl_rank_t p1_rank;
  rpl_rank_t p2_rank;

  dag = (rpl_dag_t *)p1->dag; /* Both parents must be in the same DAG. */

  p1_rank = DAG_RANK(calculate_rank(p1, 0), dag);
  p2_rank = DAG_RANK(calculate_rank(p2, 0), dag);

  /* Maintain stability of the preferred parent in case of similar ranks. */
  if(p1_rank == p2_rank) {
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
