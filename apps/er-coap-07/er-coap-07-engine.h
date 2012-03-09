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
 *      CoAP implementation of the REST Engine
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef COAP_SERVER_H_
#define COAP_SERVER_H_

#if !defined(REST)
#error "Define REST to \"coap_rest_implementation\""
#endif

#include "er-coap-07.h"
#include "er-coap-07-transactions.h"
#include "er-coap-07-observing.h"
#include "er-coap-07-separate.h"

#include "pt.h"

/* Declare server process */
PROCESS_NAME(coap_receiver);

#define SERVER_LISTEN_PORT      UIP_HTONS(COAP_SERVER_PORT)

typedef coap_packet_t rest_request_t;
typedef coap_packet_t rest_response_t;

extern const struct rest_implementation coap_rest_implementation;

void coap_receiver_init(void);

/*-----------------------------------------------------------------------------------*/
/*- Client part ---------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
struct request_state_t {
    struct pt pt;
    struct process *process;
    coap_transaction_t *transaction;
    coap_packet_t *response;
    uint32_t block_num;
};

typedef void (*blocking_response_handler) (void* response);

PT_THREAD(coap_blocking_request(struct request_state_t *state, process_event_t ev,
                                uip_ipaddr_t *remote_ipaddr, uint16_t remote_port,
                                coap_packet_t *request,
                                blocking_response_handler request_callback));

#define COAP_BLOCKING_REQUEST(server_addr, server_port, request, chunk_handler) \
{ \
  static struct request_state_t request_state; \
  PT_SPAWN(process_pt, &request_state.pt, \
           coap_blocking_request(&request_state, ev, \
                                 server_addr, server_port, \
                                 request, chunk_handler) \
  ); \
}
/*-----------------------------------------------------------------------------------*/

#endif /* COAP_SERVER_H_ */
