/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         A sixtop protocol that performs link addition/deletion
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 */

#ifndef __SIXTOP_H__
#define __SIXTOP_H__

#include "contiki.h"
#include "stdbool.h"
#include "net/ip/uip-debug.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-asn.h"

#define SIXTOP_SUBIE_ID                                   0x00
#define SIXTOP_VERSION                                    0x01

/* Link Option OFF */
#define LINK_OPTION_OFF                                   0

/* Maximum number of retransmissions permissible at MAC layer */
#define SIXTOP_CONF_MAX_MAC_TRANSMISSIONS                 3

/* 6P Command ID */
enum sixtop_command_id {
  CMD_ADD       = 0x01,
  CMD_DELETE    = 0x02,
  CMD_COUNT     = 0x03,
  CMD_LIST      = 0x04,
  CMD_CLEAR     = 0x05,
};

/* 6P Return Code */
enum sixtop_return_code {
  RC_SUCCESS    = 0x06, /* Operation succeeded */
  RC_VER_ERR    = 0x07, /* Unsupported 6P version */
  RC_SFID_ERR   = 0x08, /* Unsupported SFID */
  RC_BUSY       = 0x09, /* Handling previous request */
  RC_RESET      = 0x0a, /* Abort 6P transaction */
  RC_ERR        = 0x0b, /* Operation failed */
};

/* Sixtop State Machine */
enum {
  SIXTOP_IDLE             = 0x00,
  SIXTOP_ADD_REQUEST_WAIT_SENDDONE  = 0x01,       /* Waiting for SendDone confirmation of Add Request */
  SIXTOP_ADD_RESPONSE_WAIT      = 0x02,     /* Waiting for Add Response */
  SIXTOP_ADD_RESPONSE_WAIT_SENDDONE   = 0x03,     /* Waiting for SendDone confirmation of Add Response */
  SIXTOP_ADD_RESPONSE_RECEIVED    = 0x04,     /* Received Add Response */
  SIXTOP_DELETE_REQUEST_WAIT_SENDDONE = 0x05,     /* Waiting for SendDone confirmation of Delete Request */
  SIXTOP_DELETE_RESPONSE_WAIT     = 0x06,     /* waiting for Delete Response */
  SIXTOP_DELETE_RESPONSE_WAIT_SENDDONE = 0x07,    /* Waiting for SendDone confirmation of Add Response */
  SIXTOP_DELETE_RESPONSE_RECEIVED   = 0x08,     /* Received Delete Response */
} sixtop_state;

/********** Functions *********/
/* Initiates a Sixtop Link addition */
int sixtop_add_links(linkaddr_t *dest_addr, uint8_t num_Links);
/* Initiates a Sixtop Link deletion */
int sixtop_remove_link(linkaddr_t *dest_addr);
/* Is it a Sixtop IE? Returns 0 if success */
int sixtop_is_sixtop_ie(const uint8_t *buf, int buf_size, frame802154_t *frame, struct ieee802154_ies *ies);
/* Set the Sequence Number of Link Response as in Link Request */
void sixtop_set_seqno(uint8_t seq_num);
/* Parse a Sixtop IE. Returns length of IE */
int sixtop_parse_ie(const uint8_t *buf, linkaddr_t *dest_addr);

#endif /* __SIXTOP_H__ */
