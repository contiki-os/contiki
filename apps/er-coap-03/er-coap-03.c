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
 *      An implementation of the Constrained Application Protocol (draft 03)
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifdef CONTIKI_TARGET_NETSIM
  #include <stdio.h>
  #include <iostream>
  #include <cstring>
  #include <cstdlib>
  #include <unistd.h>
  #include <errno.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#else
  #include "contiki.h"
  #include "contiki-net.h"
  #include <string.h>
  #include <stdio.h>
#endif

#include "er-coap-03.h"
#include "er-coap-03-transactions.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


static struct uip_udp_conn *udp_conn = NULL;
static uint16_t current_tid = 0;

/*-----------------------------------------------------------------------------------*/
/*- LOCAL HELP FUNCTIONS ------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
static
uint16_t
log_2(uint16_t value)
{
  uint16_t result = 0;
  do {
    value = value >> 1;
    result++;
  } while (value);

  return result ? result - 1 : result;
}
/*-----------------------------------------------------------------------------------*/
static
uint32_t
bytes_2_uint32(uint8_t *bytes, uint16_t length)
{
  uint32_t var = 0;
  int i = 0;
  while (i<length)
  {
    var <<= 8;
    var |= 0xFF & bytes[i++];
  }
  return var;
}
/*-----------------------------------------------------------------------------------*/
/* Unused in coap-03.
static
int
uint16_2_bytes(uint8_t *bytes, uint16_t var)
{
  int i = 0;
  if (0xFF00 & var) bytes[i++] = var>>8;
  bytes[i++] = 0xFF & var;

  return i;
}
*/
/*-----------------------------------------------------------------------------------*/
static
int
uint32_2_bytes(uint8_t *bytes, uint32_t var)
{
  int i = 0;
  if (0xFF000000 & var) bytes[i++] = (0xFF & var>>24);
  if (0xFFFF0000 & var) bytes[i++] = (0xFF & var>>16);
  if (0xFFFFFF00 & var) bytes[i++] = (0xFF & var>>8);
  bytes[i++] = 0xFF & var;

  return i;
}
/*-----------------------------------------------------------------------------------*/
static
int
coap_get_variable(const char *buffer, size_t length, const char *name, const char **output)
{
  const char *start = NULL;
  const char *end = NULL;
  const char *value_end = NULL;
  size_t name_len = 0;

  /*initialize the output buffer first*/
  *output = 0;

  name_len = strlen(name);
  end = buffer + length;

  for (start = buffer; start + name_len < end; ++start){
    if ((start == buffer || start[-1] == '&') && start[name_len] == '=' &&
        strncmp(name, start, name_len)==0) {

      /* Point start to variable value */
      start += name_len + 1;

      /* Point end to the end of the value */
      value_end = (const char *) memchr(start, '&', end - start);
      if (value_end == NULL) {
        value_end = end;
      }

      *output = start;

      return (value_end - start);
    }
  }

  return 0;
}
/*-----------------------------------------------------------------------------------*/
/*- MEASSAGE SENDING ----------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
void
coap_init_connection(uint16_t port)
{
  /* new connection with remote host */
  udp_conn = udp_new(NULL, 0, NULL);
  udp_bind(udp_conn, port);
  PRINTF("Listening on port %u\n", uip_ntohs(udp_conn->lport));

  /* Initialize transaction ID. */
  current_tid = random_rand();
}
/*-----------------------------------------------------------------------------------*/
uint16_t
coap_get_tid()
{
  ++current_tid;
  PRINTF("Get TID %u\n", current_tid);
  return current_tid;
}
/*-----------------------------------------------------------------------------------*/
void
coap_send_message(uip_ipaddr_t *addr, uint16_t port, const uint8_t *data, uint16_t length)
{
  /*configure connection to reply to client*/
  uip_ipaddr_copy(&udp_conn->ripaddr, addr);
  udp_conn->rport = port;

  uip_udp_packet_send(udp_conn, data, length);
  PRINTF("-sent UDP datagram------\n Length: %u\n -----------------------\n", length);

  /* Restore server connection to allow data from any node */
  memset(&udp_conn->ripaddr, 0, sizeof(udp_conn->ripaddr));
  udp_conn->rport = 0;
}
/*-----------------------------------------------------------------------------------*/
/*- MEASSAGE PROCESSING -------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
void
coap_init_message(void *packet, coap_message_type_t type, uint8_t code, uint16_t tid)
{
  memset(packet, 0, sizeof(coap_packet_t));

  ((coap_packet_t *)packet)->type = type;
  ((coap_packet_t *)packet)->code = code;
  ((coap_packet_t *)packet)->tid = tid;
}
/*-----------------------------------------------------------------------------------*/
int
coap_serialize_message(void *packet, uint8_t *buffer)
{
  ((coap_packet_t *)packet)->buffer = buffer;
  ((coap_packet_t *)packet)->version = 1;
  ((coap_packet_t *)packet)->option_count = 0;

  /* serialize options */
  uint8_t *option = ((coap_packet_t *)packet)->buffer + COAP_HEADER_LEN;
  size_t option_len = 0;
  int index = 0;

  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_CONTENT_TYPE)) {
    ((coap_header_option_t *)option)->s.delta = 1;
    ((coap_header_option_t *)option)->s.length = 1;
    *(++option) = ((coap_packet_t *)packet)->content_type;
    PRINTF("OPTION %u (type %u, len 1, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_CONTENT_TYPE, COAP_OPTION_CONTENT_TYPE - index);
    PRINTF("Content-Type [%u]\n", ((coap_packet_t *)packet)->content_type);
    index = COAP_OPTION_CONTENT_TYPE;
    option += 1;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_MAX_AGE)) {
    option_len = uint32_2_bytes(option+1, ((coap_packet_t *)packet)->max_age);
    ((coap_header_option_t *)option)->s.delta = COAP_OPTION_MAX_AGE - index;
    ((coap_header_option_t *)option)->s.length = option_len;
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_MAX_AGE, option_len, COAP_OPTION_MAX_AGE - index);
    PRINTF("Max-Age [%lu]\n", ((coap_packet_t *)packet)->max_age);
    index = COAP_OPTION_MAX_AGE;
    option += 1 + option_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_ETAG)) {
    ((coap_header_option_t *)option)->s.delta = COAP_OPTION_ETAG - index;
    ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->etag_len;
    memcpy(++option, ((coap_packet_t *)packet)->etag, ((coap_packet_t *)packet)->etag_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_ETAG, ((coap_packet_t *)packet)->etag_len, COAP_OPTION_ETAG - index);
    PRINTF("ETag %u [0x%02X", ((coap_packet_t *)packet)->etag_len, ((coap_packet_t *)packet)->etag[0]); /*FIXME always prints 4 bytes */
    PRINTF("%02X", ((coap_packet_t *)packet)->etag[1]);
    PRINTF("%02X", ((coap_packet_t *)packet)->etag[2]);
    PRINTF("%02X", ((coap_packet_t *)packet)->etag[3]);
    PRINTF("]\n");
    index = COAP_OPTION_ETAG;
    option += ((coap_packet_t *)packet)->etag_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_HOST)) {
    if (((coap_packet_t *)packet)->uri_host_len<15) {
      ((coap_header_option_t *)option)->s.delta = COAP_OPTION_URI_HOST - index;
      ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->uri_host_len;
      option += 1;
    } else {
      ((coap_header_option_t *)option)->l.delta = COAP_OPTION_URI_HOST - index;
      ((coap_header_option_t *)option)->s.length = 15;
      ((coap_header_option_t *)option)->l.length = ((coap_packet_t *)packet)->uri_host_len - 15;
      option += 2;
    }
    memcpy(option, ((coap_packet_t *)packet)->uri_host, ((coap_packet_t *)packet)->uri_host_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_URI_HOST, ((coap_packet_t *)packet)->uri_host_len, COAP_OPTION_URI_HOST - index);
    PRINTF("Uri-Auth [%.*s]\n", ((coap_packet_t *)packet)->uri_host_len, ((coap_packet_t *)packet)->uri_host);
    index = COAP_OPTION_URI_HOST;
    option += ((coap_packet_t *)packet)->uri_host_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH)) {
    if (((coap_packet_t *)packet)->location_path_len<15) {
      ((coap_header_option_t *)option)->s.delta = COAP_OPTION_LOCATION_PATH - index;
      ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->location_path_len;
      option += 1;
    } else {
      ((coap_header_option_t *)option)->l.delta = COAP_OPTION_LOCATION_PATH - index;
      ((coap_header_option_t *)option)->s.length = 15;
      ((coap_header_option_t *)option)->l.length = ((coap_packet_t *)packet)->location_path_len - 15;
      option += 2;
    }
    memcpy(option, ((coap_packet_t *)packet)->location_path, ((coap_packet_t *)packet)->location_path_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_LOCATION_PATH, ((coap_packet_t *)packet)->location_path_len, COAP_OPTION_LOCATION_PATH - index);
    PRINTF("Location [%.*s]\n", ((coap_packet_t *)packet)->location_path_len, ((coap_packet_t *)packet)->location_path);
    index = COAP_OPTION_LOCATION_PATH;
    option += ((coap_packet_t *)packet)->location_path_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PATH)) {
    if (((coap_packet_t *)packet)->uri_path_len<15) {
      ((coap_header_option_t *)option)->s.delta = COAP_OPTION_URI_PATH - index;
      ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->uri_path_len;
      option += 1;
    } else {
      ((coap_header_option_t *)option)->l.delta = COAP_OPTION_URI_PATH - index;
      ((coap_header_option_t *)option)->s.length = 15;
      ((coap_header_option_t *)option)->l.length = ((coap_packet_t *)packet)->uri_path_len - 15;
      option += 2;
    }
    memcpy(option, ((coap_packet_t *)packet)->uri_path, ((coap_packet_t *)packet)->uri_path_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_URI_PATH, ((coap_packet_t *)packet)->uri_path_len, COAP_OPTION_URI_PATH - index);
    PRINTF("Uri-Path [%.*s]\n", ((coap_packet_t *)packet)->uri_path_len, ((coap_packet_t *)packet)->uri_path);
    index = COAP_OPTION_URI_PATH;
    option += ((coap_packet_t *)packet)->uri_path_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_OBSERVE)) {
    option_len = uint32_2_bytes(option+1, ((coap_packet_t *)packet)->observe);
    ((coap_header_option_t *)option)->s.delta = COAP_OPTION_OBSERVE - index;
    ((coap_header_option_t *)option)->s.length = option_len;
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_OBSERVE, option_len, COAP_OPTION_OBSERVE - index);
    PRINTF("Observe [%lu]\n", ((coap_packet_t *)packet)->observe);
    index = COAP_OPTION_OBSERVE;
    option += 1 + option_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_TOKEN)) {
    ((coap_header_option_t *)option)->s.delta = COAP_OPTION_TOKEN - index;
    ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->token_len;
    memcpy(++option, ((coap_packet_t *)packet)->token, ((coap_packet_t *)packet)->token_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_TOKEN, ((coap_packet_t *)packet)->token_len, COAP_OPTION_TOKEN - index);
    PRINTF("Token %u [0x%02X%02X]\n", ((coap_packet_t *)packet)->token_len, ((coap_packet_t *)packet)->token[0], ((coap_packet_t *)packet)->token[1]); /*FIXME always prints 2 bytes */
    index = COAP_OPTION_TOKEN;
    option += ((coap_packet_t *)packet)->token_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK)) {
    uint32_t block = ((coap_packet_t *)packet)->block_num << 4;
    if (((coap_packet_t *)packet)->block_more) block |= 0x8;
    block |= 0xF & log_2(((coap_packet_t *)packet)->block_size/16);
    option_len = uint32_2_bytes(option+1, block);
    ((coap_header_option_t *)option)->s.delta = COAP_OPTION_BLOCK - index;
    ((coap_header_option_t *)option)->s.length = option_len;
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_BLOCK, option_len, COAP_OPTION_BLOCK - index);
    PRINTF("Block [%lu%s (%u B/blk)]\n", ((coap_packet_t *)packet)->block_num, ((coap_packet_t *)packet)->block_more ? "+" : "", ((coap_packet_t *)packet)->block_size);
    index = COAP_OPTION_BLOCK;
    option += 1 + option_len;
    ++(((coap_packet_t *)packet)->option_count);
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY)) {
    if (((coap_packet_t *)packet)->uri_query_len<15) {
      ((coap_header_option_t *)option)->s.delta = COAP_OPTION_URI_QUERY - index;
      ((coap_header_option_t *)option)->s.length = ((coap_packet_t *)packet)->uri_query_len;
      option += 1;
    } else {
      ((coap_header_option_t *)option)->l.delta = COAP_OPTION_URI_QUERY - index;
      ((coap_header_option_t *)option)->s.length = 15;
      ((coap_header_option_t *)option)->l.length = ((coap_packet_t *)packet)->uri_query_len - 15;
      option += 2;
    }
    memcpy(option, ((coap_packet_t *)packet)->uri_query, ((coap_packet_t *)packet)->uri_query_len);
    PRINTF("OPTION %u (type %u, len %u, delta %u): ", ((coap_packet_t *)packet)->option_count, COAP_OPTION_URI_QUERY, ((coap_packet_t *)packet)->uri_query_len, COAP_OPTION_URI_QUERY - index);
    PRINTF("Uri-Query [%.*s]\n", ((coap_packet_t *)packet)->uri_query_len, ((coap_packet_t *)packet)->uri_query);
    index = COAP_OPTION_URI_QUERY;
    option += ((coap_packet_t *)packet)->uri_query_len;
    ++(((coap_packet_t *)packet)->option_count);
  }

  /* pack payload */
  if ((option - ((coap_packet_t *)packet)->buffer)<=COAP_MAX_HEADER_SIZE)
  {
    memmove(option, ((coap_packet_t *)packet)->payload, ((coap_packet_t *)packet)->payload_len);
  }
  else
  {
    /* An error occured. Caller must check for !=0. */
    return 0;
  }

  /* set header fields */
  ((coap_packet_t *)packet)->buffer[0]  = 0x00;
  ((coap_packet_t *)packet)->buffer[0] |= COAP_HEADER_VERSION_MASK & (((coap_packet_t *)packet)->version)<<COAP_HEADER_VERSION_POSITION;
  ((coap_packet_t *)packet)->buffer[0] |= COAP_HEADER_TYPE_MASK & (((coap_packet_t *)packet)->type)<<COAP_HEADER_TYPE_POSITION;
  ((coap_packet_t *)packet)->buffer[0] |= COAP_HEADER_OPTION_COUNT_MASK & (((coap_packet_t *)packet)->option_count)<<COAP_HEADER_OPTION_COUNT_POSITION;
  ((coap_packet_t *)packet)->buffer[1] = ((coap_packet_t *)packet)->code;
  ((coap_packet_t *)packet)->buffer[2] = 0xFF & (((coap_packet_t *)packet)->tid)>>8;
  ((coap_packet_t *)packet)->buffer[3] = 0xFF & ((coap_packet_t *)packet)->tid;

  PRINTF("Serialized %u options, header len %u, payload len %u\n", ((coap_packet_t *)packet)->option_count, option - ((coap_packet_t *)packet)->buffer, ((coap_packet_t *)packet)->payload_len);

  return (option - ((coap_packet_t *)packet)->buffer) + ((coap_packet_t *)packet)->payload_len; /* packet length */
}
/*-----------------------------------------------------------------------------------*/
error_t
coap_parse_message(void *packet, uint8_t *data, uint16_t data_len)
{
  /* Initialize packet */
  memset(packet, 0, sizeof(coap_packet_t));

  /* pointer to packet bytes */
  ((coap_packet_t *)packet)->buffer = data;

  /* parse header fields */
  ((coap_packet_t *)packet)->version = (COAP_HEADER_VERSION_MASK & ((coap_packet_t *)packet)->buffer[0])>>COAP_HEADER_VERSION_POSITION;
  ((coap_packet_t *)packet)->type = (COAP_HEADER_TYPE_MASK & ((coap_packet_t *)packet)->buffer[0])>>COAP_HEADER_TYPE_POSITION;
  ((coap_packet_t *)packet)->option_count = (COAP_HEADER_OPTION_COUNT_MASK & ((coap_packet_t *)packet)->buffer[0])>>COAP_HEADER_OPTION_COUNT_POSITION;
  ((coap_packet_t *)packet)->code = ((coap_packet_t *)packet)->buffer[1];
  ((coap_packet_t *)packet)->tid = ((coap_packet_t *)packet)->buffer[2]<<8 | ((coap_packet_t *)packet)->buffer[3];

  /* parse options */
  ((coap_packet_t *)packet)->options = 0x0000;
  coap_header_option_t *current_option = (coap_header_option_t *) (data + COAP_HEADER_LEN);

  if (((coap_packet_t *)packet)->option_count) {
    uint8_t option_index = 0;
    uint8_t option_type = 0;

    uint16_t option_len = 0;
    uint8_t *option_data = NULL;

    uint8_t *last_option = NULL;

    for (option_index=0; option_index < ((coap_packet_t *)packet)->option_count; ++option_index) {

      option_type += current_option->s.delta;

      if (current_option->s.length<15) {
        option_len = current_option->s.length;
        option_data = ((uint8_t *) current_option) + 1;
      } else {
        option_len = current_option->l.length + 15;
        option_data = ((uint8_t *) current_option) + 2;
      }

      PRINTF("OPTION %u (type %u, len %u, delta %u): ", option_index, option_type, option_len, current_option->s.delta);

      SET_OPTION((coap_packet_t *)packet, option_type);

      switch (option_type) {
        case COAP_OPTION_CONTENT_TYPE:
          ((coap_packet_t *)packet)->content_type = option_data[0];
          PRINTF("Content-Type [%u]\n", ((coap_packet_t *)packet)->content_type);
          break;
        case COAP_OPTION_MAX_AGE:
          ((coap_packet_t *)packet)->max_age = bytes_2_uint32(option_data, option_len);
          PRINTF("Max-Age [%lu]\n", ((coap_packet_t *)packet)->max_age);
          break;
        case COAP_OPTION_ETAG:
          ((coap_packet_t *)packet)->etag_len = MIN(COAP_ETAG_LEN, option_len);
          memcpy(((coap_packet_t *)packet)->etag, option_data, ((coap_packet_t *)packet)->etag_len);
          PRINTF("ETag %u [0x%02X", ((coap_packet_t *)packet)->etag_len, ((coap_packet_t *)packet)->etag[0]); /*FIXME always prints 4 bytes */
          PRINTF("%02X", ((coap_packet_t *)packet)->etag[1]);
          PRINTF("%02X", ((coap_packet_t *)packet)->etag[2]);
          PRINTF("%02X", ((coap_packet_t *)packet)->etag[3]);
          PRINTF("]\n");
          break;
        case COAP_OPTION_URI_HOST:
          ((coap_packet_t *)packet)->uri_host = (char *) option_data;
          ((coap_packet_t *)packet)->uri_host_len = option_len;
          PRINTF("Uri-Auth [%.*s]\n", ((coap_packet_t *)packet)->uri_host_len, ((coap_packet_t *)packet)->uri_host);
          break;
        case COAP_OPTION_LOCATION_PATH:
          ((coap_packet_t *)packet)->location_path = (char *) option_data;
          ((coap_packet_t *)packet)->location_path_len = option_len;
          PRINTF("Location [%.*s]\n", ((coap_packet_t *)packet)->location_path_len, ((coap_packet_t *)packet)->location_path);
          break;
        case COAP_OPTION_URI_PATH:
          ((coap_packet_t *)packet)->uri_path = (char *) option_data;
          ((coap_packet_t *)packet)->uri_path_len = option_len;
          PRINTF("Uri-Path [%.*s]\n", ((coap_packet_t *)packet)->uri_path_len, ((coap_packet_t *)packet)->uri_path);
          break;
        case COAP_OPTION_OBSERVE:
          ((coap_packet_t *)packet)->observe = bytes_2_uint32(option_data, option_len);
          PRINTF("Observe [%lu]\n", ((coap_packet_t *)packet)->observe);
          break;
        case COAP_OPTION_TOKEN:
          ((coap_packet_t *)packet)->token_len = MIN(COAP_TOKEN_LEN, option_len);
          memcpy(((coap_packet_t *)packet)->token, option_data, ((coap_packet_t *)packet)->token_len);
          PRINTF("Token %u [0x%02X%02X]\n", ((coap_packet_t *)packet)->token_len, ((coap_packet_t *)packet)->token[0], ((coap_packet_t *)packet)->token[1]); /*FIXME always prints 2 bytes */
          break;
        case COAP_OPTION_BLOCK:
          ((coap_packet_t *)packet)->block_num = bytes_2_uint32(option_data, option_len);
          ((coap_packet_t *)packet)->block_more = (((coap_packet_t *)packet)->block_num & 0x08)>>3;
          ((coap_packet_t *)packet)->block_size = 16 << (((coap_packet_t *)packet)->block_num & 0x07);
          ((coap_packet_t *)packet)->block_offset = (((coap_packet_t *)packet)->block_num & ~0x0F)<<(((coap_packet_t *)packet)->block_num & 0x07);
          ((coap_packet_t *)packet)->block_num >>= 4;
          PRINTF("Block [%lu%s (%u B/blk)]\n", ((coap_packet_t *)packet)->block_num, ((coap_packet_t *)packet)->block_more ? "+" : "", ((coap_packet_t *)packet)->block_size);
          break;
        case COAP_OPTION_NOOP:
          PRINTF("Noop-Fencepost\n");
          break;
        case COAP_OPTION_URI_QUERY:
          ((coap_packet_t *)packet)->uri_query = (char *) option_data;
          ((coap_packet_t *)packet)->uri_query_len = option_len;
          PRINTF("Uri-Query [%.*s]\n", ((coap_packet_t *)packet)->uri_query_len, ((coap_packet_t *)packet)->uri_query);
          break;
        default:
          PRINTF("unknown (%u)\n", option_type);
          if (option_type & 1)
          {
            return UNKNOWN_CRITICAL_OPTION;
          }
      }

      /* terminate strings where possible */
      if (last_option) {
        last_option[0] = 0x00;
      }

      last_option = (uint8_t *) current_option;
      current_option = (coap_header_option_t *) (option_data+option_len);
    } /* for () */
  } /* if (oc) */

  ((coap_packet_t *)packet)->payload = (uint8_t *) current_option;
  ((coap_packet_t *)packet)->payload_len = data_len - (((coap_packet_t *)packet)->payload - data);

  return NO_ERROR;
}
/*-----------------------------------------------------------------------------------*/
/*- REST FRAMEWORK FUNCTIONS --------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
int
coap_get_query_variable(void *packet, const char *name, const char **output)
{
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY)) {
    return coap_get_variable(((coap_packet_t *)packet)->uri_query, ((coap_packet_t *)packet)->uri_query_len, name, output);
  }
  return 0;
}

int
coap_get_post_variable(void *packet, const char *name, const char **output)
{
  if (((coap_packet_t *)packet)->payload_len) {
    return coap_get_variable((const char *)((coap_packet_t *)packet)->payload, ((coap_packet_t *)packet)->payload_len, name, output);
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/*- HEADER OPTION GETTERS AND SETTERS -----------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
unsigned int
coap_get_header_content_type(void *packet)
{
  return ((coap_packet_t *)packet)->content_type;
}

int
coap_set_header_content_type(void *packet, unsigned int content_type)
{
  ((coap_packet_t *)packet)->content_type = (coap_content_type_t) content_type;
  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_CONTENT_TYPE);
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_max_age(void *packet, uint32_t *age)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_MAX_AGE)) {
    *age = COAP_DEFAULT_MAX_AGE;
  } else {
    *age = ((coap_packet_t *)packet)->max_age;
  }
  return 1;
}

int
coap_set_header_max_age(void *packet, uint32_t age)
{
  ((coap_packet_t *)packet)->max_age = age;
  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_MAX_AGE);
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_etag(void *packet, const uint8_t **etag)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_ETAG)) return 0;

  *etag = ((coap_packet_t *)packet)->etag;
  return ((coap_packet_t *)packet)->etag_len;
}

int
coap_set_header_etag(void *packet, const uint8_t *etag, size_t etag_len)
{
  ((coap_packet_t *)packet)->etag_len = MIN(COAP_ETAG_LEN, etag_len);
  memcpy(((coap_packet_t *)packet)->etag, etag, ((coap_packet_t *)packet)->etag_len);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_ETAG);
  return ((coap_packet_t *)packet)->etag_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_uri_host(void *packet, const char **host)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_HOST)) return 0;

  *host = ((coap_packet_t *)packet)->uri_host;
  return ((coap_packet_t *)packet)->uri_host_len;
}

int
coap_set_header_uri_host(void *packet, const char *host)
{
  ((coap_packet_t *)packet)->uri_host = host;
  ((coap_packet_t *)packet)->uri_host_len = strlen(host);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_HOST);
  return ((coap_packet_t *)packet)->uri_host_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_location(void *packet, const char **location)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH)) return 0;

  *location = ((coap_packet_t *)packet)->location_path;
  return ((coap_packet_t *)packet)->location_path_len;
}

int
coap_set_header_location(void *packet, const char *location)
{
  while (location[0]=='/') ++location;

  ((coap_packet_t *)packet)->location_path = location;
  ((coap_packet_t *)packet)->location_path_len = strlen(location);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH);
  return ((coap_packet_t *)packet)->location_path_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_uri_path(void *packet, const char **path)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PATH)) return 0;

  *path = ((coap_packet_t *)packet)->uri_path;
  return ((coap_packet_t *)packet)->uri_path_len;
}

int
coap_set_header_uri_path(void *packet, const char *path)
{
  while (path[0]=='/') ++path;

  ((coap_packet_t *)packet)->uri_path = path;
  ((coap_packet_t *)packet)->uri_path_len = strlen(path);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PATH);
  return ((coap_packet_t *)packet)->uri_path_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_observe(void *packet, uint32_t *observe)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_OBSERVE)) return 0;

  *observe = ((coap_packet_t *)packet)->observe;
  return 1;
}

int
coap_set_header_observe(void *packet, uint32_t observe)
{
  ((coap_packet_t *)packet)->observe = observe;
  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_OBSERVE);
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_token(void *packet, const uint8_t **token)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_TOKEN)) return 0;

  *token = ((coap_packet_t *)packet)->token;
  return ((coap_packet_t *)packet)->token_len;
}

int
coap_set_header_token(void *packet, const uint8_t *token, size_t token_len)
{
  ((coap_packet_t *)packet)->token_len = MIN(COAP_TOKEN_LEN, token_len);
  memcpy(((coap_packet_t *)packet)->token, token, ((coap_packet_t *)packet)->token_len);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_TOKEN);
  return ((coap_packet_t *)packet)->token_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_block(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK)) return 0;

  /* pointers may be NULL to get only specific block parameters */
  if (num!=NULL) *num = ((coap_packet_t *)packet)->block_num;
  if (more!=NULL) *more = ((coap_packet_t *)packet)->block_more;
  if (size!=NULL) *size = ((coap_packet_t *)packet)->block_size;
  if (offset!=NULL) *offset = ((coap_packet_t *)packet)->block_offset;

  return 1;
}

