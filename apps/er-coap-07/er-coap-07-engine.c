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

#include "er-coap-07-engine.h"

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

/*----------------------------------------------------------------------------*/
/*- Variables ----------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static service_callback_t service_cbk = NULL;
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static
int
coap_receive(void)
{
  coap_error_code = NO_ERROR;

  PRINTF("handle_incoming_data(): received uip_datalen=%u \n",(uint16_t)uip_datalen());

  /* Static declaration reduces stack peaks and program code size. */
  static coap_packet_t message[1]; /* This way the packet can be treated as pointer as usual. */
  static coap_packet_t response[1];
  static coap_transaction_t *transaction = NULL;

  if (uip_newdata()) {

    PRINTF("receiving UDP datagram from: ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(":%u\n  Length: %u\n  Data: ", uip_ntohs(UIP_UDP_BUF->srcport), uip_datalen() );
    PRINTBITS(uip_appdata, uip_datalen());
    PRINTF("\n");

    coap_error_code = coap_parse_message(message, uip_appdata, uip_datalen());

    if (coap_error_code==NO_ERROR)
    {

      /*TODO duplicates suppression, if required by application */

      PRINTF("  Parsed: v %u, t %u, oc %u, c %u, mid %u\n", message->version, message->type, message->option_count, message->code, message->mid);
      PRINTF("  URL: %.*s\n", message->uri_path_len, message->uri_path);
      PRINTF("  Payload: %.*s\n", message->payload_len, message->payload);

      /* Handle requests. */
      if (message->code >= COAP_GET && message->code <= COAP_DELETE)
      {
        /* Use transaction buffer for response to confirmable request. */
        if ( (transaction = coap_new_transaction(message->mid, &UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport)) )
        {
          uint32_t block_num = 0;
          uint16_t block_size = REST_MAX_CHUNK_SIZE;
          uint32_t block_offset = 0;
          int32_t new_offset = 0;

          /* prepare response */
          if (message->type==COAP_TYPE_CON)
          {
            /* Reliable CON requests are answered with an ACK. */
            coap_init_message(response, COAP_TYPE_ACK, CONTENT_2_05, message->mid);
          }
          else
          {
            /* Unreliable NON requests are answered with a NON as well. */
            coap_init_message(response, COAP_TYPE_NON, CONTENT_2_05, coap_get_mid());
          }

          /* resource handlers must take care of different handling (e.g., TOKEN_OPTION_REQUIRED_240) */
          if (IS_OPTION(message, COAP_OPTION_TOKEN))
          {
              coap_set_header_token(response, message->token, message->token_len);
              SET_OPTION(response, COAP_OPTION_TOKEN);
          }

          /* get offset for blockwise transfers */
          if (coap_get_header_block2(message, &block_num, NULL, &block_size, &block_offset))
          {
              PRINTF("Blockwise: block request %lu (%u/%u) @ %lu bytes\n", block_num, block_size, REST_MAX_CHUNK_SIZE, block_offset);
              block_size = MIN(block_size, REST_MAX_CHUNK_SIZE);
              new_offset = block_offset;
          }

          /* Invoke resource handler. */
          if (service_cbk)
          {
            /* Call REST framework and check if found and allowed. */
            if (service_cbk(message, response, transaction->packet+COAP_MAX_HEADER_SIZE, block_size, &new_offset))
            {
              if (coap_error_code==NO_ERROR)
              {
                /* Apply blockwise transfers. */
                if ( IS_OPTION(message, COAP_OPTION_BLOCK1) && response->code<BAD_REQUEST_4_00 && !IS_OPTION(response, COAP_OPTION_BLOCK1) )
                {
                  PRINTF("Block1 NOT IMPLEMENTED\n");

                  coap_error_code = NOT_IMPLEMENTED_5_01;
                  coap_error_message = "NoBlock1Support";
                }
                else if ( IS_OPTION(message, COAP_OPTION_BLOCK2) )
                {
                  /* unchanged new_offset indicates that resource is unaware of blockwise transfer */
                  if (new_offset==block_offset)
                  {
                    PRINTF("Blockwise: unaware resource with payload length %u/%u\n", response->payload_len, block_size);
                    if (block_offset >= response->payload_len)
                    {
                      PRINTF("handle_incoming_data(): block_offset >= response->payload_len\n");

                      response->code = BAD_OPTION_4_02;
                      coap_set_payload(response, "BlockOutOfScope", 15); /* a const char str[] and sizeof(str) produces larger code size */
                    }
                    else
                    {
                      coap_set_header_block2(response, block_num, response->payload_len - block_offset > block_size, block_size);
                      coap_set_payload(response, response->payload+block_offset, MIN(response->payload_len - block_offset, block_size));
                    } /* if (valid offset) */
                  }
                  else
                  {
                    /* resource provides chunk-wise data */
                    PRINTF("Blockwise: blockwise resource, new offset %ld\n", new_offset);
                    coap_set_header_block2(response, block_num, new_offset!=-1 || response->payload_len > block_size, block_size);
                    if (response->payload_len > block_size) coap_set_payload(response, response->payload, block_size);
                  } /* if (resource aware of blockwise) */
                }
                else if (new_offset!=0)
                {
                  PRINTF("Blockwise: no block option for blockwise resource, using block size %u\n", REST_MAX_CHUNK_SIZE);

                  coap_set_header_block2(response, 0, new_offset!=-1, REST_MAX_CHUNK_SIZE);
                  coap_set_payload(response, response->payload, MIN(response->payload_len, REST_MAX_CHUNK_SIZE));
                } /* if (blockwise request) */
              } /* no errors/hooks */
            } /* successful service callback */

            /* Serialize response. */
            if (coap_error_code==NO_ERROR)
            {
              if ((transaction->packet_len = coap_serialize_message(response, transaction->packet))==0)
              {
                coap_error_code = PACKET_SERIALIZATION_ERROR;
              }
            }

          }
          else
          {
            coap_error_code = NOT_IMPLEMENTED_5_01;
            coap_error_message = "NoServiceCallbck"; // no a to fit 16 bytes
          } /* if (service callback) */

        } else {
            coap_error_code = SERVICE_UNAVAILABLE_5_03;
            coap_error_message = "NoFreeTraBuffer";
        } /* if (transaction buffer) */
      }
      else
      {
        /* Responses */

        if (message->type==COAP_TYPE_ACK)
        {
          PRINTF("Received ACK\n");
        }
        else if (message->type==COAP_TYPE_RST)
        {
          PRINTF("Received RST\n");
          /* Cancel possible subscriptions. */
          coap_remove_observer_by_mid(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport, message->mid);
        }

        if ( (transaction = coap_get_transaction_by_mid(message->mid)) )
        {
          /* Free transaction memory before callback, as it may create a new transaction. */
          restful_response_handler callback = transaction->callback;
          void *callback_data = transaction->callback_data;
          coap_clear_transaction(transaction);

          /* Check if someone registered for the response */
          if (callback) {
            callback(callback_data, message);
          }
        } /* if (ACKed transaction) */
        transaction = NULL;

      } /* Request or Response */

    } /* if (parsed correctly) */

    if (coap_error_code==NO_ERROR)
    {
      if (transaction) coap_send_transaction(transaction);
    }
    else if (coap_error_code==MANUAL_RESPONSE)
    {
      PRINTF("Clearing transaction for manual response");
      coap_clear_transaction(transaction);
    }
    else
    {
      PRINTF("ERROR %u: %s\n", coap_error_code, coap_error_message);
      coap_clear_transaction(transaction);

      /* Set to sendable error code. */
      if (coap_error_code >= 192)
      {
        coap_error_code = INTERNAL_SERVER_ERROR_5_00;
      }
      /* Reuse input buffer for error message. */
      coap_init_message(message, COAP_TYPE_ACK, coap_error_code, message->mid);
      coap_set_payload(message, coap_error_message, strlen(coap_error_message));
      coap_send_message(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport, uip_appdata, coap_serialize_message(message, uip_appdata));
    }
  } /* if (new data) */

  return coap_error_code;
}
/*----------------------------------------------------------------------------*/
void
coap_receiver_init()
{
  process_start(&coap_receiver, NULL);
}
/*----------------------------------------------------------------------------*/
void
coap_set_service_callback(service_callback_t callback)
{
  service_cbk = callback;
}
/*----------------------------------------------------------------------------*/
rest_resource_flags_t
coap_get_rest_method(void *packet)
{
  return (rest_resource_flags_t)(1 << (((coap_packet_t *)packet)->code - 1));
}
/*----------------------------------------------------------------------------*/
/*- Server part --------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/* The discover resource is automatically included for CoAP. */
RESOURCE(well_known_core, METHOD_GET, ".well-known/core", "ct=40");
void
well_known_core_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    size_t strpos = 0; /* position in overall string (which is larger than the buffer) */
    size_t bufpos = 0; /* position within buffer (bytes written) */
    size_t tmplen = 0;
    resource_t* resource = NULL;

    /* For filtering. */
    const char *filter = NULL;
    int len = coap_get_query_variable(request, "rt", &filter);
    char *rt = NULL;

    for (resource = (resource_t*)list_head(rest_get_resources()); resource; resource = resource->next)
    {
      /* Filtering */
      if (len && ((rt=strstr(resource->attributes, "rt=\""))==NULL || memcmp(rt+4, filter, len-1)!=0 || (filter[len-1]!='*' && (filter[len-1]!=rt[3+len] || rt[4+len]!='"'))))
      {
        continue;
      }

      PRINTF("res: /%s (%p)\npos: s%d, o%d, b%d\n", resource->url, resource, strpos, *offset, bufpos);

      if (strpos >= *offset && bufpos < preferred_size)
      {
        buffer[bufpos++] = '<';
      }
      ++strpos;

      if (strpos >= *offset && bufpos < preferred_size)
      {
        buffer[bufpos++] = '/';
      }
      ++strpos;

      tmplen = strlen(resource->url);
      if (strpos+tmplen > *offset)
      {
        bufpos += snprintf((char *) buffer + bufpos, preferred_size - bufpos + 1,
                         "%s", resource->url + ((*offset-(int32_t)strpos > 0) ? (*offset-(int32_t)strpos) : 0));
                                                          /* minimal-net requires these casts */
        if (bufpos >= preferred_size)
        {
          break;
        }
      }
      strpos += tmplen;

      if (strpos >= *offset && bufpos < preferred_size)
      {
        buffer[bufpos++] = '>';
      }
      ++strpos;

      if (resource->attributes[0])
      {
        if (strpos >= *offset && bufpos < preferred_size)
        {
          buffer[bufpos++] = ';';
        }
        ++strpos;

        tmplen = strlen(resource->attributes);
        if (strpos+tmplen > *offset)
        {
          bufpos += snprintf((char *) buffer + bufpos, preferred_size - bufpos + 1,
                         "%s", resource->attributes + (*offset-(int32_t)strpos > 0 ? *offset-(int32_t)strpos : 0));
          if (bufpos >= preferred_size)
          {
            break;
          }
        }
        strpos += tmplen;
      }

      if (resource->next)
      {
        if (strpos >= *offset && bufpos < preferred_size)
        {
          buffer[bufpos++] = ',';
        }
        ++strpos;
      }

      /* buffer full, but resource not completed yet; or: do not break if resource exactly fills buffer. */
      if (bufpos >= preferred_size && strpos-bufpos > *offset)
      {
        PRINTF("res: BREAK at %s (%p)\n", resource->url, resource);
        break;
      }
    }

    if (bufpos>0) {
      PRINTF("BUF %d: %.*s\n", bufpos, bufpos, (char *) buffer);

      coap_set_payload(response, buffer, bufpos );
      coap_set_header_content_type(response, APPLICATION_LINK_FORMAT);
    }
    else if (strpos>0)
    {
      PRINTF("well_known_core_handler(): bufpos<=0\n");

      coap_set_status_code(response, BAD_OPTION_4_02);
      coap_set_payload(response, "BlockOutOfScope", 15);
    }

    if (resource==NULL) {
      PRINTF("res: DONE\n");
      *offset = -1;
    }
    else
    {
      PRINTF("res: MORE at %s (%p)\n", resource->url, resource);
      *offset += preferred_size;
    }
}
/*----------------------------------------------------------------------------*/
PROCESS_THREAD(coap_receiver, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Starting CoAP-07 receiver...\n");

  rest_activate_resource(&resource_well_known_core);

  coap_register_as_transaction_handler();
  coap_init_connection(SERVER_LISTEN_PORT);
  PRINTF("Listening on port %u\n", UIP_HTONS(SERVER_LISTEN_PORT));

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      coap_receive();
    } else if (ev == PROCESS_EVENT_TIMER) {
      /* retransmissions are handled here */
      coap_check_transactions();
    }
  } /* while (1) */

  PROCESS_END();
}
/*----------------------------------------------------------------------------*/
/*- Client part --------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void coap_blocking_request_callback(void *callback_data, void *response) {
  struct request_state_t *state = (struct request_state_t *) callback_data;
  state->response = (coap_packet_t*) response;
  process_poll(state->process);
}
/*----------------------------------------------------------------------------*/
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
    request->mid = coap_get_mid();
    if ((state->transaction = coap_new_transaction(request->mid, remote_ipaddr, remote_port)))
    {
      state->transaction->callback = coap_blocking_request_callback;
      state->transaction->callback_data = state;

      if (state->block_num>0)
      {
        coap_set_header_block2(request, state->block_num, 0, REST_MAX_CHUNK_SIZE);
      }

      state->transaction->packet_len = coap_serialize_message(request, state->transaction->packet);

      coap_send_transaction(state->transaction);
      PRINTF("Requested #%lu (MID %u)\n", state->block_num, request->mid);

      PT_YIELD_UNTIL(&state->pt, ev == PROCESS_EVENT_POLL);

      if (!state->response)
      {
        PRINTF("Server not responding\n");
        PT_EXIT(&state->pt);
      }

      coap_get_header_block2(state->response, &res_block, &more, NULL, NULL);

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
/*----------------------------------------------------------------------------*/
/*- Engine Interface ---------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
const struct rest_implementation coap_rest_implementation = {
  "CoAP-07",

  coap_receiver_init,
  coap_set_service_callback,

  coap_get_header_uri_path,
  coap_set_header_uri_path,
  coap_get_rest_method,
  coap_set_status_code,

  coap_get_header_content_type,
  coap_set_header_content_type,
  coap_get_header_accept,
  coap_get_header_max_age,
  coap_set_header_max_age,
  coap_set_header_etag,
  coap_get_header_if_match,
  coap_get_header_if_none_match,
  coap_get_header_uri_host,
  coap_set_header_location_path,

  coap_get_payload,
  coap_set_payload,

  coap_get_header_uri_query,
  coap_get_query_variable,
  coap_get_post_variable,

  coap_notify_observers,
  (restful_post_handler) coap_observe_handler,

  NULL, /* default pre-handler (set separate handler after activation if needed) */
  NULL, /* default post-handler for non-observable resources */

  {
    CONTENT_2_05,
    CREATED_2_01,
    CHANGED_2_04,
    DELETED_2_02,
    VALID_2_03,
    BAD_REQUEST_4_00,
    UNAUTHORIZED_4_01,
    BAD_OPTION_4_02,
    FORBIDDEN_4_03,
    NOT_FOUND_4_04,
    METHOD_NOT_ALLOWED_4_05,
    REQUEST_ENTITY_TOO_LARGE_4_13,
    UNSUPPORTED_MADIA_TYPE_4_15,
    INTERNAL_SERVER_ERROR_5_00,
    NOT_IMPLEMENTED_5_01,
    BAD_GATEWAY_5_02,
    SERVICE_UNAVAILABLE_5_03,
    GATEWAY_TIMEOUT_5_04,
    PROXYING_NOT_SUPPORTED_5_05
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
    APPLICATION_X_OBIX_BINARY
  }
};
