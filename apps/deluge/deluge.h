/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 *	Header for Deluge.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef DELUGE_H
#define DELUGE_H

#include "net/rime.h"

PROCESS_NAME(deluge_process);

#define LONG_TIMER(et, counter, time)			\
  do {							\
    for (counter = 0; counter < time; counter++) {	\
      etimer_set(&et, CLOCK_SECOND);			\
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	\
    }							\
  } while (0)

#define DELUGE_UNICAST_CHANNEL		55
#define DELUGE_BROADCAST_CHANNEL	56

/* All the packets in a page have been received. */
#define PAGE_COMPLETE	1
/* All pages up to, and including, this page are complete. */
#define PAGE_AVAILABLE	1

#define S_PKT		64		/* Deluge packet size. */
#define N_PKT		4		/* Packets per page. */
#define S_PAGE		(S_PKT * N_PKT)	/* Fixed page size. */

/* Bounds for the round time in seconds. */
#define T_LOW		2
#define T_HIGH		64

/* Random interval for request transmissions in jiffies. */
#define T_R		(CLOCK_SECOND * 2)

/* Bound for the number of advertisements. */
#define CONST_K		1

/* The number of pages in this object. */
#define OBJECT_PAGE_COUNT(obj)	(((obj).size + (S_PAGE - 1)) / S_PAGE)

#define ALL_PACKETS		((1 << N_PKT) - 1)

#define DELUGE_CMD_SUMMARY	1
#define DELUGE_CMD_REQUEST	2
#define DELUGE_CMD_PACKET	3
#define DELUGE_CMD_PROFILE	4

#define DELUGE_STATE_MAINTAIN	1
#define DELUGE_STATE_RX		2
#define DELUGE_STATE_TX		3

#define CONST_LAMBDA		2
#define CONST_ALPHA		0.5

#define CONST_OMEGA		8
#define ESTIMATED_TX_TIME	(CLOCK_SECOND)

typedef uint8_t deluge_object_id_t;

struct deluge_msg_summary {
  uint8_t cmd;
  uint8_t version;
  uint8_t highest_available;
  deluge_object_id_t object_id;
};

struct deluge_msg_request {
  uint8_t cmd;
  uint8_t version;
  uint8_t pagenum;
  uint8_t request_set;
  deluge_object_id_t object_id;
};

struct deluge_msg_packet {
  uint8_t cmd;
  uint8_t version;
  uint8_t pagenum;
  uint8_t packetnum;
  uint16_t crc;
  deluge_object_id_t object_id;
  unsigned char payload[S_PKT];
};

struct deluge_msg_profile {
  uint8_t cmd;
  uint8_t version;
  uint8_t npages;
  deluge_object_id_t object_id;
  uint8_t version_vector[];
};

struct deluge_object {
  char *filename;
  uint16_t object_id;
  uint16_t size;
  uint8_t version;
  uint8_t update_version;
  struct deluge_page *pages;
  uint8_t current_rx_page;
  int8_t current_tx_page;
  uint8_t nrequests;
  uint8_t current_page[S_PAGE];
  uint8_t tx_set;
  int cfs_fd;
  rimeaddr_t summary_from;
};

struct deluge_page {
  uint32_t packet_set;
  uint16_t crc;
  clock_time_t last_request;
  clock_time_t last_data;
  uint8_t flags;
  uint8_t version;
};

int deluge_disseminate(char *file, unsigned version);

#endif
