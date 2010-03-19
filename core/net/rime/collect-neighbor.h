/**
 * \addtogroup rime
 * @{
 */
/**
 * \defgroup rimeneighbor Rime neighbor management
 * @{
 *
 * The neighbor module manages the neighbor table.
 */
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
 * $Id: collect-neighbor.h,v 1.1 2010/03/19 13:17:00 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Contiki radio neighborhood management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __COLLECT_NEIGHBOR_H__
#define __COLLECT_NEIGHBOR_H__

#include "net/rime/rimeaddr.h"

#define COLLECT_NEIGHBOR_ETX_SCALE 16
#define COLLECT_NEIGHBOR_NUM_ETXS 4

struct collect_neighbor {
  struct collect_neighbor *next;
  uint16_t time;
  rimeaddr_t addr;
  uint16_t rtmetric;
  uint8_t etxptr;
  uint8_t etxs[COLLECT_NEIGHBOR_NUM_ETXS];
};

void collect_neighbor_init(void);
/*void collect_neighbor_periodic(int max_time);*/

void collect_neighbor_add(const rimeaddr_t *addr, uint8_t rtmetric, uint8_t etx);
void collect_neighbor_update(struct collect_neighbor *n, uint8_t rtmetric);
void collect_neighbor_update_etx(struct collect_neighbor *n, uint8_t etx);
void collect_neighbor_timedout_etx(struct collect_neighbor *n, uint8_t etx);
void collect_neighbor_remove(const rimeaddr_t *addr);
struct collect_neighbor *collect_neighbor_find(const rimeaddr_t *addr);
struct collect_neighbor *collect_neighbor_best(void);
void collect_neighbor_set_lifetime(int seconds);

uint8_t collect_neighbor_etx(struct collect_neighbor *n);

int collect_neighbor_num(void);
struct collect_neighbor *collect_neighbor_get(int num);

void collect_neighbor_purge(void);

#endif /* __COLLECT_NEIGHBOR_H__ */
/** @} */
/** @} */
