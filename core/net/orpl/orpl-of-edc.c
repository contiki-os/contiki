/* Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *         Implementation of the EDC (Excpected Duty Cycles) metric as an
 *         objective function for ORPL
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "net/rpl/rpl-private.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
#include "orpl.h"
#include "orpl-anycast.h"
#include "packetbuf.h"

#if WITH_ORPL

static void reset(rpl_dag_t *);
static void neighbor_link_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_dag_t *best_dag(rpl_dag_t *, rpl_dag_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);
static void update_metric_container(rpl_instance_t *);

rpl_of_t rpl_of_edc = {
  reset,
  neighbor_link_callback,
  best_parent,
  best_dag,
  calculate_rank,
  update_metric_container,
  1
};

/* Constants for the ETX moving average */
#define EDC_SCALE   100
#define EDC_ALPHA   90

/* Current hop-by-hop EDC, which is the expected strobe duration before getting
 * an ack from a parent. We maintain this as a moving average. */
static uint16_t hbh_edc = EDC_DIVISOR;

/* The size of the forwarder set and neighbor set.
 * Both are needed in some other parts of ORPL. */
int forwarder_set_size = 0;
int neighbor_set_size = 0;

/* Utility function for computing the forwarder set. Adds a parent and returns the
 * resulting EDC */
static int
add_to_forwarder_set(rpl_parent_t *curr_p, rpl_rank_t curr_p_rank, uint16_t ackcount,
    uint32_t *curr_ackcount_sum, uint32_t *curr_ackcount_edc_sum, int verbose)
{
  uint16_t tentative_edc;
  uint32_t total_tx_count;

  if(ackcount > orpl_broadcast_count) {
    ackcount = orpl_broadcast_count;
  }

  total_tx_count = orpl_broadcast_count;
  if(total_tx_count == 0) {
    /* No broadcast sent yet: assume a reception rate of 50% */
    ackcount = 1;
    total_tx_count = 2;
  }

  *curr_ackcount_sum += ackcount;
  *curr_ackcount_edc_sum += ackcount * curr_p_rank;

  /* The two main components of EDC: A, the cost of forwarding to any
   * parent, B the weighted mean EDC of the forwarder set */
  uint32_t A = hbh_edc * total_tx_count / *curr_ackcount_sum;
  uint32_t B = *curr_ackcount_edc_sum / *curr_ackcount_sum;
  if(verbose) {
    printf("-- A: %5lu, B: %5lu (%u/%lu) ",
        A,
        B,
        ackcount,
        total_tx_count
    );
  }

  /* Finally add W to EDC (cost of forwarding) */
  tentative_edc = A + B + ORPL_EDC_W;

  if(verbose) {
    printf("EDC %5u ", tentative_edc);
  }

  return tentative_edc;
}

/* Function that computes the metric EDC */
rpl_rank_t
orpl_calculate_edc(int verbose)
{
  rpl_rank_t edc = 0xffff;
  rpl_rank_t prev_edc = orpl_current_edc();
  /* Counts the total number of ACKs received from nodes in the current set */
  uint32_t curr_ackcount_sum = 0;
  /* Counts the total number of EDC*ACKs received from nodes in the current set */
  uint32_t curr_ackcount_edc_sum = 0;
  /* Variables used for looping over parents and building the forwarder set */
  rpl_parent_t *p, *curr_p;
  int index = 0, curr_index = 0;
  uint16_t curr_p_rank = 0xffff;
  uint16_t curr_p_ackcount = 0xffff;

  int prev_index = -1;
  uint16_t prev_min_rank = 0;

  if(orpl_is_edc_frozen()) {
    return prev_edc;
  }

  if(orpl_is_root()) {
    return 0;
  }

  forwarder_set_size = 0;
  neighbor_set_size = 0;

  if(verbose) {
    printf("ORPL: starting EDC calculation. hbh_edc: %u, e2e_edc %u\n", hbh_edc, orpl_current_edc());
  }

  /* Loop over the parents ordered by increasing rank, try to insert
   * them in the routing set until EDC does not improve. This is as
   * described in the IPSN'12 paper on ORW (upon which ORPL is built) */
  do {
    curr_p = NULL;

    /* This nested for loop finds the next parent for the do loop,
     * such as we get the parents by increasing rank */
    for(p = nbr_table_head(rpl_parents), index = 0;
        p != NULL;
        p = nbr_table_next(rpl_parents, p), index++) {
      uint16_t rank = p->rank;
      uint16_t ackcount = p->bc_ackcount;

      if(rank != 0xffff
          && !(orpl_broadcast_count > 0 && ackcount == 0)
          && (curr_p == NULL || rank < curr_p_rank)
          && (rank > prev_min_rank || (rank == prev_min_rank && index > prev_index))
      ) {
        curr_index = index;
        curr_p = p;
        curr_p_rank = rank;
        curr_p_ackcount = ackcount;
      }
    }

    /* Here, curr_p contains the current parent in our ordered lookup */
    if(curr_p) {
      uint16_t curr_id = rpl_get_parent_ipaddr(curr_p)->u8[sizeof(uip_ipaddr_t) - 1];
      rpl_rank_t tentative_edc;

      if(verbose) {
        printf("ORPL: EDC -> node %3u rank: %5u ack %u/%u ", curr_id, curr_p_rank, curr_p_ackcount, orpl_broadcast_count);
      }

      tentative_edc = add_to_forwarder_set(curr_p, curr_p_rank, curr_p_ackcount,
                &curr_ackcount_sum, &curr_ackcount_edc_sum, verbose);

      neighbor_set_size++;

      if(tentative_edc < edc) {
        /* The parent is now part of the forwarder set */
        edc = tentative_edc;
        forwarder_set_size++;
        if(verbose) {
          printf("*\n");
        }
        ANNOTATE("#L %u 1\n", curr_id);
      } else {
        /* The parent is not part of the forwarder set. This means next parents won't be
         * part of the set either. */
        if(verbose) {
          printf("\n");
        }
        ANNOTATE("#L %u 0\n", curr_id);
      }
      prev_index = curr_index;
      prev_min_rank = curr_p_rank;
    }
  } while(curr_p != NULL);

  if(verbose) {
    printf("ORPL: final edc %u\n", edc);
  }

  if(edc != prev_edc) {
    ANNOTATE("#A edc=%u.%u\n", edc/EDC_DIVISOR,
        (10 * (edc % EDC_DIVISOR)) / EDC_DIVISOR);
  }

  return edc;
}

