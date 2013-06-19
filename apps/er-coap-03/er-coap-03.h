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

#ifndef COAP_03_H_
#define COAP_03_H_

#include <stddef.h> /* for size_t */
#include "contiki-net.h"
#include "erbium.h"

#define COAP_DEFAULT_PORT                    61616

#ifndef COAP_SERVER_PORT
#define COAP_SERVER_PORT                     COAP_DEFAULT_PORT
#endif

#define COAP_DEFAULT_MAX_AGE                 60
#define COAP_RESPONSE_TIMEOUT                1
#define COAP_MAX_RETRANSMIT                  5

#define COAP_HEADER_LEN                      4 /* | oc:0xF0 type:0x0C version:0x03 | code | tid:0x00FF | tid:0xFF00 | */
#define COAP_ETAG_LEN                        4 /* The maximum number of bytes for the ETag, which is 4 for coap-03 */
#define COAP_TOKEN_LEN                       2 /* The maximum number of bytes for the ETag, which is 4 for coap-03 */

#define COAP_HEADER_VERSION_MASK             0xC0
#define COAP_HEADER_VERSION_POSITION         6
#define COAP_HEADER_TYPE_MASK                0x30
#define COAP_HEADER_TYPE_POSITION            4
#define COAP_HEADER_OPTION_COUNT_MASK        0x0F
#define COAP_HEADER_OPTION_COUNT_POSITION    0

#define COAP_HEADER_OPTION_DELTA_MASK        0xF0
#define COAP_HEADER_OPTION_SHORT_LENGTH_MASK 0x0F

/*
 * Conservative size limit, as not all options have to be set at the same time.
 */
/*                            Hdr CoT Age Tag Obs Tok Blo strings */
#define COAP_MAX_HEADER_SIZE  (4 + 2 + 5 + 5 + 5 + 5 + 4 + 0)
#define COAP_MAX_PACKET_SIZE  (COAP_MAX_HEADER_SIZE + REST_MAX_CHUNK_SIZE)
/*                                        0/14          48 for IPv6 (28 for IPv4) */
#if COAP_MAX_PACKET_SIZE > (UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN)
#error "UIP_CONF_BUFFER_SIZE too small for REST_MAX_CHUNK_SIZE"
#endif

/*
 * Maximum number of failed request attempts before action
 */
#ifndef COAP_MAX_ATTEMPTS
#define COAP_MAX_ATTEMPTS             4
#endif /* COAP_MAX_ATTEMPTS */

#define UIP_IP_BUF    ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF   ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])

#define SET_OPTION(packet, opt) ((packet)->options |= 1<<opt)
#define IS_OPTION(packet, opt) ((packet)->options & 1<<opt)

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

/* CoAP message types */
typedef enum {
  COAP_TYPE_CON, /* confirmables */
  COAP_TYPE_NON, /* non-confirmables */
  COAP_TYPE_ACK, /* acknowledgements */
  COAP_TYPE_RST  /* reset */
} coap_message_type_t;

/* CoAP request method codes */
typedef enum {
  COAP_GET = 1,
  COAP_POST,
  COAP_PUT,
  COAP_DELETE
} coap_method_t;

/* CoAP response codes */
typedef enum {
  CONTINUE_100 = 40,
  OK_200 = 80,
  CREATED_201 = 81,
  NOT_MODIFIED_304 = 124,
  BAD_REQUEST_400 = 160,
  NOT_FOUND_404 = 164,
  METHOD_NOT_ALLOWED_405 = 165,
  UNSUPPORTED_MEDIA_TYPE_415 = 175,
  INTERNAL_SERVER_ERROR_500 = 200,
  BAD_GATEWAY_502 = 202,
  SERVICE_UNAVAILABLE_503 = 203,
  GATEWAY_TIMEOUT_504 = 204,
  TOKEN_OPTION_REQUIRED = 240,
  HOST_REQUIRED = 241,
  CRITICAL_OPTION_NOT_SUPPORTED = 242
} coap_status_t;

/* CoAP header options */
typedef enum {
  COAP_OPTION_CONTENT_TYPE = 1,  /* 1 B */
  COAP_OPTION_MAX_AGE = 2,       /* 1-4 B */
  COAP_OPTION_ETAG = 4,          /* 1-4 B */
  COAP_OPTION_URI_HOST = 5,      /* 1-270 B */
  COAP_OPTION_LOCATION_PATH = 6, /* 1-270 B */
  COAP_OPTION_URI_PATH = 9,      /* 1-270 B */
  COAP_OPTION_OBSERVE = 10,      /* 0-4 B */
  COAP_OPTION_TOKEN = 11,        /* 1-2 B */
  COAP_OPTION_BLOCK = 13,        /* 1-3 B */
  COAP_OPTION_NOOP = 14,         /* 0 B */
  COAP_OPTION_URI_QUERY = 15     /* 1-270 B */
} coap_option_t;

