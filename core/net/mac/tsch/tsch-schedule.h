/*
 * Copyright (c) 2014, SICS Swedish ICT.
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

#ifndef __TSCH_SCHEDULE_H__
#define __TSCH_SCHEDULE_H__

/********** Includes **********/

#include "contiki.h"
#include "lib/list.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-queue.h"
#include "net/mac/tsch/tsch-slot-operation.h"
#include "net/linkaddr.h"

/******** Configuration *******/

/* Initializes TSCH with a 6TiSCH minimal schedule */
#ifdef TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL
#define TSCH_SCHEDULE_WITH_6TISCH_MINIMAL TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL
#else
#define TSCH_SCHEDULE_WITH_6TISCH_MINIMAL 1
#endif

/* 6TiSCH Minimal schedule slotframe length */
#ifdef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_DEFAULT_LENGTH TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#else
#define TSCH_SCHEDULE_DEFAULT_LENGTH 7
#endif

/* Max number of TSCH slotframes */
#ifdef TSCH_SCHEDULE_CONF_MAX_SLOTFRAMES
#define TSCH_SCHEDULE_MAX_SLOTFRAMES TSCH_SCHEDULE_CONF_MAX_SLOTFRAMES
#else
#define TSCH_SCHEDULE_MAX_SLOTFRAMES 4
#endif

/* Max number of links */
#ifdef TSCH_SCHEDULE_CONF_MAX_LINKS
#define TSCH_SCHEDULE_MAX_LINKS TSCH_SCHEDULE_CONF_MAX_LINKS
#else
#define TSCH_SCHEDULE_MAX_LINKS 32
#endif

/********** Constants *********/

/* Link options */
#define LINK_OPTION_TX              1
#define LINK_OPTION_RX              2
#define LINK_OPTION_SHARED          4
#define LINK_OPTION_TIME_KEEPING    8

/************ Types ***********/

/* 802.15.4e link types.
 * LINK_TYPE_ADVERTISING_ONLY is an extra one: for EB-only links. */
enum link_type { LINK_TYPE_NORMAL, LINK_TYPE_ADVERTISING, LINK_TYPE_ADVERTISING_ONLY };

struct tsch_link {
  /* Links are stored as a list: "next" must be the first field */
  struct tsch_link *next;
  /* Unique identifier */
  uint16_t handle;
  /* MAC address of neighbor */
  linkaddr_t addr;
  /* Slotframe identifier */
  uint16_t slotframe_handle;
  /* Identifier of Slotframe to which this link belongs
   * Unused. */
  /* uint8_t handle; */
  /* Timeslot for this link */
  uint16_t timeslot;
  /* Channel offset for this link */
  uint16_t channel_offset;
  /* A bit string that defines
   * b0 = Transmit, b1 = Receive, b2 = Shared, b3 = Timekeeping, b4 = reserved */
  uint8_t link_options;
  /* Type of link. NORMAL = 0. ADVERTISING = 1, and indicates
     the link may be used to send an Enhanced beacon. */
  enum link_type link_type;
  /* Any other data for upper layers */
  void *data;
};

struct tsch_slotframe {
  /* Slotframes are stored as a list: "next" must be the first field */
  struct tsch_slotframe *next;
  /* Unique identifier */
  uint16_t handle;
  /* Number of timeslots in the slotframe.
   * Stored as struct asn_divisor_t because we often need ASN%size */
  struct tsch_asn_divisor_t size;
  /* List of links belonging to this slotframe */
  LIST_STRUCT(links_list);
};

/********** Functions *********/

/* Module initialization, call only once at startup. Returns 1 is success, 0 if failure. */
int tsch_schedule_init(void);
/* Create a 6TiSCH minimal schedule */
void tsch_schedule_create_minimal(void);
/* Prints out the current schedule (all slotframes and links) */
void tsch_schedule_print(void);

/* Adds and returns a slotframe (NULL if failure) */
struct tsch_slotframe *tsch_schedule_add_slotframe(uint16_t handle, uint16_t size);
/* Looks for a slotframe from a handle */
struct tsch_slotframe *tsch_schedule_get_slotframe_by_handle(uint16_t handle);
/* Removes a slotframe Return 1 if success, 0 if failure */
int tsch_schedule_remove_slotframe(struct tsch_slotframe *slotframe);
/* Removes all slotframes, resulting in an empty schedule */
int tsch_schedule_remove_all_slotframes(void);

/* Returns next slotframe */
struct tsch_slotframe *tsch_schedule_slotframes_next(struct tsch_slotframe *sf);
/* Adds a link to a slotframe, return a pointer to it (NULL if failure) */
struct tsch_link *tsch_schedule_add_link(struct tsch_slotframe *slotframe,
                                         uint8_t link_options, enum link_type link_type, const linkaddr_t *address,
                                         uint16_t timeslot, uint16_t channel_offset);
/* Looks for a link from a handle */
struct tsch_link *tsch_schedule_get_link_by_handle(uint16_t handle);
/* Looks within a slotframe for a link with a given timeslot */
struct tsch_link *tsch_schedule_get_link_by_timeslot(struct tsch_slotframe *slotframe, uint16_t timeslot);
/* Removes a link. Return 1 if success, 0 if failure */
int tsch_schedule_remove_link(struct tsch_slotframe *slotframe, struct tsch_link *l);
/* Removes a link from slotframe and timeslot. Return a 1 if success, 0 if failure */
int tsch_schedule_remove_link_by_timeslot(struct tsch_slotframe *slotframe, uint16_t timeslot);

/* Returns the next active link after a given ASN, and a backup link (for the same ASN, with Rx flag) */
struct tsch_link * tsch_schedule_get_next_active_link(struct tsch_asn_t *asn, uint16_t *time_offset,
    struct tsch_link **backup_link);

#endif /* __TSCH_SCHEDULE_H__ */
