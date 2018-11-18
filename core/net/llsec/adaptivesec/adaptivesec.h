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
 *         Adaptive LLSEC driver.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#ifndef ADAPTIVESEC_H_
#define ADAPTIVESEC_H_

#include "net/mac/frame802154.h"
#include "net/llsec/llsec802154.h"
#include "net/llsec/llsec.h"
#include "net/llsec/adaptivesec/akes-nbr.h"
#include "lib/ccm-star.h"
#include "lib/aes-128.h"

#ifdef ADAPTIVESEC_CONF_UNICAST_SEC_LVL
#define ADAPTIVESEC_UNICAST_SEC_LVL ADAPTIVESEC_CONF_UNICAST_SEC_LVL
#else /* ADAPTIVESEC_CONF_UNICAST_SEC_LVL */
#define ADAPTIVESEC_UNICAST_SEC_LVL 2
#endif /* ADAPTIVESEC_CONF_UNICAST_SEC_LVL */

#ifdef ADAPTIVESEC_CONF_UNICAST_MIC_LEN
#define ADAPTIVESEC_UNICAST_MIC_LEN ADAPTIVESEC_CONF_UNICAST_MIC_LEN
#else /* ADAPTIVESEC_CONF_UNICAST_MIC_LEN */
#define ADAPTIVESEC_UNICAST_MIC_LEN LLSEC802154_MIC_LEN(ADAPTIVESEC_UNICAST_SEC_LVL)
#endif /* ADAPTIVESEC_CONF_UNICAST_MIC_LEN */

#ifdef ADAPTIVESEC_CONF_BROADCAST_SEC_LVL
#define ADAPTIVESEC_BROADCAST_SEC_LVL ADAPTIVESEC_CONF_BROADCAST_SEC_LVL
#else /* ADAPTIVESEC_CONF_BROADCAST_SEC_LVL */
#define ADAPTIVESEC_BROADCAST_SEC_LVL ADAPTIVESEC_UNICAST_SEC_LVL
#endif /* ADAPTIVESEC_CONF_BROADCAST_SEC_LVL */

#ifdef ADAPTIVESEC_CONF_BROADCAST_MIC_LEN
#define ADAPTIVESEC_BROADCAST_MIC_LEN ADAPTIVESEC_CONF_BROADCAST_MIC_LEN
#else /* ADAPTIVESEC_CONF_BROADCAST_MIC_LEN */
#define ADAPTIVESEC_BROADCAST_MIC_LEN LLSEC802154_MIC_LEN(ADAPTIVESEC_BROADCAST_SEC_LVL)
#endif /* ADAPTIVESEC_CONF_BROADCAST_MIC_LEN */

#ifdef ADAPTIVESEC_CONF_STRATEGY
#define ADAPTIVESEC_STRATEGY ADAPTIVESEC_CONF_STRATEGY
#else /* ADAPTIVESEC_CONF_STRATEGY */
#define ADAPTIVESEC_STRATEGY noncoresec_strategy
#endif /* ADAPTIVESEC_CONF_STRATEGY */

enum adaptivesec_verify {
  ADAPTIVESEC_VERIFY_SUCCESS,
  ADAPTIVESEC_VERIFY_INAUTHENTIC,
  ADAPTIVESEC_VERIFY_REPLAYED
};

/**
 * Structure of a strategy regarding compromise resilience
 */
struct adaptivesec_strategy {

  /** Secures outgoing frames */
  void (* send)(mac_callback_t sent, void *ptr);

  /** Called when the frame was created */
  int (* on_frame_created)(void);

  /** 0 <-> Success */
  enum adaptivesec_verify (* verify)(struct akes_nbr *sender);

  uint8_t (* get_overhead)(void);

  /** Initializes */
  void (* init)(void);
};

extern const struct adaptivesec_kes ADAPTIVESEC_KES;
extern const struct adaptivesec_strategy ADAPTIVESEC_STRATEGY;
extern const struct llsec_driver adaptivesec_driver;
extern const struct framer adaptivesec_framer;
#if AKES_NBR_WITH_GROUP_KEYS
extern uint8_t adaptivesec_group_key[AES_128_KEY_LENGTH];
#endif /* AKES_NBR_WITH_GROUP_KEYS */

clock_time_t adaptivesec_random_clock_time(clock_time_t min, clock_time_t max);
uint8_t adaptivesec_get_cmd_id(void);
int adaptivesec_is_hello(void);
int adaptivesec_is_helloack(void);
int adaptivesec_is_ack(void);
uint8_t adaptivesec_get_sec_lvl(void);
void adaptivesec_add_security_header(struct akes_nbr *receiver);
uint8_t *adaptivesec_prepare_command(uint8_t cmd_id, const linkaddr_t *dest);
void adaptivesec_send_command_frame(void);
uint8_t adaptivesec_mic_len(void);
void adaptivesec_aead(uint8_t *key, int shall_encrypt, uint8_t *result, int forward);
int adaptivesec_verify(uint8_t *key);

#endif /* ADAPTIVESEC_H_ */
