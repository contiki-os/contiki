/*
 * Copyright (c) 2011, Institute for Pervasive Computing, ETH Zurich
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
 */

/**
 * \file
 *      CoAP module for observing resources
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef COAP_OBSERVING_H_
#define COAP_OBSERVING_H_

#include "er-coap-03.h"
#include "er-coap-03-transactions.h"

#ifndef COAP_MAX_OBSERVERS
#define COAP_MAX_OBSERVERS      4
#endif /* COAP_MAX_OBSERVERS */

/* Interval in seconds in which NON notifies are changed to CON notifies to check client. */
#define COAP_OBSERVING_REFRESH_INTERVAL  60

#if COAP_MAX_OPEN_TRANSACTIONS<COAP_MAX_OBSERVERS
#warning "COAP_MAX_OPEN_TRANSACTIONS smaller than COAP_MAX_OBSERVERS: cannot handle CON notifications"
#endif

typedef struct coap_observer {
  struct coap_observer *next; /* for LIST */

  const char *url;
  uip_ipaddr_t addr;
  uint16_t port;
  uint8_t token_len;
  uint8_t token[COAP_TOKEN_LEN];
  struct stimer refresh_timer;
} coap_observer_t;

list_t coap_get_observers(void);
coap_observer_t *coap_add_observer(uip_ipaddr_t *addr, uint16_t port, const uint8_t *token, size_t token_len, const char *url);
void coap_remove_observer(coap_observer_t *o);
int coap_remove_observer_by_client(uip_ipaddr_t *addr, uint16_t port);
int coap_remove_observer_by_token(uip_ipaddr_t *addr, uint16_t port, uint8_t *token, size_t token_len);
void coap_notify_observers(const char *url, int type, uint32_t observe, uint8_t *payload, size_t payload_len);

void coap_observe_handler(resource_t *resource, void *request, void *response);

#endif /* COAP_OBSERVING_H_ */