int
coap_set_header_block(void *packet, uint32_t num, uint8_t more, uint16_t size)
{
  if (size<16) return 0;
  if (size>2048) return 0;
  if (num>0x0FFFFF) return 0;

  ((coap_packet_t *)packet)->block_num = num;
  ((coap_packet_t *)packet)->block_more = more;
  ((coap_packet_t *)packet)->block_size = size;

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK);
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_uri_query(void *packet, const char **query)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY)) return 0;

  *query = ((coap_packet_t *)packet)->uri_query;
  return ((coap_packet_t *)packet)->uri_query_len;
}

int
coap_set_header_uri_query(void *packet, const char *query)
{
  while (query[0]=='?') ++query;

  ((coap_packet_t *)packet)->uri_query = query;
  ((coap_packet_t *)packet)->uri_query_len = strlen(query);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY);
  return ((coap_packet_t *)packet)->uri_query_len;
}
/*-----------------------------------------------------------------------------------*/
/*- PAYLOAD -------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
int
coap_get_payload(void *packet, const uint8_t **payload)
{
  if (((coap_packet_t *)packet)->payload) {
    *payload = ((coap_packet_t *)packet)->payload;
    return ((coap_packet_t *)packet)->payload_len;
  } else {
    *payload = NULL;
    return 0;
  }
}

int
coap_set_payload(void *packet, const void *payload, size_t length)
{
  PRINTF("setting payload (%u/%u)\n", length, REST_MAX_CHUNK_SIZE);

  ((coap_packet_t *)packet)->payload = (uint8_t *) payload;
  ((coap_packet_t *)packet)->payload_len = MIN(REST_MAX_CHUNK_SIZE, length);

  return ((coap_packet_t *)packet)->payload_len;
}
/*-----------------------------------------------------------------------------------*/
