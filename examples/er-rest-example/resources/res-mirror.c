/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

static void res_any_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* This resource mirrors the incoming request. It shows how to access the options and how to set them for the response. */
RESOURCE(res_mirror,
         "title=\"Returns your decoded message\";rt=\"Debug\"",
         res_any_handler,
         res_any_handler,
         res_any_handler,
         res_any_handler);

static void
res_any_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* The ETag and Token is copied to the header. */
  uint8_t opaque[] = { 0x0A, 0xBC, 0xDE };

  /* Strings are not copied, so use static string buffers or strings in .text memory (char *str = "string in .text";). */
  static char location[] = { '/', 'f', '/', 'a', '?', 'k', '&', 'e', 0 };

  /* No default my be assumed for the Content-Format. (Unsigned -1 means all bits set.) */
  unsigned int content_format = -1;

  /* The other getters copy the value (or string/array pointer) to the given pointers and return 1 for success or the length of strings/arrays. */
  uint32_t longint = 0;
  const char *str = NULL;
  const uint8_t *bytes = NULL;
  uint32_t block_num = 0;
  uint8_t block_more = 0;
  uint16_t block_size = 0;
  int len = 0;

  /* Mirror the received header options in the response payload. Unsupported getters (e.g., rest_get_header_observe() with HTTP) will return 0. */

  int strpos = 0;
  /* snprintf() counts the terminating '\0' to the size parameter.
   * The additional byte is taken care of by allocating REST_MAX_CHUNK_SIZE+1 bytes in the REST framework.
   * Add +1 to fill the complete buffer, as the payload does not need a terminating '\0'. */
  if(REST.get_header_content_type(request, &content_format)) {
    strpos += snprintf((char *)buffer, REST_MAX_CHUNK_SIZE + 1, "CF %u\n", content_format);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = REST.get_header_accept(request, &content_format))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "Ac %u\n", content_format);
    /* Some getters such as for ETag or Location are omitted, as these options should not appear in a request.
     * Max-Age might appear in HTTP requests or used for special purposes in CoAP. */
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && REST.get_header_max_age(request, &longint)) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "MA %lu\n", longint);
    /* For HTTP this is the Length option, for CoAP it is the Size option. */
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && REST.get_header_length(request, &longint)) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "SZ %lu\n", longint);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = REST.get_header_host(request, &str))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "UH %.*s\n", len, str);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = REST.get_url(request, &str))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "UP %.*s\n", len, str);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = REST.get_query(request, &str))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "UQ %.*s\n", len, str);
    /* Undefined request options for debugging: actions not required for normal RESTful Web service. */
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_path(request, &str))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "LP %.*s\n", len, str);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_query(request, &str))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "LQ %.*s\n", len, str);
    /* CoAP-specific example: actions not required for normal RESTful Web service. */
  }
  coap_packet_t *const coap_pkt = (coap_packet_t *)request;

  if(strpos <= REST_MAX_CHUNK_SIZE && coap_pkt->token_len > 0) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "To 0x");
    int index = 0;
    for(index = 0; index < coap_pkt->token_len; ++index) {
      strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "%02X", coap_pkt->token[index]);
    }
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "\n");
  }

  if(strpos <= REST_MAX_CHUNK_SIZE && IS_OPTION(coap_pkt, COAP_OPTION_OBSERVE)) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "Ob %lu\n", coap_pkt->observe);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && IS_OPTION(coap_pkt, COAP_OPTION_ETAG)) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "ET 0x");
    int index = 0;
    for(index = 0; index < coap_pkt->etag_len; ++index) {
      strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "%02X", coap_pkt->etag[index]);
    }
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "\n");
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && coap_get_header_block2(request, &block_num, &block_more, &block_size, NULL)) { /* This getter allows NULL pointers to get only a subset of the block parameters. */
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "B2 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && coap_get_header_block1(request, &block_num, &block_more, &block_size, NULL)) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "B1 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
  }
  if(strpos <= REST_MAX_CHUNK_SIZE && (len = REST.get_request_payload(request, &bytes))) {
    strpos += snprintf((char *)buffer + strpos, REST_MAX_CHUNK_SIZE - strpos + 1, "%.*s", len, bytes);
  }
  if(strpos >= REST_MAX_CHUNK_SIZE) {
    buffer[REST_MAX_CHUNK_SIZE - 1] = 0xBB; /* '»' to indicate truncation */
  }
  REST.set_response_payload(response, buffer, strpos);

  PRINTF("/mirror options received: %s\n", buffer);

  /* Set dummy header options for response. Like getters, some setters are not implemented for HTTP and have no effect. */
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_max_age(response, 17); /* For HTTP, browsers will not re-request the page for 17 seconds. */
  REST.set_header_etag(response, opaque, 2);
  REST.set_header_location(response, location); /* Initial slash is omitted by framework */
  REST.set_header_length(response, strpos); /* For HTTP, browsers will not re-request the page for 10 seconds. CoAP action depends on the client. */

/* CoAP-specific example: actions not required for normal RESTful Web service. */
  coap_set_header_uri_host(response, "tiki");
  coap_set_header_observe(response, 10);
  coap_set_header_proxy_uri(response, "ftp://x");
  coap_set_header_block2(response, 42, 0, 64); /* The block option might be overwritten by the framework when blockwise transfer is requested. */
  coap_set_header_block1(response, 23, 0, 16);
  coap_set_header_accept(response, TEXT_PLAIN);
  coap_set_header_if_none_match(response);
}