/* CoAP content-types */
typedef enum {
  TEXT_PLAIN = 0,
  TEXT_XML = 1,
  TEXT_CSV = 2,
  TEXT_HTML = 3,
  IMAGE_GIF = 21,
  IMAGE_JPEG = 22,
  IMAGE_PNG = 23,
  IMAGE_TIFF = 24,
  AUDIO_RAW = 25,
  VIDEO_RAW = 26,
  APPLICATION_LINK_FORMAT = 40,
  APPLICATION_XML = 41,
  APPLICATION_OCTET_STREAM = 42,
  APPLICATION_RDF_XML = 43,
  APPLICATION_SOAP_XML = 44,
  APPLICATION_ATOM_XML = 45,
  APPLICATION_XMPP_XML = 46,
  APPLICATION_EXI = 47,
  APPLICATION_X_BXML = 48,
  APPLICATION_FASTINFOSET = 49,
  APPLICATION_SOAP_FASTINFOSET = 50,
  APPLICATION_JSON = 51
} coap_content_type_t;

typedef union {
  struct { /* 0--14 bytes options */
    uint8_t length:4; /* option length in bytes (15 indicates long option format) */
    uint8_t delta:4;  /* option delta */
    /* 0--14 bytes options */
  } s;
  struct { /* 15-270 bytes options */
    uint16_t flag:4;   /* must be 15 */
    uint16_t delta:4;  /* option delta */
    uint16_t length:8;   /* length - 15 */
  } l;
} coap_header_option_t;

typedef struct {
  uint8_t *buffer; /* pointer to CoAP header / incoming packet buffer / memory to serialize packet */

  uint8_t version;
  coap_message_type_t type;
  uint8_t option_count;
  uint8_t code;
  uint16_t tid;

  uint16_t options;  /* Bitmap to check if option is set */

  coap_content_type_t content_type; /* Parse options once and store; allows setting options in random order  */
  uint32_t max_age;
  uint8_t etag_len;
  uint8_t etag[COAP_ETAG_LEN];
  uint8_t uri_host_len;
  const char *uri_host;
  uint8_t location_path_len;
  const char *location_path;
  uint8_t uri_path_len;
  const char *uri_path;
  uint32_t observe; /* 0-4 bytes for coap-03 */
  uint8_t token_len;
  uint8_t token[COAP_TOKEN_LEN];
  uint32_t block_num;
  uint8_t block_more;
  uint16_t block_size;
  uint32_t block_offset;
  uint8_t uri_query_len;
  const char *uri_query;

  uint16_t payload_len;
  uint8_t *payload;

} coap_packet_t;

/*error definitions*/
typedef enum
{
  NO_ERROR,

  /* Memory errors */
  MEMORY_ALLOCATION_ERROR,
  MEMORY_BOUNDARY_EXCEEDED,

  /* CoAP errors */
  UNKNOWN_CRITICAL_OPTION,
  PACKET_SERIALIZATION_ERROR
} error_t;

void coap_init_connection(uint16_t port);
uint16_t coap_get_tid(void);
void coap_send_message(uip_ipaddr_t *addr, uint16_t port, const uint8_t *data, uint16_t length);

void coap_init_message(void *packet, coap_message_type_t type, uint8_t code, uint16_t tid);
int coap_serialize_message(void *packet, uint8_t *buffer);
error_t coap_parse_message(void *request, uint8_t *data, uint16_t data_len);

coap_method_t coap_get_method(void *packet);
void coap_set_status(void *packet, unsigned int code);

int coap_get_query_variable(void *packet, const char *name, const char **output);
int coap_get_post_variable(void *packet, const char *name, const char **output);

unsigned int coap_get_header_content_type(void *packet);
int coap_set_header_content_type(void *packet, unsigned int content_type);

int coap_get_header_max_age(void *packet, uint32_t *age);
int coap_set_header_max_age(void *packet, uint32_t age);

int coap_get_header_etag(void *packet, const uint8_t **etag);
int coap_set_header_etag(void *packet, const uint8_t *etag, size_t etag_len);

int coap_get_header_uri_host(void *packet, const char **host); /*CAUTION in-place string might not be 0-terminated */
int coap_set_header_uri_host(void *packet, const char *host);

int coap_get_header_location(void *packet, const char **uri); /*CAUTION in-place string might not be 0-terminated */
int coap_set_header_location(void *packet, const char *uri);

int coap_get_header_uri_path(void *packet, const char **uri); /*CAUTION in-place string might not be 0-terminated */
int coap_set_header_uri_path(void *packet, const char *uri);

int coap_get_header_observe(void *packet, uint32_t *observe);
int coap_set_header_observe(void *packet, uint32_t observe);

int coap_get_header_token(void *packet, const uint8_t **token);
int coap_set_header_token(void *packet, const uint8_t *token, size_t token_len);

int coap_get_header_block(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset);
int coap_set_header_block(void *packet, uint32_t num, uint8_t more, uint16_t size);

int coap_get_header_uri_query(void *packet, const char **query); /*CAUTION in-place string might not be 0-terminated */
int coap_set_header_uri_query(void *packet, const char *query);

int coap_get_payload(void *packet, const uint8_t **payload);
int coap_set_payload(void *packet, const void *payload, size_t length);

#endif /* COAP_03_H_ */
