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
 *         Block-transfer utilities
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */
#ifndef BLOCK_TRANSFER_H_
#define BLOCK_TRANSFER_H_

#include "er-coap-engine.h"

typedef void(*block_handler)(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

PT_THREAD(coap_blocking_request_block(struct request_state_t *state, process_event_t ev,
                                context_t *ctx,
                                uip_ipaddr_t *remote_ipaddr, uint16_t remote_port,
                                coap_packet_t *request, block_handler request_callback,
                                blocking_response_handler response_callback));

#define COAP_BLOCKING_REQUEST_BLOCK_RESPONSE(server_addr, server_port, request, request_callback, response_callback) \
{ \
  static struct request_state_t request_state; \
  PT_SPAWN(process_pt, &request_state.pt, \
           coap_blocking_request_block(&request_state, ev, \
                                 coap_default_context, server_addr, server_port, \
                                 request, request_callback, response_callback) \
  ); \
}

#define COAP_BLOCKING_REQUEST_BLOCK_RESPONSE_CONTEXT(ctx, server_addr, server_port, request, request_callback, response_callback) \
{ \
  static struct request_state_t request_state; \
  PT_SPAWN(process_pt, &request_state.pt, \
           coap_blocking_request_block(&request_state, ev, \
                                 ctx, server_addr, server_port, \
                                 request, request_callback, response_callback) \
  ); \
}

#define COAP_BLOCKING_REQUEST_BLOCK_STATE(request_state, ctx, server_addr, server_port, request, request_callback) \
  PT_SPAWN(process_pt, &request_state.pt, \
           coap_blocking_request_block(&request_state, ev, \
                                 ctx, server_addr, server_port, \
                                 request, request_callback, NULL) \
  );

#define COAP_BLOCKING_REQUEST_BLOCK(ctx, server_addr, server_port, request, request_callback) \
{ \
  static struct request_state_t request_state; \
  COAP_BLOCKING_REQUEST_BLOCK_STATE(request_state, ctx, server_addr, server_port, request, request_callback, NULL) \
}

#endif
