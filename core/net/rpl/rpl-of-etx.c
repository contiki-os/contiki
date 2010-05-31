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
 * $Id: rpl-of-etx.c,v 1.2 2010/05/31 14:22:00 nvt-se Exp $
 */
/**
 * \file
 *         An implementation of RPL's objective function 1 (ETX).
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/rpl/rpl.h"

#define DEBUG DEBUG_ANNOTATE
#include "net/uip-debug.h"

static void reset(void *);
static void parent_state_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_rank_t increment_rank(rpl_rank_t, rpl_parent_t *);

rpl_of_t rpl_of_etx = {
  reset,
  parent_state_callback,
  best_parent,
  increment_rank,
  1
};

#define LINK_ETX_MIN			1
#define LINK_ETX_MAX			10
#define PATH_ETX_MIN			1
#define PATH_ETX_MAX			200
#define PARENT_SWITCH_ETX_THRESHOLD	0.5
#define INFINITY			255

typedef unsigned char etx_t;

static etx_t min_path_etx = INFINITY;

static void
reset(void *dag)
{
  min_path_etx = INFINITY;
}

static void
parent_state_callback(rpl_parent_t *parent, int known, int etx)
{
  rpl_dag_t *dag;

  dag = (rpl_dag_t *)parent->dag;

  if(known) {
    if(min_path_etx != INFINITY) {
      dag->rank = min_path_etx + etx;
      PRINTF("RPL: New path ETX: %u\n", (unsigned)(min_path_etx + etx));
    }
  } else {
    if(RPL_PARENT_COUNT(dag) == 1) {
      /* Our last parent has disappeared, set the path ETX to infinity. */
      min_path_etx = INFINITY;
    }
  }
}

static rpl_rank_t
increment_rank(rpl_rank_t rank, rpl_parent_t *parent)
{
  rpl_rank_t new_rank;

  PRINTF("RPL: OF1 increment rank\n");

  if(parent->rank < min_path_etx) {
    min_path_etx = parent->rank;
  }

  new_rank = parent->rank + LINK_ETX_MAX;

  if(new_rank < rank) {
    return INFINITE_RANK;
  }

  PRINTF("RPL: Path ETX %u\n", (unsigned)new_rank);

  return new_rank;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  if(p1->rank < p2->rank) {
    return p1;
  }

  return p2;
}
