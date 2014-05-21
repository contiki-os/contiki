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
 *         Header file for routing-set.c. We implement routing sets
 *         as Bloom filters, and have a generic driver interface for
 *         hashing (get_hash). Routing sets can be turned into simple
 *         bitmaps when using a collision-free hash that maps every
 *         global IPv6 in the network to a unique position in the set.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __orpl_routing_set_H__
#define __orpl_routing_set_H__

#include "contiki.h"

/* Existing types of routing set */
#define ORPL_RS_TYPE_BITMAP         1
#define ORPL_RS_TYPE_BLOOM_SAX      2

/* Type of routing set is use */
#ifdef OPRL_CONF_RS_TYPE
#define ORPL_RS_TYPE                 OPRL_CONF_RS_TYPE
#else
#define ORPL_RS_TYPE                 ORPL_RS_TYPE_BLOOM_SAX
#endif

/* Routing set size (in bits) */
#ifdef ORPL_CONF_ROUTING_SET_M
#define ROUTING_SET_M        ORPL_CONF_ROUTING_SET_M
#else
#define ROUTING_SET_M        512
#endif

#if ROUTING_SET_M > 512
#error "ROUTING_SET_M too large (max: 512)"
#endif

/* Routing set / Bloom filter number of hashes */
#if ORPL_RS_TYPE == ORPL_RS_TYPE_BITMAP
#define ROUTING_SET_K        1
#else /* ORPL_RS_TYPE == ORPL_RS_TYPE_BITMAP */
#ifdef ORPL_CONF_ROUTING_SET_K
#define ROUTING_SET_K        ORPL_CONF_ROUTING_SET_K
#else
#define ROUTING_SET_K        4
#endif
#endif /* ORPL_RS_TYPE == ORPL_RS_TYPE_BITMAP */

#if ROUTING_SET_K > 7
#error "ROUTING_SET_K too large (max: 7)"
#endif

/* A routing set is a bitmap/Bloom filter of size ROUTING_SET_M bits */
struct routing_set_s {
  unsigned char u8[ROUTING_SET_M / 8];
};

/* Initializes the global double routing set */
void orpl_routing_set_init();
/* Returns a pointer to the currently active routing set */
struct routing_set_s *orpl_routing_set_get_active();
/* Inserts a global IPv6 in the global double routing set */
void orpl_routing_set_insert(const uip_ipaddr_t *ipv6);
/* Merges a routing set into our global double routing set */
void orpl_routing_set_merge(const struct routing_set_s *rs);
/* Checks if our global double bloom filter contains an given IPv6 */
int orpl_routing_set_contains(const uip_ipaddr_t *ipv6);
/* Swap active and warmup routing sets for ageing */
void orpl_routing_set_swap();
/* Returns the number of bits set in the active routing set */
int orpl_routing_set_count_bits();

#endif /* __orpl_routing_set_H__ */

/*
 * Some basics about Bloom filter
 *
 * m: the size of the Bloom filter in bits
 * n: number of entries (elements inserted)
 * k: number of hashes
 *
 * We have a max filter size of 512
 * Each hash must be of size log2(512) = 9 bits
 * For simplicity, we generate a single 64-bit hash that we then split
 * in k hashes. k == 7 is our maximum k value as it results 7*9 = 63 bits
 *
 * False-positive rate for an optimal k is p = exp(-(m/n)*log(2)**2)
 * False positive rates for various m/n (number of bit per entry):
 *    m/n ==  1 => 61.9 %
 *    m/n ==  2 => 38.3 %
 *    m/n ==  4 => 14.6 %
 *    m/n ==  6 =>  5.5 %
 *    m/n ==  8 =>  2.1 %
 *    m/n == 10 =>  0.8 %
 *    m/n == 12 =>  0.3 %
 *    m/n == 14 =>  0.12 %
 *    m/n == 16 =>  0.05 %
 *
 * Optimal number of hash functions, k, is: (m/n)*log(2)
 *   m/n =  1 => ~  0.69
 *   m/n =  2 => ~  1.39
 *   m/n =  4 => ~  2.77
 *   m/n =  6 => ~  4.16
 *   m/n =  8 => ~  5.55
 *   m/n = 10 => ~  6.93
 *   m/n = 12 => ~  8.32
 *   m/n = 14 => ~  7.70
 *   m/n = 16 => ~ 11.09
 *
 * False positive rate with k=4: p = (1 - e**(-k*n/m))**k
 *    m/n ==  1 => 92.87 %
 *    m/n ==  2 => 55.89 %
 *    m/n ==  4 => 15.97 %
 *    m/n ==  6 =>  5.61 %
 *    m/n ==  8 =>  2.40 %
 *    m/n == 10 =>  1.18 %
 *    m/n == 12 =>  0.6 %
 *    m/n == 14 =>  0.38 %
 *    m/n == 16 =>  0.24 %
 *
 */
