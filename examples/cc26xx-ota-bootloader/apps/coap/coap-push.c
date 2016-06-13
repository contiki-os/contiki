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
#include "contiki.h"

#include "er-coap-engine.h"
#include "coap-common.h"
#include "coap-push.h"

#define DEBUG 1
#include "net/ip/uip-debug.h"

extern void coap_blocking_request_callback(void *callback_data, void *response);

/*---------------------------------------------------------------------------*/
#define COAP_PUSH_INTERVAL 1

PROCESS(coap_push_process, "CoAP Push");

LIST(coap_push_binding);
/*---------------------------------------------------------------------------*/
list_t
coap_push_get_bindings(void)
{
  return coap_push_binding;
}
/*---------------------------------------------------------------------------*/
int
coap_push_add_binding(coap_binding_t * binding)
{
  list_add(coap_push_binding, binding);

  PRINTF("Activating %s to %s\n", binding->resource->url, binding->uri);

  binding->data.last_sent_time = clock_seconds();

  return 1;
}
/*---------------------------------------------------------------------------*/
int
coap_push_remove_binding(coap_binding_t * binding)
{
  list_remove(coap_push_binding, binding);

  PRINTF("Remove binding of %s to %s\n", binding->resource->url, binding->uri);

  return 1;
}
/*---------------------------------------------------------------------------*/
void
coap_push_update_binding(resource_t *event_resource, int value) {
  static coap_binding_t * binding = NULL;
  for(binding = (coap_binding_t *)list_head(coap_push_binding);
    binding; binding = binding->next) {
    if(binding->resource == event_resource) {
      binding->data.last_value = value;
      process_poll(&coap_push_process);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
#define COAP_BLOCKING_PUSH_STATE(request_state, ctx, server_addr, server_port, request, resource_handler) \
  PT_SPAWN(process_pt, &request_state.pt, \
           coap_blocking_push(&request_state, ev, \
                              ctx, server_addr, server_port, \
                              request, resource_handler) \
  );

#define COAP_BLOCKING_PUSH(ctx, server_addr, server_port, request, resource_handler) \
{ \
  static struct request_state_t request_state; \
  COAP_BLOCKING_PUSH_STATE(request_state, ctx, server_addr, server_port, request, resource_handler) \
}

PT_THREAD(coap_blocking_push(struct request_state_t *state, process_event_t ev,
                             context_t *ctx,
                             uip_ipaddr_t *remote_ipaddr, uint16_t remote_port,
                             coap_packet_t *request, restful_handler resource_handler)) {
  PT_BEGIN(&state->pt);

  static uint8_t block_error;
  static int32_t offset;
  state->block_num = 0;
  state->response = NULL;
  state->process = PROCESS_CURRENT();
  state->status = 0;

  block_error = 0;
  offset = 0;

  do {
    request->mid = coap_get_mid();
    if ((state->transaction = coap_new_transaction(request->mid, remote_ipaddr, remote_port)))
    {
      coap_set_transaction_context(state->transaction, ctx);
      state->transaction->callback = coap_blocking_request_callback;
      state->transaction->callback_data = state;
      request->mid = state->transaction->mid;

      resource_handler(NULL, request,
                       state->transaction->packet + COAP_MAX_HEADER_SIZE,
                       REST_MAX_CHUNK_SIZE, &offset);

      if (offset != -1) {
        PRINTF("Warning: push block transfer not yet implemented, offset : %ld\n", (long)offset);
      }
      state->transaction->packet_len = coap_serialize_message(request, state->transaction->packet);

      coap_send_transaction(state->transaction);

      PT_YIELD_UNTIL(&state->pt, ev == PROCESS_EVENT_POLL);

      if (!state->response)
      {
        PRINTF("Server not responding\n");
        state->status = 1;
        PT_EXIT(&state->pt);
      }
    }
    else
    {
      PRINTF("Could not allocate transaction buffer\n");
      state->status = 1;
      PT_EXIT(&state->pt);
    }
    break;
  } while (block_error<COAP_MAX_ATTEMPTS);

  state->status = block_error >= COAP_MAX_ATTEMPTS;

  PT_END(&state->pt);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_push_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  PRINTF("CoAP push started\n");
  etimer_set(&et, COAP_PUSH_INTERVAL * CLOCK_SECOND);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et) || ev == PROCESS_EVENT_POLL) {
      static coap_binding_t * binding = NULL;
      for(binding = (coap_binding_t *)list_head(coap_push_binding);
          binding; binding = binding->next) {
        if(coap_binding_trigger_cond(&binding->cond, &binding->data)) {
          binding->data.last_sent_time = clock_seconds();
          static coap_packet_t request[1];
          PRINTF("Pushing %s to %s\n", binding->resource->url, binding->uri);
          coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
          coap_set_header_uri_path(request, binding->uri);
          COAP_BLOCKING_PUSH(coap_default_context, &binding->dest_addr, UIP_HTONS(binding->dest_port), request, binding->resource->get_handler);
        }
      }
      etimer_set(&et, COAP_PUSH_INTERVAL * CLOCK_SECOND);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
coap_push_init(void)
{
  list_init(coap_push_binding);
  process_start(&coap_push_process, NULL);
}
/*---------------------------------------------------------------------------*/
