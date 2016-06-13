/*
 * Copyright (c) 2015, George Oikonomou - <george@contiki-os.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup ubidots-engine
 * @{
 */
/**
 * \file
 *    Implementation of the Ubidots client engine
 *
 * \author
 *    George Oikonomou - <george@contiki-os.org>,
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/process.h"
#include "net/ip/uipopt.h"
#include "net/ip/uip.h"
#include "net/ip/tcp-socket.h"
#include "net/ip/uiplib.h"
#include "net/ip/resolv.h"
#include "net/ipv6/uip-ds6.h"
#include "ubidots.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
/* Our own process' name */
PROCESS(ubidots_client_process, "Ubidots client process");
/*---------------------------------------------------------------------------*/
/**
 * \name Constants
 * @{
 */
#define RETRY_FOREVER 0xFF

#define DEFAULT_REMOTE_HOST  "things.ubidots.com"
#define DEFAULT_REMOTE_PORT                    80
#define DEFAULT_TCP_SEG_SIZE                   32
#define DEFAULT_BUF_LEN                       512
#define DEFAULT_CONNECT_ATTEMPTS    RETRY_FOREVER
#define DEFAULT_HOST_HEADER_LEN                64
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Ubidots client configuration
 * @{
 */
/**
 * Default Remote TCP endpoint:
 *
 * - hostname or
 * - IP address or (string representation)
 * - IPv6 address (string representation)
 */
#ifdef UBIDOTS_CONF_REMOTE_HOST
#define UBIDOTS_REMOTE_HOST UBIDOTS_CONF_REMOTE_HOST
#else
#define UBIDOTS_REMOTE_HOST DEFAULT_REMOTE_HOST
#endif

/** Default Remote TCP endpoint port */
#ifdef UBIDOTS_CONF_REMOTE_PORT
#define UBIDOTS_REMOTE_PORT UBIDOTS_CONF_REMOTE_PORT
#else
#define UBIDOTS_REMOTE_PORT DEFAULT_REMOTE_PORT
#endif

/**
 * \brief Maximum number of connection attempts
 *
 * Between 1 (only try once) and 0xFF (retry forever)
 */
#ifdef UBIDOTS_CONF_CONNECT_ATTEMPTS
#define UBIDOTS_CONNECT_ATTEMPTS UBIDOTS_CONF_CONNECT_ATTEMPTS
#else
#define UBIDOTS_CONNECT_ATTEMPTS DEFAULT_CONNECT_ATTEMPTS
#endif

/** Interval between two consecutive connection attempts in clock ticks */
#ifdef UBIDOTS_CONF_RECONNECT_INTERVAL
#define UBIDOTS_RECONNECT_INTERVAL UBIDOTS_CONF_RECONNECT_INTERVAL
#else
#define UBIDOTS_RECONNECT_INTERVAL (CLOCK_SECOND * 4)
#endif

/** Ubidots Auth Token */
#ifdef UBIDOTS_CONF_AUTH_TOKEN
#define UBIDOTS_AUTH_TOKEN UBIDOTS_CONF_AUTH_TOKEN
#else
#define UBIDOTS_AUTH_TOKEN
#error "Auth Token not defined."
#error "Set UBIDOTS_CONF_AUTH_TOKEN in project-conf.h"
#endif

/** Size of the HTTP reply buffer  */
#ifdef UBIDOTS_CONF_IN_BUFFER_SIZE
#define UBIDOTS_IN_BUFFER_SIZE UBIDOTS_CONF_IN_BUFFER_SIZE
#else
#define UBIDOTS_IN_BUFFER_SIZE DEFAULT_BUF_LEN
#endif

/** TCP connection output buffer size */
#ifdef UBIDOTS_CONF_OUT_BUFFER_SIZE
#define UBIDOTS_OUT_BUFFER_SIZE UBIDOTS_CONF_OUT_BUFFER_SIZE
#else
#define UBIDOTS_OUT_BUFFER_SIZE DEFAULT_BUF_LEN
#endif

/** Maximum size for outgoing TCP segments */
#ifdef UBIDOTS_CONF_MAX_TCP_SEG
#define UBIDOTS_MAX_TCP_SEG UBIDOTS_CONF_MAX_TCP_SEG
#else
#define UBIDOTS_MAX_TCP_SEG DEFAULT_TCP_SEG_SIZE
#endif

/** DNS support. Only if UDP is also enabled. */
#ifdef UBIDOTS_CONF_DNS_SUPPORT
#define UBIDOTS_DNS_SUPPORT (UBIDOTS_CONF_DNS_SUPPORT && UIP_UDP)
#else
#define UBIDOTS_DNS_SUPPORT UIP_UDP
#endif

/**
 * Size of the buffer which will be used to store the content of the HTTP
 * Host: header.
 */
#ifdef UBIDOTS_CONF_HOST_HEADER_LEN
#define UBIDOTS_HOST_HEADER_LEN UBIDOTS_CONF_HOST_HEADER_LEN
#else
#define UBIDOTS_HOST_HEADER_LEN DEFAULT_HOST_HEADER_LEN
#endif

/**
 * The maximum reply buffer size. This buffer will hold HTTP reply headers as
 * well as the actual payload. If the payload exceeds this buffer, it will
 * be truncated. The buffer will be null-terminated.
 */
#ifdef UBIDOTS_CONF_REPLY_PAYLOAD_MAX_LEN
#define UBIDOTS_REPLY_PAYLOAD_MAX_LEN UBIDOTS_CONF_REPLY_PAYLOAD_MAX_LEN
#else
#define UBIDOTS_REPLY_PAYLOAD_MAX_LEN 256
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/* IP address conversions */
#if NETSTACK_CONF_WITH_IPV6
#define ip_addr_conv(addrstr, ipaddr) uiplib_ip6addrconv(addrstr, ipaddr)
#else
#define ip_addr_conv(addrstr, ipaddr) uiplib_ip4addrconv(addrstr, ipaddr)
#endif
/*---------------------------------------------------------------------------*/
/* Connection states */
#define STATE_STARTING                0
#define STATE_DISCONNECTED            1 /* When triggered by the remote end */
#define STATE_RESOLVING               2
#define STATE_TCP_CONNECT             3
#define STATE_TCP_CONNECTED           4 /* Only just established */
#define STATE_ESTABLISHED             5 /* Connection Stable */
#define STATE_POSTING                 6

/* Soft 'error' states (0x4x) */
#define STATE_ERROR_NO_NET         0x41 /* Looking for a network */

/* Hard error states: Ones we cannot recover from (0x8x) */

/* Reached maximum attempt count */
#define STATE_ERROR_MAX_ATTEMPTS   0x8C
/*
 * We cannot convert UBIDOTS_REMOTE_HOST to an IP or IPv6 address and we do
 * not have UDP support, therefore we cannot resolve it either. This is fatal
 */
#define STATE_ERROR_INVALID_REMOTE 0x8D

/*
 * Our call to the TCP sockets API returned an error that we don't know how to
 * handle
 */
#define STATE_ERROR_TCP_SOCKET     0x8E

/* Unknown error state */
#define STATE_ERROR_UNKNOWN        0x8F
/*---------------------------------------------------------------------------*/
/* Periodic polling our own process */
#define STATE_MACHINE_PERIODIC (CLOCK_SECOND >> 1)
struct etimer periodic;
/*---------------------------------------------------------------------------*/
/* Events visible externally */
process_event_t ubidots_event_disconnected;
process_event_t ubidots_event_established;
process_event_t ubidots_event_post_sent;
process_event_t ubidots_event_post_reply_received;
/*---------------------------------------------------------------------------*/
/*
 * Headers to send to the remote with the HTTP request
 * Ubidots seems tolerant if we skip User-Agent: and Accept: so we choose to not
 * send those. However, definitions stay here in case someone wants to
 * include them in the future
 */
#define HEADER_HTTP_REQUEST_V  "POST /api/v" UBIDOTS_API_VER "/variables/%s/values HTTP/1.1\r\n"
#define HEADER_HTTP_REQUEST_C  "POST /api/v" UBIDOTS_API_VER "/collections/values HTTP/1.1\r\n"
#define HEADER_HOST            "Host: %s\r\n"
#define HEADER_CONTENT_TYPE    "Content-Type: application/json\r\n"
#define HEADER_X_AUTH_TOKEN    "X-Auth-Token: " UBIDOTS_AUTH_TOKEN "\r\n"
#define HEADER_CONTENT_LENGTH  "Content-Length:      \r\n"

/* Optional */
#define HEADER_USER_AGENT      "User-Agent: Contiki OS Ubidots Client\r\n"
#define HEADER_ACCEPT          "Accept: application/json\r\n"

/* The length of the "Content-Length: " string (without the double quotes) */
#define HEADER_CONTENT_LEN_LEN    16
#define HEADER_CONTENT_LEN_STR_LEN 5
/*---------------------------------------------------------------------------*/
#define UBIDOTS_DEBUG "Ubidots client: "
/*---------------------------------------------------------------------------*/
/* Temporary buffer used to enqueue a string */
#define TMP_BUF_SIZE 128
/*---------------------------------------------------------------------------*/
#define MIN(a, b) ((a) < (b) ? (a) : (b))
/*---------------------------------------------------------------------------*/
/* HTTP Reply Parsing */
/* Reply parser states */
#define REPLY_PARSER_STATE_READING_STATUS      0
#define REPLY_PARSER_STATE_NEW_HEADER          1
#define REPLY_PARSER_STATE_READING_HEADER      2
#define REPLY_PARSER_STATE_READING_CHUNK_LEN   3
#define REPLY_PARSER_STATE_READING_PAYLOAD     4
#define REPLY_PARSER_STATE_NOTIFYING_CONSUMER  5
#define REPLY_PARSER_STATE_DONE                6
#define REPLY_PARSER_STATE_ERROR            0xFF
/*---------------------------------------------------------------------------*/
/**
 * A data structure maintaining the connection state. It will not be used
 * by the consumer, hence only declared and used internally
 */
typedef struct ubidots_connection {
  /**
   * Incoming data buffer. This will be used by the sockets library to
   * store incoming data
   */
  uint8_t in_buf[UBIDOTS_IN_BUFFER_SIZE];

  /** Position to write the next byte in the in buffer */
  uint16_t in_buf_pos;

  /** Outgoing data buffer */
  uint8_t out_buf[UBIDOTS_OUT_BUFFER_SIZE];

  /** Position to write the next byte in the out buffer */
  uint16_t out_buf_pos;

  /** The byte where the outgoing HTTP payload starts */
  uint16_t payload_start_pos;

  /**
   * The position in the out buffer, where the value for Content-Length: will
   * be stored
   */
  uint16_t content_length_pos;

  /** The TCP socket used to connect to the remote */
  struct tcp_socket sock;

  /** The remote's IP or IPv6 address after DNS resolution and conversion. */
  uip_ipaddr_t remote_ip;

  /**
   * The content of the outgoing HTTP Host: header. Calculated only once and
   * stored here
   */
  char host_header[UBIDOTS_HOST_HEADER_LEN];

  /** Pointer to the consumer process */
  struct process *consumer;

  /** String array of headers that the consumer wants to be notified about */
  const char **headers;

  char reply_payload[UBIDOTS_REPLY_PAYLOAD_MAX_LEN];

  /**
   * Internal temporary storage
   */
  char reply_tmp_buf[8];

  /**
   * Counter of bytes stored in reply_tmp_buf
   */
  uint8_t reply_tmp_buf_counter;

  /**
   * Internal counter with the number of bytes received for the current reply
   */
  uint16_t reply_counter;

  /**
   * Length of the last HTTP chunk as per Transfer-Encoding: chunked
   */
  int reply_chunk_len;

  /** Internal state of the HTTP reply parser */
  uint8_t reply_parser_state;

  /** Connection state */
  uint8_t state;

  /** Connection attempt counter */
  uint8_t connection_attempt;
} ubidots_connection_t;

static ubidots_connection_t conn;
/*---------------------------------------------------------------------------*/
static int
enqueue_chunk(const char *format, ...)
{
  int tmp_buf_len;
  char tmp_buf[TMP_BUF_SIZE];
  va_list ap;

  va_start(ap, format);

  tmp_buf_len = vsnprintf(tmp_buf, TMP_BUF_SIZE, format, ap);

  if(tmp_buf_len < 0 ||
     tmp_buf_len >= UBIDOTS_OUT_BUFFER_SIZE - conn.out_buf_pos - 1) {
    return UBIDOTS_ERROR;
  }

  memcpy(&conn.out_buf[conn.out_buf_pos], tmp_buf, tmp_buf_len);
  conn.out_buf_pos += tmp_buf_len;

  return UBIDOTS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static void
notify_consumer(ubidots_reply_part_t *reply)
{
  PROCESS_CONTEXT_BEGIN(&ubidots_client_process);

  process_post_synch(conn.consumer, ubidots_event_post_reply_received, reply);

  PROCESS_CONTEXT_END(&ubidots_client_process);
}
/*---------------------------------------------------------------------------*/
/* Callbacks */

/*
 * Data input callback. Here, we process the reply from the Ubidots server.
 *
 * We parse the reply as it arrives, notifying the sockets library that we
 * consumed all input each time. We store the HTTP return code in a separate
 * field, HTTP reply headers in a MEMB/LIST structure, and the payload as-is in
 * a buffer.
 *
 * Ubidots will send us Transfer-Encoding: chunked. When we have parsed the
 * final chunk, we generate an event so that the consumer process can see
 * the reply.
 */
static int
data_callback(struct tcp_socket *s, void *ptr, const uint8_t *input_data_ptr,
              int input_data_len)
{
  ubidots_reply_part_t reply;
  long int http_status;
  const char **hdr;
  const char *buf_ptr = (const char *)input_data_ptr;

  if(strncmp((const char *)input_data_ptr, "HTTP/",
             MIN(5, input_data_len)) == 0) {

    /* Reset the reply parser */
    conn.reply_parser_state = REPLY_PARSER_STATE_READING_STATUS;
  }

  while(buf_ptr < (const char *)(&input_data_ptr[input_data_len])) {
    switch(conn.reply_parser_state) {
    case REPLY_PARSER_STATE_READING_STATUS:
      /*
       * First chunk. Initialise the reply buffer and then retrieve the HTTP
       * status code
       */
      memset(&conn.reply_payload, 0, UBIDOTS_REPLY_PAYLOAD_MAX_LEN);

      conn.reply_counter = 0;

      /*
       * The first message will always start with something like:
       * HTTP/1.x 200 OK
       *
       * Status always starts at the 10th char
       */
      http_status = strtol((char *)(&input_data_ptr[9]), NULL, 10);
      reply.type = UBIDOTS_REPLY_TYPE_HTTP_STATUS;
      reply.content = &http_status;

      PRINTF(UBIDOTS_DEBUG "HTTP Reply %lu\n", http_status);

      notify_consumer(&reply);

      /* Sink the remainder of the status line */
      buf_ptr = (const char *)(&input_data_ptr[9]);
      while(*buf_ptr != '\n') {
        buf_ptr++;
      }
      buf_ptr++;
      conn.reply_parser_state = REPLY_PARSER_STATE_NEW_HEADER;
      break;
    case REPLY_PARSER_STATE_NEW_HEADER:
      /*
       * First, check that this is really a header and not the start of the
       * payload. If it's the start of the payload, bail out and prepare to
       * read a chunk.
       *
       * We actually have to check both chars and increment by one each time,
       * otherwise we risk failing if the data in the socket's input buffer
       * ends after the \r
       *
       * We change state when we have encountered \n
       */
      if(*buf_ptr == '\r' || *buf_ptr == '\n') {
        if(*buf_ptr == '\n') {
          conn.reply_parser_state = REPLY_PARSER_STATE_READING_CHUNK_LEN;
          memset(conn.reply_tmp_buf, 0, sizeof(conn.reply_tmp_buf));
          memset(conn.reply_payload, 0, UBIDOTS_REPLY_PAYLOAD_MAX_LEN);
          conn.reply_tmp_buf_counter = 0;
        }
        buf_ptr++;
        break;
      }

      PRINTF(UBIDOTS_DEBUG "New header: <");

      /*
       * Zero-out the buffer. We subsequently make sure to leave at least one
       * free byte, so that takes care of null-termination
       */
      memset(conn.reply_payload, 0, UBIDOTS_REPLY_PAYLOAD_MAX_LEN);

      /* And prepare to populate it */
      conn.reply_counter = 0;
      conn.reply_parser_state = REPLY_PARSER_STATE_READING_HEADER;
      break;
    case REPLY_PARSER_STATE_READING_HEADER:
      if(*buf_ptr != '\r' && *buf_ptr != '\n') {
        /* Truncate if length exceeded */
        PRINTF("%c", *buf_ptr);
        if(conn.reply_counter < UBIDOTS_REPLY_PAYLOAD_MAX_LEN - 1) {
          conn.reply_payload[conn.reply_counter] = *buf_ptr;
          conn.reply_counter++;
        }
      } else {
        if(*buf_ptr == '\n') {
          PRINTF(">\n");
          conn.reply_counter = 0;
          conn.reply_parser_state = REPLY_PARSER_STATE_NEW_HEADER;

          hdr = conn.headers;
          while(*hdr) {
            if(strncmp(*hdr, conn.reply_payload, strlen(*hdr)) == 0) {
              PRINTF(UBIDOTS_DEBUG "Client wants header '%s'\n", *hdr);
              reply.type = UBIDOTS_REPLY_TYPE_HTTP_HEADER;
              reply.content = conn.reply_payload;
              notify_consumer(&reply);
              break;
            }
            hdr++;
          }
        }
      }
      buf_ptr++;
      break;
    case REPLY_PARSER_STATE_READING_CHUNK_LEN:
      /* Discard \r, read chunk length after \n */
      if(*buf_ptr != '\r' && *buf_ptr != '\n') {
        conn.reply_tmp_buf[conn.reply_tmp_buf_counter] = *buf_ptr;
        conn.reply_tmp_buf_counter++;
      } else {
        if(*buf_ptr == '\n') {
          conn.reply_chunk_len = 0;
          /* Read out hex length for this chunk */
          conn.reply_parser_state = REPLY_PARSER_STATE_READING_PAYLOAD;
          conn.reply_chunk_len = strtol(conn.reply_tmp_buf, NULL, 16);

          PRINTF(UBIDOTS_DEBUG "Chunk, len %d: <", conn.reply_chunk_len);

          /* Wipe temporary buffer for next pass */
          memset(conn.reply_tmp_buf, 0, sizeof(conn.reply_tmp_buf));

          /* Check for last chunk */
          if(conn.reply_chunk_len == 0) {
            conn.reply_parser_state = REPLY_PARSER_STATE_DONE;
            PRINTF("(End of Reply)> (Payload Length %u bytes)\n",
                   conn.reply_counter);

            reply.type = UBIDOTS_REPLY_TYPE_PAYLOAD;
            reply.content = conn.reply_payload;
            notify_consumer(&reply);
          }
        }
      }
      buf_ptr++;
      break;
    case REPLY_PARSER_STATE_READING_PAYLOAD:
      if(conn.reply_chunk_len > 0 &&
         conn.reply_counter < UBIDOTS_REPLY_PAYLOAD_MAX_LEN - 1) {
        PRINTF("%c", *buf_ptr);
        conn.reply_payload[conn.reply_counter] = *buf_ptr;
        conn.reply_counter++;
        conn.reply_chunk_len--;
      } else {
        /* Done with this chunk */
        PRINTF("> (counter = %u)\n", conn.reply_counter);
        conn.reply_parser_state = REPLY_PARSER_STATE_READING_CHUNK_LEN;
      }
      buf_ptr++;
      break;
    case REPLY_PARSER_STATE_DONE:
    case REPLY_PARSER_STATE_ERROR:
    default:
      /*
       * Sink the remainder if we reach here due to some error or because
       * we're done parsing
       */
      buf_ptr++;
      break;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
event_callback(struct tcp_socket *s, void *ptr, tcp_socket_event_t event)
{
  switch(event) {
  case TCP_SOCKET_CONNECTED:
    PRINTF("event_callback: connected\n");
    conn.state = STATE_TCP_CONNECTED;
    break;
  case TCP_SOCKET_CLOSED:
  case TCP_SOCKET_TIMEDOUT:
  case TCP_SOCKET_ABORTED:
    PRINTF("event_callback: disconnected\n");
    conn.state = STATE_DISCONNECTED;
    break;
  case TCP_SOCKET_DATA_SENT:
  default:
    break;
  }

  process_poll(&ubidots_client_process);
}
/*---------------------------------------------------------------------------*/
static int
tcp_conn_connect(void)
{
  PRINTF(UBIDOTS_DEBUG "Connect ");
  uip_debug_ipaddr_print(&(conn.remote_ip));
  PRINTF(" port %u\n", UBIDOTS_REMOTE_PORT);

  return tcp_socket_connect(&(conn.sock), &(conn.remote_ip),
                            UBIDOTS_REMOTE_PORT);
}
/*---------------------------------------------------------------------------*/
static int
http_prepare_header(const char *variable)
{
  /* If variable == NULL, we are sending a collection */
  if(variable == NULL) {
    if(enqueue_chunk(HEADER_HTTP_REQUEST_C) == UBIDOTS_ERROR) {
      return UBIDOTS_ERROR;
    }
  } else {
    if(enqueue_chunk(HEADER_HTTP_REQUEST_V, variable) == UBIDOTS_ERROR) {
      return UBIDOTS_ERROR;
    }
  }

  /* Send Host: if host_header has a valid string */
  if(conn.host_header[0] != 0) {
    if(enqueue_chunk(HEADER_HOST, conn.host_header) == UBIDOTS_ERROR) {
      return UBIDOTS_ERROR;
    }
  }

  if(enqueue_chunk(HEADER_CONTENT_TYPE) == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  /*
   * Content-Length: Business as usual
   * We have no idea how much it'll be... We can either put the entire
   * payload in a separate buffer, count it, then create the HTTP POST
   * Or we can skip the secondary buffer and leave Content-Length empty to
   * start with, until just before we send out the POST. We go for the latter
   * here to save RAM.
   *
   * We do know that the actual value will be written at the current buffer
   * position, plus the length of 'Content-Length: ' (16)
   * We also know that it can't really be longer than say 5 bytes. So we
   * write 5 spaces for now and we adjust later with the actual value.
   */
  conn.content_length_pos = conn.out_buf_pos + HEADER_CONTENT_LEN_LEN;
  if(enqueue_chunk(HEADER_CONTENT_LENGTH) == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  if(enqueue_chunk(HEADER_X_AUTH_TOKEN "\r\n") == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  return UBIDOTS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/* Encode the hostname to be used as the value for the HTTP Host: header */
static void
encode_http_host(uint8_t host_is_ip_address)
{
  int remaining = UBIDOTS_HOST_HEADER_LEN;
  int host_len;

  if(host_is_ip_address && NETSTACK_CONF_WITH_IPV6) {
    host_len = snprintf(conn.host_header, remaining,
                        "[%s]", UBIDOTS_REMOTE_HOST);
  } else {
    host_len = snprintf(conn.host_header, remaining,
                        "%s", UBIDOTS_REMOTE_HOST);
  }

  if(host_len < 0 || host_len >= remaining) {
    printf(UBIDOTS_DEBUG "Error populating hostname buffer\n");
    memset(conn.host_header, 0, UBIDOTS_HOST_HEADER_LEN);
    return;
  }

  remaining -= host_len;

  if(UBIDOTS_REMOTE_PORT != 80) {
    host_len = snprintf(&conn.host_header[host_len], remaining,
                        ":%u", UBIDOTS_REMOTE_PORT);

    if(host_len < 0 || host_len >= remaining) {
      printf(UBIDOTS_DEBUG "Error populating hostname buffer\n");
      memset(conn.host_header, 0, UBIDOTS_HOST_HEADER_LEN);
      return;
    }
    remaining -= host_len;
  }

  PRINTF(UBIDOTS_DEBUG "'Host: %s' (remaining %u)\n", conn.host_header,
         remaining);
}
/*---------------------------------------------------------------------------*/
static void
ubidots_conn_reset(void)
{
  memset(conn.in_buf, 0, UIP_TCP_MSS);
  conn.in_buf_pos = 0;

  memset(conn.out_buf, 0, UBIDOTS_OUT_BUFFER_SIZE);
  conn.out_buf_pos = 0;

  conn.connection_attempt = 0;
  conn.state = STATE_ERROR_NO_NET;

  tcp_socket_register(&(conn.sock), NULL, conn.in_buf, UIP_TCP_MSS,
                      conn.out_buf, UBIDOTS_OUT_BUFFER_SIZE,
                      data_callback, event_callback);
}
/*---------------------------------------------------------------------------*/
static void
ubidots_conn_teardown(void)
{
  tcp_socket_unregister(&(conn.sock));

  memset(&conn, 0, sizeof(conn));
}
/*---------------------------------------------------------------------------*/
static int
have_network(void)
{
#if NETSTACK_CONF_WITH_IPV6
  /* For v6 builds, wait until we have a usable v6 address */
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
    return 0;
  }
#endif
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
#if UBIDOTS_DNS_SUPPORT
  uip_ipaddr_t *tmp_addr_ptr = NULL;
  resolv_status_t resolv_status;
#endif

  switch(conn.state) {
  case STATE_ERROR_NO_NET:
    PRINTF(UBIDOTS_DEBUG "STATE_ERROR_NO_NET\n");
    if(have_network() == 0) {
      break;
    }

    conn.state = STATE_STARTING;
  /* Continue */
  case STATE_STARTING:
    PRINTF(UBIDOTS_DEBUG "STATE_STARTING\n");
    conn.connection_attempt = 0;

    /*
     * First, check whether UBIDOTS_REMOTE_HOST is a string representation of
     * an IP address. If not, we need to resolve it over DNS
     */
    PRINTF(UBIDOTS_DEBUG "Checking " UBIDOTS_REMOTE_HOST "\n");

    if(ip_addr_conv(UBIDOTS_REMOTE_HOST, &conn.remote_ip) == 0) {
#if UBIDOTS_DNS_SUPPORT
      /*
       * The address was not parsed, so we assume it is a hostname. Enter
       * STATE_RESOLVING to attempt resolution over DNS
       */
      conn.state = STATE_RESOLVING;

      /* Populate the buffer which will be used for the HTTP Host: header */
      encode_http_host(0);
#else
      /*
       * Could not convert UBIDOTS_REMOTE_HOST to an IP address and we do not
       * have UDP support, therefore we cannot resolve over DNS.
       *
       * This is fatal and we cannot recover. The process will exit.
       */
      conn.state = STATE_ERROR_INVALID_REMOTE;
#endif
    } else {
      conn.state = STATE_TCP_CONNECT;

      /* Populate the buffer which will be used for the HTTP Host: header */
      encode_http_host(1);
    }

    /*
     * If we reach here, we can be in one of the following states:
     * - STATE_RESOLVING
     * - STATE_TCP_CONNECT
     * - STATE_ERROR_INVALID_REMOTE
     * In all cases, continue immediately
     */
    process_poll(&ubidots_client_process);
    return;
  case STATE_DISCONNECTED:
    PRINTF(UBIDOTS_DEBUG "STATE_DISCONNECTED\n");
    process_post(conn.consumer, ubidots_event_disconnected, NULL);
    etimer_set(&periodic, UBIDOTS_RECONNECT_INTERVAL);
    conn.state = STATE_TCP_CONNECT;
    return;
#if UBIDOTS_DNS_SUPPORT
  /* This state is only relevant if we have DNS support */
  case STATE_RESOLVING:
    resolv_status = resolv_lookup(UBIDOTS_REMOTE_HOST, &tmp_addr_ptr);

    PRINTF(UBIDOTS_DEBUG "STATE_RESOLVING (%u)\n", resolv_status);

    if(resolv_status == RESOLV_STATUS_CACHED) {
      /*
       * An entry for the hostname was found in the resolver's cache, and
       * it's fresh and usable. Copy over and continue
       */
      PRINTF(UBIDOTS_DEBUG "Entry for " UBIDOTS_REMOTE_HOST " cached and usable.\n");
      memcpy(&conn.remote_ip, tmp_addr_ptr, sizeof(uip_ipaddr_t));
      conn.state = STATE_TCP_CONNECT;
      process_poll(&ubidots_client_process);
      return;
    } else if(resolv_status == RESOLV_STATUS_EXPIRED ||
              resolv_status == RESOLV_STATUS_UNCACHED) {
      PRINTF(UBIDOTS_DEBUG "Resolving " UBIDOTS_REMOTE_HOST "\n");
      resolv_query(UBIDOTS_REMOTE_HOST);
    }

    /* RESOLV_STATUS_RESOLVING, _NOT_FOUND, _ERROR: wait */
    break;
#endif
  case STATE_TCP_CONNECT:
    conn.connection_attempt++;

    PRINTF(UBIDOTS_DEBUG "STATE_TCP_CONNECT (%u)\n", conn.connection_attempt);

    if(conn.connection_attempt <= UBIDOTS_CONNECT_ATTEMPTS ||
       UBIDOTS_CONNECT_ATTEMPTS == RETRY_FOREVER) {
      if(tcp_conn_connect() == 1) {
        /*
         * Here we simply return and we wait for the TCP socket to generate
         * an event for us. event_callback() will put us in the correct state
         * and poll us.
         */
        return;
      } else {
        /*
         * tcp_conn_connect() returned an error, this means that
         * tcp_socket_connect() returned an error. This is fatal.
         */
        conn.state = STATE_ERROR_TCP_SOCKET;
      }
    } else {
      /* Exceeded max connection attempts */
      conn.state = STATE_ERROR_MAX_ATTEMPTS;
    }
    break;
  case STATE_TCP_CONNECTED:
    PRINTF(UBIDOTS_DEBUG "STATE_TCP_CONNECTED\n");

    /*
     * Set maximum size for this socket's outgoing TCP segments
     * Low: Avoid or decrease 6lowpan fragmentation
     * High: Reduce TCP data/ACK back and forth
     */
    conn.sock.output_data_max_seg = UBIDOTS_MAX_TCP_SEG;
    conn.connection_attempt = 0;
    conn.state = STATE_ESTABLISHED;

    process_post(conn.consumer, ubidots_event_established, NULL);
    break;
  case STATE_ESTABLISHED:
    /* Idle state. We wait for the consumer to do something */
    break;
  case STATE_POSTING:
    PRINTF(UBIDOTS_DEBUG "STATE_POSTING (%u)\n", conn.sock.output_data_len);
    if(conn.sock.output_data_len == 0) {
      process_post(conn.consumer, ubidots_event_post_sent, NULL);
      conn.state = STATE_ESTABLISHED;
    }
    break;
  case STATE_ERROR_MAX_ATTEMPTS:
  case STATE_ERROR_TCP_SOCKET:
  case STATE_ERROR_INVALID_REMOTE:
  case STATE_ERROR_UNKNOWN:
  default:
    printf("State 0x%02x. This is fatal and the client will exit.\n",
           conn.state);
    /*
     * These are all hard error states that we cannot recover from.
     * Default should never happen
     */
    ubidots_conn_teardown();
    ubidots_conn_reset();
    process_exit(&ubidots_client_process);
    return;
  }

  etimer_set(&periodic, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
int
ubidots_enqueue_value(const char *variable, const char *value)
{
  /* Add a JSON comma separator, except before the opening { */
  if(conn.out_buf[conn.out_buf_pos - 1] != '[') {
    if(enqueue_chunk(",") == UBIDOTS_ERROR) {
      return UBIDOTS_ERROR;
    }
  }

  if(enqueue_chunk("{") == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  if(variable) {
    if(enqueue_chunk("\"variable\":\"%s\",", variable) == UBIDOTS_ERROR) {
      return UBIDOTS_ERROR;
    }
  }

  if(enqueue_chunk("\"value\":%s}", value) == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  PRINTF(UBIDOTS_DEBUG "Enqueue value: Buffer at %u\n", conn.out_buf_pos);

  return UBIDOTS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
ubidots_prepare_post(const char *variable)
{
  if(conn.state != STATE_ESTABLISHED) {
    return UBIDOTS_ERROR;
  }

  conn.in_buf_pos = 0;
  conn.out_buf_pos = 0;

  if(http_prepare_header(variable) == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  conn.payload_start_pos = conn.out_buf_pos;

  if(enqueue_chunk("[") == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  PRINTF(UBIDOTS_DEBUG "Prepare POST: Buffer at %u\n", conn.out_buf_pos);

  return UBIDOTS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
ubidots_post()
{
  uint16_t content_length;
  uint8_t content_len_len;
  uint16_t move_from;
  uint16_t move_to;
  uint16_t move_count;

  if(enqueue_chunk("]") == UBIDOTS_ERROR) {
    return UBIDOTS_ERROR;
  }

  /* Calculate and fill in Content-Length: */
  content_length = conn.out_buf_pos - conn.payload_start_pos;
  content_len_len = snprintf((char *)(&conn.out_buf[conn.content_length_pos]),
                             HEADER_CONTENT_LEN_STR_LEN, "%u", content_length);

  PRINTF(UBIDOTS_DEBUG "POST: Buffer at %u, content-length %u (%u), at %u\n",
         conn.out_buf_pos, content_length, content_len_len,
         conn.content_length_pos);

  /*
   * Pedantry: Slide the remainder of the POST left, to remove the spaces
   * that may be trailing Content-Length:. snprintf will have also left a NULL
   * character in there, which we also want to get rid of in the process
   *
   * We occupied content_len_len of HEADER_CONTENT_LEN_STR_LEN bytes
   * Move:
   * from conn.content_length_pos + HEADER_CONTENT_LEN_STR_LEN
   * to conn.content_length_pos + content_len_len:
   * count: conn.out_buf_pos - conn.content_length_pos - (move_from - move_to)
   */
  if(content_len_len > 0 && content_len_len < HEADER_CONTENT_LEN_STR_LEN) {
    move_to = conn.content_length_pos + content_len_len;
    move_from = conn.content_length_pos + HEADER_CONTENT_LEN_STR_LEN;
    move_count = conn.out_buf_pos - conn.content_length_pos -
      (move_from - move_to);
    memmove((char *)(&conn.out_buf[move_to]),
            (char *)(&conn.out_buf[move_from]),
            move_count);
    conn.out_buf_pos -= HEADER_CONTENT_LEN_STR_LEN - content_len_len;
  }

  tcp_socket_send(&conn.sock, conn.out_buf, conn.out_buf_pos);

  conn.state = STATE_POSTING;

  PRINTF(UBIDOTS_DEBUG "POST: Buffer at %u\n", conn.out_buf_pos);

  return UBIDOTS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
void
ubidots_init(struct process *consumer, const char **headers)
{
  ubidots_event_disconnected = process_alloc_event();
  ubidots_event_established = process_alloc_event();
  ubidots_event_post_sent = process_alloc_event();
  ubidots_event_post_reply_received = process_alloc_event();

  conn.consumer = consumer;
  conn.headers = headers;

  ubidots_conn_reset();

  process_start(&ubidots_client_process, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ubidots_client_process, ev, data)
{
  PROCESS_BEGIN();

  state_machine();

  while(1) {

    PROCESS_YIELD();

    if((ev == PROCESS_EVENT_TIMER && data == &periodic) ||
       ev == PROCESS_EVENT_POLL ||
       ev == resolv_event_found) {
      state_machine();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
