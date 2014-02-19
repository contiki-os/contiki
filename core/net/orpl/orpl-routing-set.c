/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 *         Routing set support for ORPL.  We implement routing sets
 *         as Bloom filters, and have a generic driver interface for
 *         hashing (get_hash). Routing sets can be turned into simple
 *         bitmaps when using a collision-free hash that maps every
 *         global IPv6 in the network to a unique position in the set.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "net/uip.h"
#include "orpl.h"
#include "orpl-routing-set.h"
#include "node-id.h"
#include <string.h>
#include <stdio.h>

#if WITH_ORPL

/* We maintain two routing sets, one "active" and one "warmup" to implement ageing. */
static routing_set routing_sets[2];
/* Index of the currently active set. 1-current is the warmup one. */
static int active_index;

#if ORPL_RS_TYPE == ORPL_RS_TYPE_BITMAP

/* In the bitmap case, we uniquely map all global ipv6 in the
   * network to a unique index, based on a deployment-specific
   * node_id_from_ipaddr(ipv6) function */

#include "deployment.h"
/* Returns the 64-bit hash of a given global IPv6 */
static uint64_t
get_hash(const uip_ipaddr_t *ipv6)
{
  return node_id_from_ipaddr(ipv6);
}

#elif ORPL_RS_TYPE == ORPL_RS_TYPE_BLOOM_SAX

/* Simple SAX (shift-and-xor) hash */

/* Returns the 64-bit hash of a given global IPv6 */
static uint64_t
get_hash(const uip_ipaddr_t *ipv6)
{
  int i;
  uint64_t hash;
  memcpy(&hash, ipv6->u8+8, 8); /* Initialize our hash with the IPv6 IID
  rather than 0 for increased entropy (when initialized with 0 and hashing
  entries of only 16 bytes, SAX produces comparatively many collisions) */
  for(i=0; i<16; i++) {
    hash ^= ( hash << 5 ) + ( hash >> 2 ) + ipv6->u8[i];
  }
  return hash;
}

#else

#error "ORPL_RS_TYPE not supported"

#endif /* ORPL_RS_TYPE == ORPL_RS_TYPE_BITMAP */

/* Set a bit in a routing set */
static void
rs_set_bit(routing_set rs, int i) {
  rs[i/8] |= 1 << (i%8);
}

/* Get a bit in a routing set */
static int
rs_get_bit(routing_set rs, int i) {
  return (rs[i/8] & (1 << (i%8))) != 0;
}

/* Initializes the global double routing set */
void
orpl_routing_set_init()
{
  memset(routing_sets, 0, sizeof(routing_sets));
}

/* Returns a pointer to the currently active routing set */
routing_set *
orpl_routing_set_get_active() {
  return &routing_sets[active_index];
}

/* Inserts a global IPv6 in the global double routing set */
void
orpl_routing_set_insert(const uip_ipaddr_t *ipv6)
{
  int k;
  uint64_t hash = get_hash(ipv6);
  /* For each hash, set a bit in both routing sets */
  for(k=0; k<ROUTING_SET_K; k++) {
    rs_set_bit(routing_sets[0], hash % ROUTING_SET_M);
    rs_set_bit(routing_sets[1], hash % ROUTING_SET_M);
    hash /= ROUTING_SET_M;
  }
}

/* Merges a routing set into our global double routing set */
void
orpl_routing_set_merge(routing_set rs)
{
  int i;
  for(i=0; i<sizeof(routing_set); i++) {
    /* We merge into both active and warmup routing sets.
     * Merging is ORing */
    routing_sets[0][i] |= rs[i];
    routing_sets[1][i] |= rs[i];
  }
}

/* Checks if our global double routing set contains an given IPv6 */
int
orpl_routing_set_contains(const uip_ipaddr_t *ipv6)
{
  int k;
  int contains = 1;
  uint64_t hash = get_hash(ipv6);
  /* For each hash, check a bit in the bloom filter */
  for(k=0; k<ROUTING_SET_K; k++) {
    /* Check against the active routing set */
    if(rs_get_bit(*orpl_routing_set_get_active(), hash % ROUTING_SET_M) == 0) {
      /* If one bucket is empty, then the element isn't included in the filter */
      contains = 0;
      break;
    hash /= ROUTING_SET_M;
    }
  }
  return contains;
}

/* Swap active and warmup routing sets for ageing */
void
orpl_routing_set_swap()
{
  /* Swap active flag */
  active_index = 1 - active_index;
  /* Reset the newly inactive routing set */
  memset(routing_sets[1 - active_index], 0, sizeof(routing_set));
}

/* Returns the number of bits set in the active routing set */
int
orpl_routing_set_count_bits()
{
  int i;
    int cnt = 0;
    for(i=0; i<ROUTING_SET_M; i++) {
      if(rs_get_bit(*orpl_routing_set_get_active(), i)) {
        cnt++;
      }
    }
  return cnt;
}

/* Prints out the content of the active routing set */
void
orpl_routing_set_print()
{
  printf("Routing set: bits set %d/%d\n", orpl_routing_set_count_bits(), ROUTING_SET_M);
  printf("Routing set: start\n");
  int i;
  for(i=0; i<ROUTING_SET_M/8; i++) {
    if(i%16 == 0) {
      printf("Routing set: [%2u] ", i/16);
    }
    printf("%02x ", *orpl_routing_set_get_active()[i]);
    if(i%16 == 15) {
      printf("\n");
    }
  }
  printf("\nRouting set: end\n");
}

#endif /* WITH_ORPL */
