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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "er-coap-03-engine.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#define PRINTBITS(buf,len) { \
      int i,j=0; \
      for (i=0; i<len; ++i) { \
        for (j=7; j>=0; --j) { \
          PRINTF("%c", (((char *)buf)[i] & 1<<j) ? '1' : '0'); \
        } \
        PRINTF(" "); \
      } \
    }
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#define PRINTBITS(buf,len)
#endif

PROCESS(coap_receiver, "CoAP Receiver");

/*-----------------------------------------------------------------------------------*/
/*- Constants -----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
const char* error_messages[] = {
  "",

  /* Memory errors */
  "Transaction buffer allocation failed",
  "Memory boundary exceeded",

  /* CoAP errors */
  "Request has unknown critical option", /*FIXME which one? */
  "Packet could not be serialized"
};
/*-----------------------------------------------------------------------------------*/
/*- Variables -----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
static service_callback_t service_cbk = NULL;
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
static
int
handle_incoming_data(void)
{
  int error = NO_ERROR;

  PRINTF("handle_incoming_data(): received uip_datalen=%u \n",(uint16_t)uip_datalen());

  if (uip_newdata()) {

    PRINTF("receiving UDP datagram from: ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(":%u\n  Length: %u\n  Data: ", uip_ntohs(UIP_UDP_BUF->srcport), uip_datalen() );
    PRINTBITS(uip_appdata, uip_datalen());
    PRINTF("\n");

    coap_packet_t message[1];
    coap_transaction_t *transaction = NULL;

    error = coap_parse_message(message, uip_appdata, uip_datalen());

    if (error==NO_ERROR)
    {

      /*TODO duplicates suppression, if required */

      PRINTF("  Parsed: v %u, t %u, oc %u, c %u, tid %u\n", message->version, message->type, message->option_count, message->code, message->tid);
      PRINTF("  URL: %.*s\n", message->uri_path_len, message->uri_path);
      PRINTF("  Payload: %.*s\n", message->payload_len, message->payload);

      /* Handle requests. */
      if (message->code >= COAP_GET && message->code <= COAP_DELETE)
      {
        /* Use transaction buffer for response to confirmable request. */
        if ( (transaction = coap_new_transaction(message->tid, &UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport)) )
        {
            uint32_t block_num = 0;
            uint16_t block_size = REST_MAX_CHUNK_SIZE;
            uint32_t block_offset = 0;
            int32_t new_offset = 0;

            /* prepare response */
            coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */
            if (message->type==COAP_TYPE_CON)
            {
              /* Reliable CON requests are answered with an ACK. */
              coap_init_message(response, COAP_TYPE_ACK, OK_200, message->tid);
            }
            else
            {
              /* Unreliable NON requests are answered with a NON as well. */
              coap_init_message(response, COAP_TYPE_NON, OK_200, coap_get_tid());
            }

            /* resource handlers must take care of different handling (e.g., TOKEN_OPTION_REQUIRED_240) */
            if (IS_OPTION(message, COAP_OPTION_TOKEN))
            {
                coap_set_header_token(response, message->token, message->token_len);
                SET_OPTION(response, COAP_OPTION_TOKEN);
            }

            /* get offset for blockwise transfers */
            if (coap_get_header_block(message, &block_num, NULL, &block_size, &block_offset))
            {
                PRINTF("Blockwise: block request %lu (%u/%u) @ %lu bytes\n", block_num, block_size, REST_MAX_CHUNK_SIZE, block_offset);
                block_size = MIN(block_size, REST_MAX_CHUNK_SIZE);
                new_offset = block_offset;
            }

            /*------------------------------------------*/
            /* call application-specific handler        */
            /*------------------------------------------*/
            if (service_cbk) {
              service_cbk(message, response, transaction->packet+COAP_MAX_HEADER_SIZE, block_size, &new_offset);
            }
            /*------------------------------------------*/


            /* apply blockwise transfers */
            if ( IS_OPTION(message, COAP_OPTION_BLOCK) )
            {
              /* unchanged new_offset indicates that resource is unaware of blockwise transfer */
              if (new_offset==block_offset)
              {
                PRINTF("Blockwise: unaware resource with payload length %u/%u\n", response->payload_len, block_size);
                if (block_offset >= response->payload_len)
                {
                  response->code = BAD_REQUEST_400;
                  coap_set_payload(response, (uint8_t*)"Block out of scope", 18);
                }
                else
                {
                  coap_set_header_block(response, block_num, response->payload_len - block_offset > block_size, block_size);
                  coap_set_payload(response, response->payload+block_offset, MIN(response->payload_len - block_offset, block_size));
                } /* if (valid offset) */
              }
              else
              {
                /* resource provides chunk-wise data */
                PRINTF("Blockwise: blockwise resource, new offset %ld\n", new_offset);
                coap_set_header_block(response, block_num, new_offset!=-1 || response->payload_len > block_size, block_size);
                if (response->payload_len > block_size) coap_set_payload(response, response->payload, block_size);
              } /* if (resource aware of blockwise) */
            }
            else if (new_offset!=0)
            {
              PRINTF("Blockwise: no block option for blockwise resource, using block size %u\n", REST_MAX_CHUNK_SIZE);

              coap_set_header_block(response, 0, new_offset!=-1, REST_MAX_CHUNK_SIZE);
              coap_set_payload(response, response->payload, MIN(response->payload_len, REST_MAX_CHUNK_SIZE));
            } /* if (blockwise request) */

            if ((transaction->packet_len = coap_serialize_message(response, transaction->packet))==0)
            {
              error = PACKET_SERIALIZATION_ERROR;
            }

        } else {
            error = MEMORY_ALLOCATION_ERROR;
        }
      }
      else
      {
        /* Responses */
        coap_transaction_t *t;

        if (message->type==COAP_TYPE_ACK)
        {
          PRINTF("Received ACK\n");
        }
        else if (message->type==COAP_TYPE_RST)
        {
          PRINTF("Received RST\n");
          /* Cancel possible subscriptions. */
          if (IS_OPTION(message, COAP_OPTION_TOKEN))
          {
            PRINTF("  Token 0x%02X%02X\n", message->token[0], message->token[1]);
            coap_remove_observer_by_token(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport, message->token, message->token_len);
          }
        }

        if ( (t = coap_get_transaction_by_tid(message->tid)) )
        {
          /* Free transaction memory before callback, as it may create a new transaction. */
          restful_response_handler callback = t->callback;
          void *callback_data = t->callback_data;
          coap_clear_transaction(t);

          /* Check if someone registered for the response */
          if (callback) {
            callback(callback_data, message);
          }
        } /* if (transaction) */
      }
    } /* if (parsed correctly) */

    if (error==NO_ERROR) {
      if (transaction) coap_send_transaction(transaction);
    }
    else
    {
      PRINTF("ERROR %u: %s\n", error, error_messages[error]);

      /* reuse input buffer */
      coap_init_message(message, COAP_TYPE_ACK, INTERNAL_SERVER_ERROR_500, message->tid);
      coap_set_payload(message, (uint8_t *) error_messages[error], strlen(error_messages[error]));
      coap_send_message(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport, uip_appdata, coap_serialize_message(message, uip_appdata));
    }
  } /* if (new data) */

  return error;
}
/*-----------------------------------------------------------------------------------*/
void
coap_receiver_init()
{
  process_start(&coap_receiver, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
coap_set_service_callback(service_callback_t callback)
{
  service_cbk = callback;
}
/*-----------------------------------------------------------------------------------*/
rest_resource_flags_t
coap_get_rest_method(void *packet)
{
  return (rest_resource_flags_t)(1 << (((coap_packet_t *)packet)->code - 1));
}
/*-----------------------------------------------------------------------------------*/
int
coap_set_rest_status(void *packet, unsigned int code)
{
  if (code <= 0xFF)
  {
    ((coap_packet_t *)packet)->code = (uint8_t) code;
    return 1;
  }
  else
  {
    return 0;
  }
}
/*-----------------------------------------------------------------------------------*/
/*- Server part ---------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/* The discover resource should be included when using CoAP. */
RESOURCE(well_known_core, METHOD_GET, ".well-known/core", "");
void
well_known_core_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* Response might be NULL for non-confirmable requests. */
  if (response)
  {
    size_t strpos = 0;
    size_t bufpos = 0;
    resource_t* resource = NULL;

    for (resource = (resource_t*)list_head(rest_get_resources()); resource; resource = resource->next)
    {
      strpos += snprintf((char *) buffer + bufpos, REST_MAX_CHUNK_SIZE - bufpos + 1,
                         "</%s>%s%s%s",
                         resource->url,
                         resource->attributes[0] ? ";" : "",
                         resource->attributes,
                         resource->next ? "," : "" );

      PRINTF("discover: %s\n", resource->url);

      if (strpos <= *offset)
      {
        /* Discard output before current block */
        PRINTF("  if %d <= %ld B\n", strpos, *offset);
        PRINTF("  %s\n", buffer);
        bufpos = 0;
      }
      else /* (strpos > *offset) */
      {
        /* output partly in block */
        size_t len = MIN(strpos - *offset, preferred_size);

        PRINTF("  el %d/%d @ %ld B\n", len, preferred_size, *offset);

        /* Block might start in the middle of the output; align with buffer start. */
        if (bufpos == 0)
        {
          memmove(buffer, buffer+strlen((char *)buffer)-strpos+*offset, len);
        }

        bufpos = len;
        PRINTF("  %s\n", buffer);

        if (bufpos >= preferred_size)
        {
          break;
        }
      }
    }

    if (bufpos>0) {
      coap_set_payload(response, buffer, bufpos );
      coap_set_header_content_type(response, APPLICATION_LINK_FORMAT);
    }
    else
    {
      coap_set_rest_status(response, BAD_REQUEST_400);
      coap_set_payload(response, (uint8_t*)"Block out of scope", 18);
    }

    if (resource==NULL) {
      *offset = -1;
    }
    else
    {
      *offset += bufpos;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(coap_receiver, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Starting CoAP-03 receiver...\n");

  rest_activate_resource(&resource_well_known_core);

  coap_register_as_transaction_handler();
  coap_init_connection(SERVER_LISTEN_PORT);

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      handle_incoming_data();
    } else if (ev == PROCESS_EVENT_TIMER) {
      /* retransmissions are handled here */
      coap_check_transactions();
    }
  } /* while (1) */

  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
/*- Client part ---------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
void blocking_request_callback(void *callback_data, void *response) {
  struct request_state_t *state = (struct request_state_t *) callback_data;
  state->response = (coap_packet_t*) response;
  process_poll(state->process);
}
/*-----------------------------------------------------------------------------------*/
PT_THREAD(coap_blocking_request(struct request_state_t *state, process_event_t ev,
                                uip_ipaddr_t *remote_ipaddr, uint16_t remote_port,
                                coap_packet_t *request,
                                blocking_response_handler request_callback)) {
  PT_BEGIN(&state->pt);

  static uint8_t more;
  static uint32_t res_block;
  static uint8_t block_error;

  state->block_num = 0;
  state->response = NULL;
  state->process = PROCESS_CURRENT();

  more = 0;
  res_block = 0;
  block_error = 0;

  do {
    request->tid = coap_get_tid();
    if ((state->transaction = coap_new_transaction(request->tid, remote_ipaddr, remote_port)))
    {
      state->transaction->callback = blocking_request_callback;
      state->transaction->callback_data = state;

      if (state->block_num>0)
      {
        coap_set_header_block(request, state->block_num, 0, REST_MAX_CHUNK_SIZE);
      }

      state->transaction->packet_len = coap_serialize_message(request, state->transaction->packet);

      coap_send_transaction(state->transaction);
      PRINTF("Requested #%lu (TID %u)\n", state->block_num, request->tid);

      PT_YIELD_UNTIL(&state->pt, ev == PROCESS_EVENT_POLL);

      if (!state->response)
      {
        PRINTF("Server not responding\n");
        PT_EXIT(&state->pt);
      }

      coap_get_header_block(state->response, &res_block, &more, NULL, NULL);

      PRINTF("Received #%lu%s (%u bytes)\n", res_block, more ? "+" : "", state->response->payload_len);

      if (res_block==state->block_num)
      {
        request_callback(state->response);
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
      PT_EXIT(&state->pt);
    }
  } while (more && block_error<COAP_MAX_ATTEMPTS);

  PT_END(&state->pt);
}
/*-----------------------------------------------------------------------------------*/
/*- Engine Interface ----------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
const struct rest_implementation coap_rest_implementation = {
    "CoAP-03",

    coap_receiver_init,
    coap_set_service_callback,

    coap_get_header_uri_path,
    coap_set_header_uri_path,
    coap_get_rest_method,
    coap_set_rest_status,

    coap_get_header_content_type,
    coap_set_header_content_type,
    NULL,
    NULL,
    NULL,
    coap_get_header_max_age,
    coap_set_header_max_age,
    coap_set_header_etag,
    NULL,
    NULL,
    coap_get_header_uri_host,
    coap_set_header_location,

    coap_get_payload,
    coap_set_payload,

    coap_get_header_uri_query,
    coap_get_query_variable,
    coap_get_post_variable,

    coap_notify_observers,
    (restful_post_handler) coap_observe_handler,

    NULL,
    NULL,

    {
      OK_200,
      CREATED_201,
      OK_200,
      OK_200,
      NOT_MODIFIED_304,

      BAD_REQUEST_400,
      METHOD_NOT_ALLOWED_405,
      BAD_REQUEST_400,
      METHOD_NOT_ALLOWED_405,
      NOT_FOUND_404,
      METHOD_NOT_ALLOWED_405,
      UNSUPPORTED_MEDIA_TYPE_415,
      BAD_REQUEST_400,
      UNSUPPORTED_MEDIA_TYPE_415,

      INTERNAL_SERVER_ERROR_500,
      CRITICAL_OPTION_NOT_SUPPORTED,
      BAD_GATEWAY_502,
      SERVICE_UNAVAILABLE_503,
      GATEWAY_TIMEOUT_504,
      INTERNAL_SERVER_ERROR_500
    },

    {
      TEXT_PLAIN,
      TEXT_XML,
      TEXT_CSV,
      TEXT_HTML,
      IMAGE_GIF,
      IMAGE_JPEG,
      IMAGE_PNG,
      IMAGE_TIFF,
      AUDIO_RAW,
      VIDEO_RAW,
      APPLICATION_LINK_FORMAT,
      APPLICATION_XML,
      APPLICATION_OCTET_STREAM,
      APPLICATION_RDF_XML,
      APPLICATION_SOAP_XML,
      APPLICATION_ATOM_XML,
      APPLICATION_XMPP_XML,
      APPLICATION_EXI,
      APPLICATION_FASTINFOSET,
      APPLICATION_SOAP_FASTINFOSET,
      APPLICATION_JSON,
      APPLICATION_OCTET_STREAM
    }
};
