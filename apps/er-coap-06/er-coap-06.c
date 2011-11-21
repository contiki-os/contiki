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
 *      An implementation of the Constrained Application Protocol (draft 06)
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"
#include "contiki-net.h"
#include <string.h>
#include <stdio.h>

#include "er-coap-06.h"
#include "er-coap-06-transactions.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*-----------------------------------------------------------------------------------*/
/*- Variables -----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
static struct uip_udp_conn *udp_conn = NULL;
static uint16_t current_tid = 0;

coap_status_t coap_error_code = NO_ERROR;
char *coap_error_message = "";
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
parse_int_option(uint8_t *bytes, uint16_t length)
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
static
size_t
set_option_header(int delta, size_t length, uint8_t *buffer)
{
  if (length<15)
  {
    buffer[0] = (0x0F & length) | (0xF0 & delta<<4);
    return 1;
  }
  else
  {
    buffer[0] = 0x0F | (0xF0 & delta<<4);
    buffer[1] = 0xFF & (length - 15);
    return 2;
  }
}
/*-----------------------------------------------------------------------------------*/
static
size_t
insert_option_fence_posts(int number, int *current_number, uint8_t *buffer)
{
  size_t i = 0;
  while (number-*current_number > 15)
  {
    uint8_t delta = COAP_OPTION_FENCE_POST - (*current_number%COAP_OPTION_FENCE_POST);
    set_option_header(delta, 0, &buffer[i++]);
    *current_number += delta;

    PRINTF("OPTION FENCE POST delta %u\n", delta);
  }
  return i;
}
/*-----------------------------------------------------------------------------------*/
static
size_t
serialize_int_option(int number, int current_number, uint8_t *buffer, uint32_t value)
{
  /* Insert fence-posts for large deltas */
  size_t i = insert_option_fence_posts(number, &current_number, buffer);
  size_t start_i = i;

  uint8_t *option = &buffer[i];

  if (0xFF000000 & value) buffer[++i] = (uint8_t) (value>>24);
  if (0x00FF0000 & value) buffer[++i] = (uint8_t) (value>>16);
  if (0x0000FF00 & value) buffer[++i] = (uint8_t) (value>>8);
  if (0x000000FF & value) buffer[++i] = (uint8_t) value;

  i += set_option_header(number - current_number, i-start_i, option);

  PRINTF("OPTION type %u, delta %u, len %u\n", number, number - current_number, i-start_i);

  return i;
}
/*-----------------------------------------------------------------------------------*/
static
size_t
serialize_array_option(int number, int current_number, uint8_t *buffer, uint8_t *array, size_t length, uint8_t *split_option)
{
  /* Insert fence-posts for large deltas */
  size_t i = insert_option_fence_posts(number, &current_number, buffer);

  if (split_option!=NULL)
  {
    int j;
    uint8_t *part_start = array;
    uint8_t *part_end = NULL;
    size_t temp_length;

    char split_char = *split_option;
    *split_option = 0; /* Ensure reflecting the created option count */

    for (j = 0; j<=length; ++j)
    {
      if (array[j]==split_char || j==length)
      {
        part_end = array + j;
        temp_length = part_end-part_start;

        i += set_option_header(number - current_number, temp_length, &buffer[i]);
        memcpy(&buffer[i], part_start, temp_length);
        i += temp_length;

        PRINTF("OPTION type %u, delta %u, len %u, part [%.*s]\n", number, number - current_number, i, temp_length, part_start);

        ++(*split_option);
        ++j; /* skip the slash */
        current_number = number;
        while( array[j]=='/') ++j;
        part_start = array + j;
      }
    } /* for */
  }
  else
  {
    i += set_option_header(number - current_number, length, &buffer[i]);
    memcpy(&buffer[i], array, length);
    i += length;

    PRINTF("OPTION type %u, delta %u, len %u\n", number, number - current_number, i);
  }

  return i;
}
/*-----------------------------------------------------------------------------------*/
static
void
coap_merge_multi_option(char **dst, size_t *dst_len, uint8_t *option, size_t option_len, char separator)
{
  /* Merge multiple options. */
  if (*dst_len > 0)
  {
    (*dst)[*dst_len] = separator;
    *dst_len += 1;

    /* memmove handles 2-byte option headers */
    memmove((*dst)+(*dst_len), option, option_len);

    *dst_len += option_len;
  }
  else
  {
    *dst = (char *) option; /* thus pointer pointer */
    *dst_len = option_len;
  }
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
  return ++current_tid;
}
/*-----------------------------------------------------------------------------------*/
/*- MEASSAGE PROCESSING -------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
void
coap_init_message(void *packet, coap_message_type_t type, uint8_t code, uint16_t tid)
{
  /* Important thing */
  memset(packet, 0, sizeof(coap_packet_t));

  ((coap_packet_t *)packet)->type = type;
  ((coap_packet_t *)packet)->code = code;
  ((coap_packet_t *)packet)->tid = tid;
}
/*-----------------------------------------------------------------------------------*/
size_t
coap_serialize_message(void *packet, uint8_t *buffer)
{
  /* Initialize */
  ((coap_packet_t *)packet)->buffer = buffer;
  ((coap_packet_t *)packet)->version = 1;
  ((coap_packet_t *)packet)->option_count = 0;

  /* serialize options */
  uint8_t *option = ((coap_packet_t *)packet)->buffer + COAP_HEADER_LEN;
  int current_number = 0;

  PRINTF("-Serializing options-\n");

  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_CONTENT_TYPE)) {
    PRINTF("Content-Type [%u]\n", ((coap_packet_t *)packet)->content_type);

    option += serialize_int_option(COAP_OPTION_CONTENT_TYPE, current_number, option, ((coap_packet_t *)packet)->content_type);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_CONTENT_TYPE;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_MAX_AGE)) {
    PRINTF("Max-Age [%lu]\n", ((coap_packet_t *)packet)->max_age);

    option += serialize_int_option(COAP_OPTION_MAX_AGE, current_number, option, ((coap_packet_t *)packet)->max_age);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_MAX_AGE;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_PROXY_URI)) {
    PRINTF("Proxy-Uri [%.*s]\n", ((coap_packet_t *)packet)->proxy_uri_len, ((coap_packet_t *)packet)->proxy_uri);

    int length = ((coap_packet_t *)packet)->proxy_uri_len;
    int j = 0;
    while (length>0)
    {
        option += serialize_array_option(COAP_OPTION_PROXY_URI, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->proxy_uri + j*270, MIN(270, length), NULL);
        ((coap_packet_t *)packet)->option_count += 1;
        current_number = COAP_OPTION_PROXY_URI;

        ++j;
        length -= 270;
    }
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_ETAG)) {
    PRINTF("ETag %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", ((coap_packet_t *)packet)->etag_len,
      ((coap_packet_t *)packet)->etag[0],
      ((coap_packet_t *)packet)->etag[1],
      ((coap_packet_t *)packet)->etag[2],
      ((coap_packet_t *)packet)->etag[3],
      ((coap_packet_t *)packet)->etag[4],
      ((coap_packet_t *)packet)->etag[5],
      ((coap_packet_t *)packet)->etag[6],
      ((coap_packet_t *)packet)->etag[7]
    ); /*FIXME always prints 8 bytes */

    option += serialize_array_option(COAP_OPTION_ETAG, current_number, option, ((coap_packet_t *)packet)->etag, ((coap_packet_t *)packet)->etag_len, NULL);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_ETAG;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_HOST)) {
    PRINTF("Uri-Host [%.*s]\n", ((coap_packet_t *)packet)->uri_host_len, ((coap_packet_t *)packet)->uri_host);

    option += serialize_array_option(COAP_OPTION_URI_HOST, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->uri_host, ((coap_packet_t *)packet)->uri_host_len, NULL);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_URI_HOST;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH)) {
    PRINTF("Location [%.*s]\n", ((coap_packet_t *)packet)->location_path_len, ((coap_packet_t *)packet)->location_path);

    uint8_t split_options = '/';

    option += serialize_array_option(COAP_OPTION_LOCATION_PATH, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->location_path, ((coap_packet_t *)packet)->location_path_len, &split_options);
    ((coap_packet_t *)packet)->option_count += split_options;
    current_number = COAP_OPTION_LOCATION_PATH;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PORT)) {
    PRINTF("Uri-Port [%u]\n", ((coap_packet_t *)packet)->uri_port);

    option += serialize_int_option(COAP_OPTION_URI_PORT, current_number, option, ((coap_packet_t *)packet)->uri_port);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_URI_PORT;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_QUERY)) {
    PRINTF("Location-Query [%.*s]\n", ((coap_packet_t *)packet)->location_query_len, ((coap_packet_t *)packet)->location_query);

    uint8_t split_options = '&';

    option += serialize_array_option(COAP_OPTION_LOCATION_QUERY, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->location_query, ((coap_packet_t *)packet)->location_query_len, &split_options);
    ((coap_packet_t *)packet)->option_count += split_options;
    current_number = COAP_OPTION_LOCATION_QUERY;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PATH)) {
    PRINTF("Uri-Path [%.*s]\n", ((coap_packet_t *)packet)->uri_path_len, ((coap_packet_t *)packet)->uri_path);

    uint8_t split_options = '/';

    option += serialize_array_option(COAP_OPTION_URI_PATH, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->uri_path, ((coap_packet_t *)packet)->uri_path_len, &split_options);
    ((coap_packet_t *)packet)->option_count += split_options;
    current_number = COAP_OPTION_URI_PATH;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_OBSERVE)) {
    PRINTF("Observe [%u]\n", ((coap_packet_t *)packet)->observe);

    option += serialize_int_option(COAP_OPTION_OBSERVE, current_number, option, ((coap_packet_t *)packet)->observe);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_OBSERVE;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_TOKEN)) {
    PRINTF("Token %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", ((coap_packet_t *)packet)->token_len,
      ((coap_packet_t *)packet)->token[0],
      ((coap_packet_t *)packet)->token[1],
      ((coap_packet_t *)packet)->token[2],
      ((coap_packet_t *)packet)->token[3],
      ((coap_packet_t *)packet)->token[4],
      ((coap_packet_t *)packet)->token[5],
      ((coap_packet_t *)packet)->token[6],
      ((coap_packet_t *)packet)->token[7]
    ); /*FIXME always prints 8 bytes */

    option += serialize_array_option(COAP_OPTION_TOKEN, current_number, option, ((coap_packet_t *)packet)->token, ((coap_packet_t *)packet)->token_len, NULL);
    ((coap_packet_t *)packet)->option_count += 1;
    current_number = COAP_OPTION_TOKEN;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY)) {
    PRINTF("Uri-Query [%.*s]\n", ((coap_packet_t *)packet)->uri_query_len, ((coap_packet_t *)packet)->uri_query);

    uint8_t split_options = '&';

    option += serialize_array_option(COAP_OPTION_URI_QUERY, current_number, option, (uint8_t *) ((coap_packet_t *)packet)->uri_query, ((coap_packet_t *)packet)->uri_query_len, &split_options);
    ((coap_packet_t *)packet)->option_count += split_options + (COAP_OPTION_URI_QUERY-current_number)/COAP_OPTION_FENCE_POST;
    current_number = COAP_OPTION_URI_QUERY;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK2))
  {
    PRINTF("Block2 [%lu%s (%u B/blk)]\n", ((coap_packet_t *)packet)->block2_num, ((coap_packet_t *)packet)->block2_more ? "+" : "", ((coap_packet_t *)packet)->block2_size);

    uint32_t block = ((coap_packet_t *)packet)->block2_num << 4;
    if (((coap_packet_t *)packet)->block2_more) block |= 0x8;
    block |= 0xF & log_2(((coap_packet_t *)packet)->block2_size/16);

    PRINTF("Block2 encoded: 0x%lX\n", block);

    option += serialize_int_option(COAP_OPTION_BLOCK2, current_number, option, block);

    ((coap_packet_t *)packet)->option_count += 1 + (COAP_OPTION_BLOCK2-current_number)/COAP_OPTION_FENCE_POST;
    current_number = COAP_OPTION_BLOCK2;
  }
  if (IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK1))
  {
    PRINTF("Block1 [%lu%s (%u B/blk)]\n", ((coap_packet_t *)packet)->block1_num, ((coap_packet_t *)packet)->block1_more ? "+" : "", ((coap_packet_t *)packet)->block1_size);

    uint32_t block = ((coap_packet_t *)packet)->block1_num << 4;
    if (((coap_packet_t *)packet)->block1_more) block |= 0x8;
    block |= 0xF & log_2(((coap_packet_t *)packet)->block1_size/16);

    PRINTF("Block1 encoded: 0x%lX\n", block);

    option += serialize_int_option(COAP_OPTION_BLOCK1, current_number, option, block);

    ((coap_packet_t *)packet)->option_count += 1 + (COAP_OPTION_BLOCK1-current_number)/COAP_OPTION_FENCE_POST;
    current_number = COAP_OPTION_BLOCK1;
  }

  /* pack payload */
  if ((option - ((coap_packet_t *)packet)->buffer)<=COAP_MAX_HEADER_SIZE)
  {
    memmove(option, ((coap_packet_t *)packet)->payload, ((coap_packet_t *)packet)->payload_len);
  }
  else
  {
    /* An error occured. Caller must check for !=0. */
    ((coap_packet_t *)packet)->buffer = NULL;
    coap_error_message = "Serialized header exceeds COAP_MAX_HEADER_SIZE";
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

  PRINTF("-Done %u options, header len %u, payload len %u-\n", ((coap_packet_t *)packet)->option_count, option - buffer, ((coap_packet_t *)packet)->payload_len);

  return (option - buffer) + ((coap_packet_t *)packet)->payload_len; /* packet length */
}
/*-----------------------------------------------------------------------------------*/
void
coap_send_message(uip_ipaddr_t *addr, uint16_t port, uint8_t *data, uint16_t length)
{
  /*configure connection to reply to client*/
  uip_ipaddr_copy(&udp_conn->ripaddr, addr);
  udp_conn->rport = port;

  uip_udp_packet_send(udp_conn, data, length);
  PRINTF("-sent UDP datagram (%u)-\n", length);

  /* Restore server connection to allow data from any node */
  memset(&udp_conn->ripaddr, 0, sizeof(udp_conn->ripaddr));
  udp_conn->rport = 0;
}
/*-----------------------------------------------------------------------------------*/
coap_status_t
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

  if (((coap_packet_t *)packet)->version != 1)
  {
    coap_error_message = "CoAP version must be 1";
    return BAD_REQUEST_4_00;
  }

  /* parse options */
  ((coap_packet_t *)packet)->options = 0x0000;
  uint8_t *current_option = data + COAP_HEADER_LEN;

  if (((coap_packet_t *)packet)->option_count)
  {
    uint8_t option_index = 0;

    uint8_t current_number = 0;
    size_t option_len = 0;

    PRINTF("-Parsing %u options-\n", ((coap_packet_t *)packet)->option_count);
    for (option_index=0; option_index < ((coap_packet_t *)packet)->option_count; ++option_index) {

      current_number += current_option[0]>>4;

      PRINTF("OPTION %u (type %u, delta %u, len %u): ", option_index, current_number, current_option[0]>>4, (0x0F & current_option[0]) < 15 ? (0x0F & current_option[0]) : current_option[1] + 15);

      if ((0x0F & current_option[0]) < 15) {
        option_len = 0x0F & current_option[0];
        current_option += 1;
      } else {
        option_len = current_option[1] + 15;
        current_option += 2;
      }

      SET_OPTION((coap_packet_t *)packet, current_number);

      switch (current_number) {
        case COAP_OPTION_CONTENT_TYPE:
          ((coap_packet_t *)packet)->content_type = parse_int_option(current_option, option_len);
          PRINTF("Content-Type [%u]\n", ((coap_packet_t *)packet)->content_type);
          break;
        case COAP_OPTION_MAX_AGE:
          ((coap_packet_t *)packet)->max_age = parse_int_option(current_option, option_len);
          PRINTF("Max-Age [%lu]\n", ((coap_packet_t *)packet)->max_age);
          break;
        case COAP_OPTION_PROXY_URI:
          /*FIXME check for own end-point */
          ((coap_packet_t *)packet)->proxy_uri = (char *) current_option;
          ((coap_packet_t *)packet)->proxy_uri_len = option_len;
          /*TODO length > 270 not implemented (actually not required) */
          PRINTF("Proxy-Uri NOT IMPLEMENTED [%.*s]\n", ((coap_packet_t *)packet)->proxy_uri_len, ((coap_packet_t *)packet)->proxy_uri);
          coap_error_message = "This is a constrained server (Contiki)";
          return PROXYING_NOT_SUPPORTED_5_05;
          break;
        case COAP_OPTION_ETAG:
          ((coap_packet_t *)packet)->etag_len = MIN(COAP_ETAG_LEN, option_len);
          memcpy(((coap_packet_t *)packet)->etag, current_option, ((coap_packet_t *)packet)->etag_len);
          PRINTF("ETag %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", ((coap_packet_t *)packet)->etag_len,
            ((coap_packet_t *)packet)->etag[0],
            ((coap_packet_t *)packet)->etag[1],
            ((coap_packet_t *)packet)->etag[2],
            ((coap_packet_t *)packet)->etag[3],
            ((coap_packet_t *)packet)->etag[4],
            ((coap_packet_t *)packet)->etag[5],
            ((coap_packet_t *)packet)->etag[6],
            ((coap_packet_t *)packet)->etag[7]
          ); /*FIXME always prints 8 bytes */
          break;
        case COAP_OPTION_URI_HOST:
          ((coap_packet_t *)packet)->uri_host = (char *) current_option;
          ((coap_packet_t *)packet)->uri_host_len = option_len;
          PRINTF("Uri-Host [%.*s]\n", ((coap_packet_t *)packet)->uri_host_len, ((coap_packet_t *)packet)->uri_host);
          break;
        case COAP_OPTION_LOCATION_PATH:
          coap_merge_multi_option(&(((coap_packet_t *)packet)->location_path), &(((coap_packet_t *)packet)->location_path_len), current_option, option_len, '/');
          PRINTF("Location-Path [%.*s]\n", ((coap_packet_t *)packet)->location_path_len, ((coap_packet_t *)packet)->location_path);
          break;
        case COAP_OPTION_URI_PORT:
          ((coap_packet_t *)packet)->uri_port = parse_int_option(current_option, option_len);
          PRINTF("Uri-Port [%u]\n", ((coap_packet_t *)packet)->uri_port);
          break;
        case COAP_OPTION_LOCATION_QUERY:
          coap_merge_multi_option(&(((coap_packet_t *)packet)->location_query), &(((coap_packet_t *)packet)->location_query_len), current_option, option_len, '&');
          PRINTF("Location-Query [%.*s]\n", ((coap_packet_t *)packet)->location_query_len, ((coap_packet_t *)packet)->location_query);
          break;
        case COAP_OPTION_URI_PATH:
          coap_merge_multi_option(&(((coap_packet_t *)packet)->uri_path), &(((coap_packet_t *)packet)->uri_path_len), current_option, option_len, '/');
          PRINTF("Uri-Path [%.*s]\n", ((coap_packet_t *)packet)->uri_path_len, ((coap_packet_t *)packet)->uri_path);
          break;
        case COAP_OPTION_OBSERVE:
          ((coap_packet_t *)packet)->observe = parse_int_option(current_option, option_len);
          PRINTF("Observe [%u]\n", ((coap_packet_t *)packet)->observe);
          break;
        case COAP_OPTION_TOKEN:
          ((coap_packet_t *)packet)->token_len = MIN(COAP_TOKEN_LEN, option_len);
          memcpy(((coap_packet_t *)packet)->token, current_option, ((coap_packet_t *)packet)->token_len);
          PRINTF("Token %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", ((coap_packet_t *)packet)->token_len,
            ((coap_packet_t *)packet)->token[0],
            ((coap_packet_t *)packet)->token[1],
            ((coap_packet_t *)packet)->token[2],
            ((coap_packet_t *)packet)->token[3],
            ((coap_packet_t *)packet)->token[4],
            ((coap_packet_t *)packet)->token[5],
            ((coap_packet_t *)packet)->token[6],
            ((coap_packet_t *)packet)->token[7]
          ); /*FIXME always prints 8 bytes */
          break;
        case COAP_OPTION_FENCE_POST:
          PRINTF("Fence-Post\n");
          break;
        case COAP_OPTION_URI_QUERY:
          coap_merge_multi_option(&(((coap_packet_t *)packet)->uri_query), &(((coap_packet_t *)packet)->uri_query_len), current_option, option_len, '&');
          PRINTF("Uri-Query [%.*s]\n", ((coap_packet_t *)packet)->uri_query_len, ((coap_packet_t *)packet)->uri_query);
          break;
        case COAP_OPTION_BLOCK2:
          ((coap_packet_t *)packet)->block2_num = parse_int_option(current_option, option_len);
          ((coap_packet_t *)packet)->block2_more = (((coap_packet_t *)packet)->block2_num & 0x08)>>3;
          ((coap_packet_t *)packet)->block2_size = 16 << (((coap_packet_t *)packet)->block2_num & 0x07);
          ((coap_packet_t *)packet)->block2_offset = (((coap_packet_t *)packet)->block2_num & ~0x0F)<<(((coap_packet_t *)packet)->block2_num & 0x07);
          ((coap_packet_t *)packet)->block2_num >>= 4;
          PRINTF("Block2 [%lu%s (%u B/blk)]\n", ((coap_packet_t *)packet)->block2_num, ((coap_packet_t *)packet)->block2_more ? "+" : "", ((coap_packet_t *)packet)->block2_size);
          break;
        case COAP_OPTION_BLOCK1:
          PRINTF("Block1 NOT IMPLEMENTED\n");
          /*TODO implement */
          coap_error_message = "Blockwise POST/PUT not supported";
          return NOT_IMPLEMENTED_5_01;
          break;
        default:
          PRINTF("unknown (%u)\n", current_number);
          /* Check if critical (odd) */
          if (current_number & 1)
          {
            coap_error_message = "Unsupported critical option";
            return BAD_OPTION_4_02;
          }
      }

      current_option += option_len;
    } /* for */
    PRINTF("-Done parsing-------\n");
  } /* if (oc) */

  ((coap_packet_t *)packet)->payload = current_option;
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
coap_set_header_etag(void *packet, uint8_t *etag, size_t etag_len)
{
  ((coap_packet_t *)packet)->etag_len = MIN(COAP_ETAG_LEN, etag_len);
  memcpy(((coap_packet_t *)packet)->etag, etag, ((coap_packet_t *)packet)->etag_len);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_ETAG);
  return ((coap_packet_t *)packet)->etag_len;
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
coap_set_header_token(void *packet, uint8_t *token, size_t token_len)
{
  ((coap_packet_t *)packet)->token_len = MIN(COAP_TOKEN_LEN, token_len);
  memcpy(((coap_packet_t *)packet)->token, token, ((coap_packet_t *)packet)->token_len);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_TOKEN);
  return ((coap_packet_t *)packet)->token_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_proxy_uri(void *packet, const char **uri)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_PROXY_URI)) return 0;

  *uri = ((coap_packet_t *)packet)->proxy_uri;
  return ((coap_packet_t *)packet)->proxy_uri_len;
}

