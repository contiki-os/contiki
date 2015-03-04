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
 *         Publish-Subscribe 802.15.4 MAC commmand frames.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#ifndef CMD_BROKER_H
#define CMD_BROKER_H

#include "contiki.h"

enum cmd_broker_result {
  CMD_BROKER_UNCONSUMED = 0,
  CMD_BROKER_CONSUMED,
  CMD_BROKER_ERROR
};

struct cmd_broker_subscription {
  struct cmd_broker_subscription *next;
  /** Returns 0 <-> not consumed */
  enum cmd_broker_result (* on_command)(uint8_t cmd_id, uint8_t *payload);
};

/**
 * \brief Subscribe to commands.
 */
void cmd_broker_subscribe(struct cmd_broker_subscription *subscription);

/**
 * \brief Cancel subscription (if any).
 */
void cmd_broker_unsubscribe(struct cmd_broker_subscription *subscription);

/**
 * \brief Called by NETSTACK_LLSEC upon receiving a command.
 */
enum cmd_broker_result cmd_broker_publish(void);

/**
 * \brief Called by NETSTACK_LLSEC.
 */
void cmd_broker_init(void);

#endif /* CMD_BROKER_H */
