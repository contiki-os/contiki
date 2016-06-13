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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-net.h"

#include "block-transfer.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

extern void coap_blocking_request_callback(void *callback_data, void *response);

PT_THREAD(coap_blocking_request_block(struct request_state_t *state, process_event_t ev,
                                context_t *ctx,
                                uip_ipaddr_t *remote_ipaddr, uint16_t remote_port,
                                coap_packet_t *request, block_handler request_callback,
                                blocking_response_handler response_callback)) {
  PT_BEGIN(&state->pt);

  static uint8_t more;
  static uint8_t more_server;
  static uint32_t res_block;
  static uint8_t block_error;
  static int32_t offset;

  state->block_num = 0;
  state->response = NULL;
  state->process = PROCESS_CURRENT();
  state->status = 0;

  more = 0;
  res_block = 0;
  block_error = 0;

  do {
    request->mid = coap_get_mid();
    if ((state->transaction = coap_new_transaction(request->mid, remote_ipaddr, remote_port)))
    {
      coap_set_transaction_context(state->transaction, ctx);
      state->transaction->callback = coap_blocking_request_callback;
      state->transaction->callback_data = state;

      offset = state->block_num*REST_MAX_CHUNK_SIZE;
      request_callback((void *)request, (void *)state->response, state->transaction->packet+COAP_MAX_HEADER_SIZE, REST_MAX_CHUNK_SIZE, &offset);
      more = offset != -1;
      coap_set_header_block1(request, state->block_num, more, REST_MAX_CHUNK_SIZE);

      state->transaction->packet_len = coap_serialize_message(request, state->transaction->packet);

      coap_send_transaction(state->transaction);
      PRINTF("Send block #%lu%s (MID %u)\n", state->block_num,  more ? "+" : "", request->mid);

      PT_YIELD_UNTIL(&state->pt, ev == PROCESS_EVENT_POLL);

      if (!state->response)
      {
        PRINTF("Server not responding\n");
        state->status = 1;
        PT_EXIT(&state->pt);
      }

      coap_get_header_block1(state->response, &res_block, &more_server, NULL, NULL);

      PRINTF("Acked #%lu%s\n", res_block, more_server ? "+" : "");

      if (res_block==state->block_num)
      {
        if(response_callback != NULL) {
          response_callback(state->response);
        }
        ++(state->block_num);
      }
      else
      {
        PRINTF("WRONG BLOCK %lu/%lu\n", res_block, state->block_num);
        ++block_error;
      }
    }
    else
    {
      PRINTF("Could not allocate transaction buffer");
      state->status = 1;
      PT_EXIT(&state->pt);
    }
  } while (more && block_error<COAP_MAX_ATTEMPTS);

  state->status = more ? 1 : 0;

  PT_END(&state->pt);
}