int
coap_set_header_proxy_uri(void *packet, char *uri)
{
  ((coap_packet_t *)packet)->proxy_uri = uri;
  ((coap_packet_t *)packet)->proxy_uri_len = strlen(uri);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_PROXY_URI);
  return ((coap_packet_t *)packet)->proxy_uri_len;
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
coap_set_header_uri_host(void *packet, char *host)
{
  ((coap_packet_t *)packet)->uri_host = host;
  ((coap_packet_t *)packet)->uri_host_len = strlen(host);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_HOST);
  return ((coap_packet_t *)packet)->uri_host_len;
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
coap_set_header_uri_path(void *packet, char *path)
{
  while (path[0]=='/') ++path;

  ((coap_packet_t *)packet)->uri_path = path;
  ((coap_packet_t *)packet)->uri_path_len = strlen(path);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_PATH);
  return ((coap_packet_t *)packet)->uri_path_len;
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
coap_set_header_uri_query(void *packet, char *query)
{
  while (query[0]=='?') ++query;

  ((coap_packet_t *)packet)->uri_query = query;
  ((coap_packet_t *)packet)->uri_query_len = strlen(query);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_URI_QUERY);
  return ((coap_packet_t *)packet)->uri_query_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_location_path(void *packet, const char **path)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH)) return 0;

  *path = ((coap_packet_t *)packet)->location_path;
  return ((coap_packet_t *)packet)->location_path_len;
}

