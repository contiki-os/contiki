/*
 * Copyright (c) 2014, CETIC.
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
 */

/**
 * \file
 *         Simple CoAP Library
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */
#ifndef COAP_PUSH_H_
#define COAP_PUSH_H_

#include "contiki.h"
#include "coap-binding.h"

#ifdef COAP_PUSH_CONF_ENABLED
#define COAP_PUSH_ENABLED COAP_PUSH_CONF_ENABLED
#else
#define COAP_PUSH_ENABLED 1
#endif

#define COAP_PUSH_CONF_DEFAULT_PMIN 10
#define COAP_PUSH_CONF_DEFAULT_PMAX 0

#define COAP_BINDING(name, resource_name) \
  extern resource_t resource_##resource_name; \
  coap_binding_t binding_##name = { NULL, &resource_##resource_name, {}, COAP_DEFAULT_PORT, {}, 0, COAP_PUSH_CONF_DEFAULT_PMIN, COAP_PUSH_CONF_DEFAULT_PMAX };

void
coap_push_init();

list_t
coap_push_get_bindings(void);

int
coap_push_add_binding(coap_binding_t * binding);

int
coap_push_remove_binding(coap_binding_t * binding);

void
coap_push_update_binding(resource_t *event_resource, int value);

#endif /* COAP_PUSH_H_ */
