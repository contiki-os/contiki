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
 *         Header file for the Contiki radio neighborhood management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeneighbor Collect neighbor management
 * @{
 *
 * The neighbor module manages the neighbor table that is used by the
 * Collect module.
 */

#ifndef COLLECT_NEIGHBOR_H_
#define COLLECT_NEIGHBOR_H_

#include "net/linkaddr.h"
#include "net/rime/collect-link-estimate.h"
#include "net/nbr-table.h"

/* NBR Table for RIME Collect Neighbors */
NBR_TABLE_DECLARE(collect_nbr_table);

/* An entry in the RIME Collect NBR cache */
typedef struct collect_nbr {
  uint16_t rtmetric;
  uint16_t age;
  uint16_t le_age;
  struct collect_link_estimate le;
  struct timer congested_timer;
} collect_nbr_t;

/* Init Collect NBR Table */
void collect_nbr_init(void);

/* Add a neighbor to the Collect NBR Table */
int collect_nbr_add(const linkaddr_t *addr, uint16_t rtmetric);

/* Remove a neighbor from the Collect NBR Table */
void collect_nbr_rm(collect_nbr_t *n);

/* Get a neighbor from the Collect NBR Table with a specific LLADDR */
collect_nbr_t *collect_nbr_find(const linkaddr_t *addr);

/* Get the best neighbor in the Collect NBR Table */
collect_nbr_t *collect_nbr_best(void);

/* Get the worst neighbor in the Collect NBR Table */
collect_nbr_t *collect_nbr_worst(void);

/* Get number of neighbors in the Collect NBR Table */
int collect_nbr_num(void);

/* Get neighbor in the Collect NBR Table from index number */
collect_nbr_t *collect_nbr_get(int num);

/* Purge the Collect NBR Table - TO DO */
void collect_nbr_purge(void);

void collect_nbr_update_rtmetric(collect_nbr_t *n, uint16_t rtmetric);
void collect_nbr_tx(collect_nbr_t *n, uint16_t num_tx);
void collect_nbr_rx(collect_nbr_t *n);
void collect_nbr_tx_fail(collect_nbr_t *n, uint16_t num_tx);
void collect_nbr_set_congested(collect_nbr_t *n);
int collect_nbr_is_congested(collect_nbr_t *n);
uint16_t collect_nbr_link_estimate(collect_nbr_t *n);
uint16_t collect_nbr_rtmetric_link_estimate(collect_nbr_t *n);
uint16_t collect_nbr_rtmetric(const collect_nbr_t *n);

#endif /* COLLECT_NEIGHBOR_H_ */
/** @} */
/** @} */