int
coap_set_header_location_path(void *packet, char *path)
{
  char *query;

  while (path[0]=='/') ++path;

  if ((query = strchr(path, '?')))
  {
    coap_set_header_location_query(packet, query+1);
    ((coap_packet_t *)packet)->location_path_len = query - path;
  }
  else
  {
    ((coap_packet_t *)packet)->location_path_len = strlen(path);
  }

  ((coap_packet_t *)packet)->location_path = path;

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_PATH);
  return ((coap_packet_t *)packet)->location_path_len;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_location_query(void *packet, const char **query)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_QUERY)) return 0;

  *query = ((coap_packet_t *)packet)->location_query;
  return ((coap_packet_t *)packet)->location_query_len;
}

int
coap_set_header_location_query(void *packet, char *query)
{
  while (query[0]=='?') ++query;

  ((coap_packet_t *)packet)->location_query = query;
  ((coap_packet_t *)packet)->location_query_len = strlen(query);

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_LOCATION_QUERY);
  return ((coap_packet_t *)packet)->location_query_len;
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
coap_get_header_block2(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK2)) return 0;

  /* pointers may be NULL to get only specific block parameters */
  if (num!=NULL) *num = ((coap_packet_t *)packet)->block2_num;
  if (more!=NULL) *more = ((coap_packet_t *)packet)->block2_more;
  if (size!=NULL) *size = ((coap_packet_t *)packet)->block2_size;
  if (offset!=NULL) *offset = ((coap_packet_t *)packet)->block2_offset;

  return 1;
}

