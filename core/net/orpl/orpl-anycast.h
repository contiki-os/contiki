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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         orpl-anycast.c header file
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */


#ifndef __ORPL_ANYCAST_H__
#define __ORPL_ANYCAST_H__

#include "uip.h"

#define EXTRA_ACK_LEN    10 /* Number of bytes we add to standard IEEE 802.15.4 ACK frames */

/* The different link-layer addresses used for anycast */
extern rimeaddr_t anycast_addr_up;
extern rimeaddr_t anycast_addr_down;
extern rimeaddr_t anycast_addr_nbr;
extern rimeaddr_t anycast_addr_recover;

enum anycast_direction_e {
  direction_none,
  direction_up,
  direction_down,
  direction_nbr,
  direction_recover
};

struct anycast_parsing_info {
  uint8_t do_ack;
  enum anycast_direction_e direction;
  uint16_t neighbor_edc;
  uint32_t seqno;
};

/* Set the destination link-layer address in packetbuf in case of anycast */
void orpl_anycast_set_packetbuf_addr();
/* Parse a modified 802.15.4 frame */
struct anycast_parsing_info orpl_anycast_parse_802154_frame(uint8_t *data, uint8_t len, int set_dest_addr);
/* Anycast-specific inits */
void orpl_anycast_init();

#endif /* __ORPL_ANYCAST_H__ */
