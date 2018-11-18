/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
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
 *         Adaptive Key Establishment Scheme (AKES).
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#ifndef AKES_H_
#define AKES_H_

#include "net/llsec/adaptivesec/adaptivesec.h"
#include "net/llsec/adaptivesec/akes-nbr.h"

#ifdef AKES_CONF_MAX_WAITING_PERIOD
#define AKES_MAX_WAITING_PERIOD AKES_CONF_MAX_WAITING_PERIOD
#else /* AKES_CONF_MAX_WAITING_PERIOD */
#define AKES_MAX_WAITING_PERIOD (5) /* seconds */
#endif /* AKES_CONF_MAX_WAITING_PERIOD */

#ifdef AKES_CONF_ACK_DELAY
#define AKES_ACK_DELAY AKES_CONF_ACK_DELAY
#else /* AKES_CONF_ACK_DELAY */
#define AKES_ACK_DELAY (5) /* seconds */
#endif /* AKES_CONF_ACK_DELAY */

/* Defines the plugged-in scheme */
#ifdef AKES_CONF_SCHEME
#define AKES_SCHEME AKES_CONF_SCHEME
#else /* AKES_CONF_SCHEME */
#define AKES_SCHEME akes_single_scheme
#endif /* AKES_CONF_SCHEME */

#if AKES_NBR_WITH_GROUP_KEYS
#define AKES_ACKS_SEC_LVL (ADAPTIVESEC_UNICAST_SEC_LVL | (1 << 2))
#else /* AKES_NBR_WITH_GROUP_KEYS */
#define AKES_ACKS_SEC_LVL (ADAPTIVESEC_UNICAST_SEC_LVL & 3)
#endif /* AKES_NBR_WITH_GROUP_KEYS */

#define AKES_UPDATES_SEC_LVL (ADAPTIVESEC_UNICAST_SEC_LVL & 3)

/* Command frame identifiers */
enum {
  AKES_HELLO_IDENTIFIER = 0x0A,
  AKES_HELLOACK_IDENTIFIER = 0x0B,
  AKES_HELLOACK_P_IDENTIFIER = 0x1B,
  AKES_ACK_IDENTIFIER = 0x0C,
  AKES_UPDATE_IDENTIFIER = 0x0E,
  AKES_UPDATEACK_IDENTIFIER = 0x0F
};

/**
 * Structure of a pluggable scheme
 */
struct akes_scheme {

  /** Called at startup */
  void (* init)(void);

  /**
   * \return      Shared secret of length AES_128_KEY_LENGTH
   * \retval NULL HELLO shall be discarded
   */
  uint8_t * (* get_secret_with_hello_sender)(const linkaddr_t *addr);

  /**
   * \return      Shared secret of length AES_128_KEY_LENGTH
   * \retval NULL HELLOACK shall be discarded
   */
  uint8_t * (* get_secret_with_helloack_sender)(const linkaddr_t *addr);
};

extern const struct akes_scheme AKES_SCHEME;

void akes_broadcast_hello(void);
int akes_is_acceptable_helloack(void);
int akes_is_acceptable_ack(struct akes_nbr_entry *entry);
void akes_init(void);
void akes_send_update(struct akes_nbr_entry *entry);
enum akes_nbr_status akes_get_receiver_status(void);

#endif /* AKES_H_ */