int
coap_set_header_block2(void *packet, uint32_t num, uint8_t more, uint16_t size)
{
  if (size<16) return 0;
  if (size>2048) return 0;
  if (num>0x0FFFFF) return 0;

  ((coap_packet_t *)packet)->block2_num = num;
  ((coap_packet_t *)packet)->block2_more = more;
  ((coap_packet_t *)packet)->block2_size = size;

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK2);
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
coap_get_header_block1(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset)
{
  if (!IS_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK1)) return 0;

  /* pointers may be NULL to get only specific block parameters */
  if (num!=NULL) *num = ((coap_packet_t *)packet)->block1_num;
  if (more!=NULL) *more = ((coap_packet_t *)packet)->block1_more;
  if (size!=NULL) *size = ((coap_packet_t *)packet)->block1_size;
  if (offset!=NULL) *offset = ((coap_packet_t *)packet)->block1_offset;

  return 1;
}

int
coap_set_header_block1(void *packet, uint32_t num, uint8_t more, uint16_t size)
{
  if (size<16) return 0;
  if (size>2048) return 0;
  if (num>0x0FFFFF) return 0;

  ((coap_packet_t *)packet)->block1_num = num;
  ((coap_packet_t *)packet)->block1_more = more;
  ((coap_packet_t *)packet)->block1_size = size;

  SET_OPTION((coap_packet_t *)packet, COAP_OPTION_BLOCK1);
  return 1;
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
coap_set_payload(void *packet, uint8_t *payload, size_t length)
{
  PRINTF("setting payload (%u/%u)\n", length, REST_MAX_CHUNK_SIZE);

  ((coap_packet_t *)packet)->payload = payload;
  ((coap_packet_t *)packet)->payload_len = MIN(REST_MAX_CHUNK_SIZE, length);

  return ((coap_packet_t *)packet)->payload_len;
}
/*-----------------------------------------------------------------------------------*/
