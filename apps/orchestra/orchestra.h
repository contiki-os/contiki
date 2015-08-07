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
 *         Orchestra header file
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __ORCHESTRA_H__
#define __ORCHESTRA_H__

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"

/* The structure of an Orchestra rule */
struct orchestra_rule {
  void (* init)(uint16_t slotframe_handle);
  void (* new_time_source)(struct tsch_neighbor *old, struct tsch_neighbor *new);
  int  (* select_packet)(uint16_t *slotframe, uint16_t *timeslot);
  void (* child_added)(linkaddr_t *addr);
  void (* child_removed)(linkaddr_t *addr);
};

struct orchestra_rule eb_per_time_source;
struct orchestra_rule unicast_per_neighbor;
struct orchestra_rule default_common;

#define ORCHESTRA_RULES { &eb_per_time_source, \
                          &unicast_per_neighbor, \
                          &default_common, \
                        }

#define ORCHESTRA_EBSF_PERIOD                     397
#define ORCHESTRA_COMMON_SHARED_PERIOD            31
#define ORCHESTRA_UNICAST_PERIOD                  17

#define ORCHESTRA_LINKADDR_HASH(addr)             ((addr != NULL) ? (addr)->u8[LINKADDR_SIZE-1] : -1)
#define ORCHESTRA_MAX_HASH                        0x7fff
#define ORCHESTRA_COLLISION_FREE_HASH             0 /* Set to 1 if ORCHESTRA_LINKADDR_HASH returns unique hashes */

extern linkaddr_t orchestra_parent_linkaddr;
extern int orchestra_parent_knows_us;

void orchestra_callback_packet_ready();
void orchestra_callback_new_time_source(struct tsch_neighbor *old, struct tsch_neighbor *new);
void orchestra_callback_child_added(linkaddr_t *addr);
void orchestra_callback_child_removed(linkaddr_t *addr);
void orchestra_init();

#endif /* __ORCHESTRA_H__ */