static void
reset(rpl_dag_t *sag)
{
  PRINTF("ORPL: reset EDC\n");
  hbh_edc = EDC_DIVISOR;
  forwarder_set_size = 0;
  neighbor_set_size = 0;
}

/* Called after transmitting to a neighbor */
static void
neighbor_link_callback(rpl_parent_t *parent, int known, int edc)
{
  /* First check if we are allowed to change rank */
  if(orpl_is_edc_frozen()) {
    return;
  }
  /* Calculate the average hop-by-hop EDC, i.e. the average strobe time
   * required before getting our anycast ACKed. We compute this only for
   * upwards traffic, as the metric and the topology are directed to the root */
  if(packetbuf_attr(PACKETBUF_ATTR_ORPL_DIRECTION) == direction_up) {
    uint16_t curr_hbh_edc = packetbuf_attr(PACKETBUF_ATTR_EDC); /* The strobe time for this packet */
    uint16_t weighted_curr_hbh_edc;
    uint16_t hbh_edc_prev = hbh_edc;
    if(curr_hbh_edc == 0xffff) { /* This was NOACK, use a more aggressive alpha (of 50%) */
      weighted_curr_hbh_edc = EDC_DIVISOR * 2 * forwarder_set_size;
      hbh_edc = (hbh_edc_prev * (EDC_SCALE/2) + weighted_curr_hbh_edc * (EDC_SCALE/2)) / EDC_SCALE;
    } else {
      weighted_curr_hbh_edc = curr_hbh_edc * forwarder_set_size;
      hbh_edc = ((hbh_edc_prev * EDC_ALPHA) + (weighted_curr_hbh_edc * (EDC_SCALE-EDC_ALPHA))) / EDC_SCALE;
    }

    PRINTF("ORPL: updated hbh_edc %u -> %u (%u %u)\n", hbh_edc_prev, hbh_edc, curr_hbh_edc, weighted_curr_hbh_edc);

    /* Calculate EDC and update rank */
    if(parent && parent->dag) {
      parent->dag->rank = calculate_rank(parent, 0);
    }
  }
}

static rpl_rank_t
calculate_rank(rpl_parent_t *parent, rpl_rank_t base_rank)
{
  /* EDC is an estimate of the number of cycles to reach the root with
   * multi-path routing, using all potential forwarders. We therefore
   * update the ORPL EDC every time we calculate it. */
  rpl_rank_t edc = orpl_calculate_edc(0);
  orpl_update_edc(edc);
  return edc;
}

static rpl_dag_t *
best_dag(rpl_dag_t *d1, rpl_dag_t *d2)
{
  if(d1->grounded != d2->grounded) {
    return d1->grounded ? d1 : d2;
  }

  if(d1->preference != d2->preference) {
    return d1->preference > d2->preference ? d1 : d2;
  }

  return d1->rank < d2->rank ? d1 : d2;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  /* With EDC, we don't need to compare parents */
  return p1;
}

static void
update_metric_container(rpl_instance_t *instance)
{
  /* We don't use any metric container (we only
   * use the rank field of DIO messages) */
}

#endif /* WITH_ORPL */
