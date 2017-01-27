/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup mqtt-engine
 * @{
 */
/**
 * \file
 *    Implementation of the Contiki MQTT engine
 *
 * \author
 *    Texas Instruments
 */
/*---------------------------------------------------------------------------*/
#include "mqtt.h"
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/pt.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "dev/leds.h"

#include "tcp-socket.h"

#include "lib/assert.h"
#include "lib/list.h"
#include "sys/cc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
typedef enum {
  MQTT_FHDR_MSG_TYPE_CONNECT       = 0x10,
  MQTT_FHDR_MSG_TYPE_CONNACK       = 0x20,
  MQTT_FHDR_MSG_TYPE_PUBLISH       = 0x30,
  MQTT_FHDR_MSG_TYPE_PUBACK        = 0x40,
  MQTT_FHDR_MSG_TYPE_PUBREC        = 0x50,
  MQTT_FHDR_MSG_TYPE_PUBREL        = 0x60,
  MQTT_FHDR_MSG_TYPE_PUBCOMP       = 0x70,
  MQTT_FHDR_MSG_TYPE_SUBSCRIBE     = 0x80,
  MQTT_FHDR_MSG_TYPE_SUBACK        = 0x90,
  MQTT_FHDR_MSG_TYPE_UNSUBSCRIBE   = 0xA0,
  MQTT_FHDR_MSG_TYPE_UNSUBACK      = 0xB0,
  MQTT_FHDR_MSG_TYPE_PINGREQ       = 0xC0,
  MQTT_FHDR_MSG_TYPE_PINGRESP      = 0xD0,
  MQTT_FHDR_MSG_TYPE_DISCONNECT    = 0xE0,

  MQTT_FHDR_DUP_FLAG               = 0x08,

  MQTT_FHDR_QOS_LEVEL_0            = 0x00,
  MQTT_FHDR_QOS_LEVEL_1            = 0x02,
  MQTT_FHDR_QOS_LEVEL_2            = 0x04,

  MQTT_FHDR_RETAIN_FLAG            = 0x01,
} mqtt_fhdr_fields_t;
/*---------------------------------------------------------------------------*/
typedef enum {
  MQTT_VHDR_USERNAME_FLAG      = 0x80,
  MQTT_VHDR_PASSWORD_FLAG      = 0x40,

  MQTT_VHDR_WILL_RETAIN_FLAG   = 0x20,
  MQTT_VHDR_WILL_QOS_LEVEL_0   = 0x00,
  MQTT_VHDR_WILL_QOS_LEVEL_1   = 0x08,
  MQTT_VHDR_WILL_QOS_LEVEL_2   = 0x10,

  MQTT_VHDR_WILL_FLAG          = 0x04,
  MQTT_VHDR_CLEAN_SESSION_FLAG = 0x02,
} mqtt_vhdr_conn_fields_t;
/*---------------------------------------------------------------------------*/
typedef enum {
  MQTT_VHDR_CONN_ACCEPTED,
  MQTT_VHDR_CONN_REJECTED_PROTOCOL,
  MQTT_VHDR_CONN_REJECTED_IDENTIFIER,
  MQTT_VHDR_CONN_REJECTED_UNAVAILABLE,
  MQTT_VHDR_CONN_REJECTED_BAD_USER_PASS,
  MQTT_VHDR_CONN_REJECTED_UNAUTHORIZED,
} mqtt_vhdr_connack_fields_t;
/*---------------------------------------------------------------------------*/
#define MQTT_CONNECT_VHDR_FLAGS_SIZE 12

#define MQTT_STRING_LEN_SIZE 2
#define MQTT_MID_SIZE 2
#define MQTT_QOS_SIZE 1
/*---------------------------------------------------------------------------*/
#define RESPONSE_WAIT_TIMEOUT (CLOCK_SECOND * 10)
/*---------------------------------------------------------------------------*/
#define INCREMENT_MID(conn)   (conn)->mid_counter += 2
#define MQTT_STRING_LENGTH(s) (((s)->length) == 0 ? 0 : (MQTT_STRING_LEN_SIZE + (s)->length))
/*---------------------------------------------------------------------------*/
/* Protothread send macros */
#define PT_MQTT_WRITE_BYTES(conn, data, len)                                   \
  while(write_bytes(conn, data, len)) {                                        \
    PT_WAIT_UNTIL(pt, (conn)->out_buffer_sent);                                \
  }

#define PT_MQTT_WRITE_BYTE(conn, data)                                         \
  while(write_byte(conn, data)) {                                              \
    PT_WAIT_UNTIL(pt, (conn)->out_buffer_sent);                                \
  }
/*---------------------------------------------------------------------------*/
/*
 * Sends the continue send event and wait for that event.
 *
 * The reason we cannot use PROCESS_PAUSE() is since we would risk loosing any
 * events posted during the sending process.
 */
#define PT_MQTT_WAIT_SEND()                                                    \
  do {                                                                         \
    process_post(PROCESS_CURRENT(), mqtt_continue_send_event, NULL);           \
    PROCESS_WAIT_EVENT();                                                      \
    if(ev == mqtt_abort_now_event) {                                           \
      conn->state = MQTT_CONN_STATE_ABORT_IMMEDIATE;                           \
      PT_EXIT(&conn->out_proto_thread);                                        \
      process_post(PROCESS_CURRENT(), ev, data);                               \
    } else if(ev >= mqtt_event_min && ev <= mqtt_event_max) {                  \
      process_post(PROCESS_CURRENT(), ev, data);                               \
    }                                                                          \
  } while(0)
/*---------------------------------------------------------------------------*/
static process_event_t mqtt_do_connect_tcp_event;
static process_event_t mqtt_do_connect_mqtt_event;
static process_event_t mqtt_do_disconnect_mqtt_event;
static process_event_t mqtt_do_subscribe_event;
static process_event_t mqtt_do_unsubscribe_event;
static process_event_t mqtt_do_publish_event;
static process_event_t mqtt_do_pingreq_event;
static process_event_t mqtt_continue_send_event;
static process_event_t mqtt_abort_now_event;
process_event_t mqtt_update_event;

/*
 * Min and Max event numbers we want to acknowledge while we're in the process
 * of doing something else. continue_send does not count, therefore must be
 * allocated last
 */
static process_event_t mqtt_event_min;
static process_event_t mqtt_event_max;
/*---------------------------------------------------------------------------*/
/* Prototypes */
static int
tcp_input(struct tcp_socket *s, void *ptr, const uint8_t *input_data_ptr,
          int input_data_len);

static void tcp_event(struct tcp_socket *s, void *ptr,
                      tcp_socket_event_t event);

static void reset_packet(struct mqtt_in_packet *packet);
/*---------------------------------------------------------------------------*/
LIST(mqtt_conn_list);
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_process, "MQTT process");
/*---------------------------------------------------------------------------*/
static void
call_event(struct mqtt_connection *conn,
           mqtt_event_t event,
           void *data)
{
  conn->event_callback(conn, event, data);
  process_post(conn->app_process, mqtt_update_event, NULL);
}
/*---------------------------------------------------------------------------*/
static void
reset_defaults(struct mqtt_connection *conn)
{
  conn->mid_counter = 1;
  PT_INIT(&conn->out_proto_thread);
  conn->waiting_for_pingresp = 0;

  reset_packet(&conn->in_packet);
  conn->out_buffer_sent = 0;
}
/*---------------------------------------------------------------------------*/
static void
abort_connection(struct mqtt_connection *conn)
{
  conn->out_buffer_ptr = conn->out_buffer;
  conn->out_queue_full = 0;

  /* Reset outgoing packet */
  memset(&conn->out_packet, 0, sizeof(conn->out_packet));

  tcp_socket_close(&conn->socket);
  tcp_socket_unregister(&conn->socket);

  memset(&conn->socket, 0, sizeof(conn->socket));

  conn->state = MQTT_CONN_STATE_NOT_CONNECTED;
}
/*---------------------------------------------------------------------------*/
static void
connect_tcp(struct mqtt_connection *conn)
{
  conn->state = MQTT_CONN_STATE_TCP_CONNECTING;

  reset_defaults(conn);
  tcp_socket_register(&(conn->socket),
                      conn,
                      conn->in_buffer,
                      MQTT_TCP_INPUT_BUFF_SIZE,
                      conn->out_buffer,
                      MQTT_TCP_OUTPUT_BUFF_SIZE,
                      tcp_input,
                      tcp_event);
  tcp_socket_connect(&(conn->socket), &(conn->server_ip), conn->server_port);
}
/*---------------------------------------------------------------------------*/
static void
disconnect_tcp(struct mqtt_connection *conn)
{
  conn->state = MQTT_CONN_STATE_DISCONNECTING;
  tcp_socket_close(&(conn->socket));
  tcp_socket_unregister(&conn->socket);

  memset(&conn->socket, 0, sizeof(conn->socket));
}
/*---------------------------------------------------------------------------*/
static void
send_out_buffer(struct mqtt_connection *conn)
{
  if(conn->out_buffer_ptr - conn->out_buffer == 0) {
    conn->out_buffer_sent = 1;
    return;
  }
  conn->out_buffer_sent = 0;

  DBG("MQTT - (send_out_buffer) Space used in buffer: %i\n",
      conn->out_buffer_ptr - conn->out_buffer);

  tcp_socket_send(&conn->socket, conn->out_buffer,
                  conn->out_buffer_ptr - conn->out_buffer);
}
/*---------------------------------------------------------------------------*/
static void
string_to_mqtt_string(struct mqtt_string *mqtt_string, char *string)
{
  if(mqtt_string == NULL) {
    return;
  }
  mqtt_string->string = string;

  if(string != NULL) {
    mqtt_string->length = strlen(string);
  } else {
    mqtt_string->length = 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
write_byte(struct mqtt_connection *conn, uint8_t data)
{
  DBG("MQTT - (write_byte) buff_size: %i write: '%02X'\n",
      &conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr,
      data);

  if(&conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr == 0) {
    send_out_buffer(conn);
    return 1;
  }

  *conn->out_buffer_ptr = data;
  conn->out_buffer_ptr++;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
write_bytes(struct mqtt_connection *conn, uint8_t *data, uint16_t len)
{
  uint16_t write_bytes;
  write_bytes =
    MIN(&conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr,
        len - conn->out_write_pos);

  memcpy(conn->out_buffer_ptr, &data[conn->out_write_pos], write_bytes);
  conn->out_write_pos += write_bytes;
  conn->out_buffer_ptr += write_bytes;

  DBG("MQTT - (write_bytes) len: %u write_pos: %lu\n", len,
      conn->out_write_pos);

  if(len - conn->out_write_pos == 0) {
    conn->out_write_pos = 0;
    return 0;
  } else {
    send_out_buffer(conn);
    return len - conn->out_write_pos;
  }
}
/*---------------------------------------------------------------------------*/
static void
encode_remaining_length(uint8_t *remaining_length,
                        uint8_t *remaining_length_bytes,
                        uint32_t length)
{
  uint8_t digit;

  DBG("MQTT - Encoding length %lu\n", length);

  *remaining_length_bytes = 0;
  do {
    digit = length % 128;
    length = length / 128;
    if(length > 0) {
      digit = digit | 0x80;
    }

    remaining_length[*remaining_length_bytes] = digit;
    (*remaining_length_bytes)++;
    DBG("MQTT - Encode len digit '%u' length '%lu'\n", digit, length);
  } while(length > 0 && *remaining_length_bytes < 5);
  DBG("MQTT - remaining_length_bytes %u\n", *remaining_length_bytes);
}
/*---------------------------------------------------------------------------*/
static void
keep_alive_callback(void *ptr)
{
  struct mqtt_connection *conn = ptr;

  DBG("MQTT - (keep_alive_callback) Called!\n");

  /* The flag is set when the PINGREQ has been sent */
  if(conn->waiting_for_pingresp) {
    PRINTF("MQTT - Disconnect due to no PINGRESP from broker.\n");
    disconnect_tcp(conn);
    return;
  }

  process_post(&mqtt_process, mqtt_do_pingreq_event, conn);
}
/*---------------------------------------------------------------------------*/
static void
reset_packet(struct mqtt_in_packet *packet)
{
  memset(packet, 0, sizeof(struct mqtt_in_packet));
  packet->remaining_multiplier = 1;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(connect_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  DBG("MQTT - Sending CONNECT message...\n");

  /* Set up FHDR */
  conn->out_packet.fhdr = MQTT_FHDR_MSG_TYPE_CONNECT;
  conn->out_packet.remaining_length = 0;
  conn->out_packet.remaining_length += MQTT_CONNECT_VHDR_FLAGS_SIZE;
  conn->out_packet.remaining_length += MQTT_STRING_LENGTH(&conn->client_id);
  conn->out_packet.remaining_length += MQTT_STRING_LENGTH(&conn->credentials.username);
  conn->out_packet.remaining_length += MQTT_STRING_LENGTH(&conn->credentials.password);
  conn->out_packet.remaining_length += MQTT_STRING_LENGTH(&conn->will.topic);
  conn->out_packet.remaining_length += MQTT_STRING_LENGTH(&conn->will.message);
  encode_remaining_length(conn->out_packet.remaining_length_enc,
                          &conn->out_packet.remaining_length_enc_bytes,
                          conn->out_packet.remaining_length);
  if(conn->out_packet.remaining_length_enc_bytes > 4) {
    call_event(conn, MQTT_EVENT_PROTOCOL_ERROR, NULL);
    PRINTF("MQTT - Error, remaining length > 4 bytes\n");
    PT_EXIT(pt);
  }

  /* Write Fixed Header */
  PT_MQTT_WRITE_BYTE(conn, conn->out_packet.fhdr);
  PT_MQTT_WRITE_BYTES(conn,
                      conn->out_packet.remaining_length_enc,
                      conn->out_packet.remaining_length_enc_bytes);
  PT_MQTT_WRITE_BYTE(conn, 0);
  PT_MQTT_WRITE_BYTE(conn, 6);
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)MQTT_PROTOCOL_NAME, 6);
  PT_MQTT_WRITE_BYTE(conn, MQTT_PROTOCOL_VERSION);
  PT_MQTT_WRITE_BYTE(conn, conn->connect_vhdr_flags);
  PT_MQTT_WRITE_BYTE(conn, (conn->keep_alive >> 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->keep_alive & 0x00FF));
  PT_MQTT_WRITE_BYTE(conn, conn->client_id.length << 8);
  PT_MQTT_WRITE_BYTE(conn, conn->client_id.length & 0x00FF);
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->client_id.string,
                      conn->client_id.length);
  if(conn->connect_vhdr_flags & MQTT_VHDR_WILL_FLAG) {
    PT_MQTT_WRITE_BYTE(conn, conn->will.topic.length << 8);
    PT_MQTT_WRITE_BYTE(conn, conn->will.topic.length & 0x00FF);
    PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->will.topic.string,
                        conn->will.topic.length);
    PT_MQTT_WRITE_BYTE(conn, conn->will.message.length << 8);
    PT_MQTT_WRITE_BYTE(conn, conn->will.message.length & 0x00FF);
    PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->will.message.string,
                        conn->will.message.length);
    DBG("MQTT - Setting will topic to '%s' %u bytes and message to '%s' %u bytes\n",
        conn->will.topic.string,
        conn->will.topic.length,
        conn->will.message.string,
        conn->will.message.length);
  }
  if(conn->connect_vhdr_flags & MQTT_VHDR_USERNAME_FLAG) {
    PT_MQTT_WRITE_BYTE(conn, conn->credentials.username.length << 8);
    PT_MQTT_WRITE_BYTE(conn, conn->credentials.username.length & 0x00FF);
    PT_MQTT_WRITE_BYTES(conn,
                        (uint8_t *)conn->credentials.username.string,
                        conn->credentials.username.length);
  }
  if(conn->connect_vhdr_flags & MQTT_VHDR_PASSWORD_FLAG) {
    PT_MQTT_WRITE_BYTE(conn, conn->credentials.password.length << 8);
    PT_MQTT_WRITE_BYTE(conn, conn->credentials.password.length & 0x00FF);
    PT_MQTT_WRITE_BYTES(conn,
                        (uint8_t *)conn->credentials.password.string,
                        conn->credentials.password.length);
  }

  /* Send out buffer */
  send_out_buffer(conn);
  conn->state = MQTT_CONN_STATE_CONNECTING_TO_BROKER;

  timer_set(&conn->t, RESPONSE_WAIT_TIMEOUT);

  /* Wait for CONNACK */
  reset_packet(&conn->in_packet);
  PT_WAIT_UNTIL(pt, conn->out_packet.qos_state == MQTT_QOS_STATE_GOT_ACK ||
                timer_expired(&conn->t));
  if(timer_expired(&conn->t)) {
    DBG("Timeout waiting for CONNACK\n");
    /* We stick to the letter of the spec here: Tear the connection down */
    mqtt_disconnect(conn);
  }
  reset_packet(&conn->in_packet);

  DBG("MQTT - Done sending CONNECT\n");

#if DEBUG_MQTT == 1
  DBG("MQTT - CONNECT message sent: \n");
  uint16_t i;
  for(i = 0; i < (conn->out_buffer_ptr - conn->out_buffer); i++) {
    DBG("%02X ", conn->out_buffer[i]);
  }
  DBG("\n");
#endif

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(disconnect_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  PT_MQTT_WRITE_BYTE(conn, MQTT_FHDR_MSG_TYPE_DISCONNECT);
  PT_MQTT_WRITE_BYTE(conn, 0);

  send_out_buffer(conn);

  /*
   * Wait a couple of seconds for a TCP ACK. We don't really need the ACK,
   * we do want the TCP/IP stack to actually send this disconnect before we
   * tear down the session.
   */
  timer_set(&conn->t, (CLOCK_SECOND * 2));
  PT_WAIT_UNTIL(pt, conn->out_buffer_sent || timer_expired(&conn->t));

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(subscribe_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  DBG("MQTT - Sending subscribe message! topic %s topic_length %i\n",
      conn->out_packet.topic,
      conn->out_packet.topic_length);
  DBG("MQTT - Buffer space is %i \n",
      &conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr);

  /* Set up FHDR */
  conn->out_packet.fhdr = MQTT_FHDR_MSG_TYPE_SUBSCRIBE | MQTT_FHDR_QOS_LEVEL_1;
  conn->out_packet.remaining_length = MQTT_MID_SIZE +
    MQTT_STRING_LEN_SIZE +
    conn->out_packet.topic_length +
    MQTT_QOS_SIZE;
  encode_remaining_length(conn->out_packet.remaining_length_enc,
                          &conn->out_packet.remaining_length_enc_bytes,
                          conn->out_packet.remaining_length);
  if(conn->out_packet.remaining_length_enc_bytes > 4) {
    call_event(conn, MQTT_EVENT_PROTOCOL_ERROR, NULL);
    PRINTF("MQTT - Error, remaining length > 4 bytes\n");
    PT_EXIT(pt);
  }

  /* Write Fixed Header */
  PT_MQTT_WRITE_BYTE(conn, conn->out_packet.fhdr);
  PT_MQTT_WRITE_BYTES(conn,
                      conn->out_packet.remaining_length_enc,
                      conn->out_packet.remaining_length_enc_bytes);
  /* Write Variable Header */
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid << 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid & 0x00FF));
  /* Write Payload */
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length >> 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length & 0x00FF));
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->out_packet.topic,
                      conn->out_packet.topic_length);
  PT_MQTT_WRITE_BYTE(conn, conn->out_packet.qos);

  /* Send out buffer */
  send_out_buffer(conn);
  timer_set(&conn->t, RESPONSE_WAIT_TIMEOUT);

  /* Wait for SUBACK. */
  reset_packet(&conn->in_packet);
  PT_WAIT_UNTIL(pt, conn->out_packet.qos_state == MQTT_QOS_STATE_GOT_ACK ||
                timer_expired(&conn->t));

  if(timer_expired(&conn->t)) {
    DBG("Timeout waiting for SUBACK\n");
  }
  reset_packet(&conn->in_packet);

  /* This is clear after the entire transaction is complete */
  conn->out_queue_full = 0;

  DBG("MQTT - Done in send_subscribe!\n");

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(unsubscribe_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  DBG("MQTT - Sending unsubscribe message on topic %s topic_length %i\n",
      conn->out_packet.topic,
      conn->out_packet.topic_length);
  DBG("MQTT - Buffer space is %i \n",
      &conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr);

  /* Set up FHDR */
  conn->out_packet.fhdr = MQTT_FHDR_MSG_TYPE_UNSUBSCRIBE |
    MQTT_FHDR_QOS_LEVEL_1;
  conn->out_packet.remaining_length = MQTT_MID_SIZE +
    MQTT_STRING_LEN_SIZE +
    conn->out_packet.topic_length;
  encode_remaining_length(conn->out_packet.remaining_length_enc,
                          &conn->out_packet.remaining_length_enc_bytes,
                          conn->out_packet.remaining_length);
  if(conn->out_packet.remaining_length_enc_bytes > 4) {
    call_event(conn, MQTT_EVENT_PROTOCOL_ERROR, NULL);
    PRINTF("MQTT - Error, remaining length > 4 bytes\n");
    PT_EXIT(pt);
  }

  /* Write Fixed Header */
  PT_MQTT_WRITE_BYTE(conn, conn->out_packet.fhdr);
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->out_packet.remaining_length_enc,
                      conn->out_packet.remaining_length_enc_bytes);
  /* Write Variable Header */
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid << 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid & 0x00FF));
  /* Write Payload */
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length >> 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length & 0x00FF));
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->out_packet.topic,
                      conn->out_packet.topic_length);

  /* Send out buffer */
  send_out_buffer(conn);
  timer_set(&conn->t, RESPONSE_WAIT_TIMEOUT);

  /* Wait for UNSUBACK */
  reset_packet(&conn->in_packet);
  PT_WAIT_UNTIL(pt, conn->out_packet.qos_state == MQTT_QOS_STATE_GOT_ACK ||
                timer_expired(&conn->t));

  if(timer_expired(&conn->t)) {
    DBG("Timeout waiting for UNSUBACK\n");
  }

  reset_packet(&conn->in_packet);

  /* This is clear after the entire transaction is complete */
  conn->out_queue_full = 0;

  DBG("MQTT - Done writing subscribe message to out buffer!\n");

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(publish_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  DBG("MQTT - Sending publish message! topic %s topic_length %i\n",
      conn->out_packet.topic,
      conn->out_packet.topic_length);
  DBG("MQTT - Buffer space is %i \n",
      &conn->out_buffer[MQTT_TCP_OUTPUT_BUFF_SIZE] - conn->out_buffer_ptr);

  /* Set up FHDR */
  conn->out_packet.fhdr = MQTT_FHDR_MSG_TYPE_PUBLISH |
    conn->out_packet.qos << 1;
  if(conn->out_packet.retain == MQTT_RETAIN_ON) {
    conn->out_packet.fhdr |= MQTT_FHDR_RETAIN_FLAG;
  }
  conn->out_packet.remaining_length = MQTT_STRING_LEN_SIZE +
    conn->out_packet.topic_length +
    conn->out_packet.payload_size;
  if(conn->out_packet.qos > MQTT_QOS_LEVEL_0) {
    conn->out_packet.remaining_length += MQTT_MID_SIZE;
  }
  encode_remaining_length(conn->out_packet.remaining_length_enc,
                          &conn->out_packet.remaining_length_enc_bytes,
                          conn->out_packet.remaining_length);
  if(conn->out_packet.remaining_length_enc_bytes > 4) {
    call_event(conn, MQTT_EVENT_PROTOCOL_ERROR, NULL);
    PRINTF("MQTT - Error, remaining length > 4 bytes\n");
    PT_EXIT(pt);
  }

  /* Write Fixed Header */
  PT_MQTT_WRITE_BYTE(conn, conn->out_packet.fhdr);
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->out_packet.remaining_length_enc,
                      conn->out_packet.remaining_length_enc_bytes);
  /* Write Variable Header */
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length >> 8));
  PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.topic_length & 0x00FF));
  PT_MQTT_WRITE_BYTES(conn, (uint8_t *)conn->out_packet.topic,
                      conn->out_packet.topic_length);
  if(conn->out_packet.qos > MQTT_QOS_LEVEL_0) {
    PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid << 8));
    PT_MQTT_WRITE_BYTE(conn, (conn->out_packet.mid & 0x00FF));
  }
  /* Write Payload */
  PT_MQTT_WRITE_BYTES(conn,
                      conn->out_packet.payload,
                      conn->out_packet.payload_size);

  send_out_buffer(conn);
  timer_set(&conn->t, RESPONSE_WAIT_TIMEOUT);

  /*
   * If QoS is zero then wait until the message has been sent, since there is
   * no ACK to wait for.
   *
   * Also notify the app will not be notified via PUBACK or PUBCOMP
   */
  if(conn->out_packet.qos == 0) {
    process_post(conn->app_process, mqtt_update_event, NULL);
  } else if(conn->out_packet.qos == 1) {
    /* Wait for PUBACK */
    reset_packet(&conn->in_packet);
    PT_WAIT_UNTIL(pt, conn->out_packet.qos_state == MQTT_QOS_STATE_GOT_ACK ||
                  timer_expired(&conn->t));
    if(timer_expired(&conn->t)) {
      DBG("Timeout waiting for PUBACK\n");
    }
    if(conn->in_packet.mid != conn->out_packet.mid) {
      DBG("MQTT - Warning, got PUBACK with none matching MID. Currently there "
          "is no support for several concurrent PUBLISH messages.\n");
    }
  } else if(conn->out_packet.qos == 2) {
    DBG("MQTT - QoS not implemented yet.\n");
    /* Should wait for PUBREC, send PUBREL and then wait for PUBCOMP */
  }

  reset_packet(&conn->in_packet);

  /* This is clear after the entire transaction is complete */
  conn->out_queue_full = 0;

  DBG("MQTT - Publish Enqueued\n");

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(pingreq_pt(struct pt *pt, struct mqtt_connection *conn))
{
  PT_BEGIN(pt);

  DBG("MQTT - Sending PINGREQ\n");

  /* Write Fixed Header */
  PT_MQTT_WRITE_BYTE(conn, MQTT_FHDR_MSG_TYPE_PINGREQ);
  PT_MQTT_WRITE_BYTE(conn, 0);

  send_out_buffer(conn);

  /* Start timeout for reply. */
  conn->waiting_for_pingresp = 1;

  /* Wait for PINGRESP or timeout */
  reset_packet(&conn->in_packet);
  timer_set(&conn->t, RESPONSE_WAIT_TIMEOUT);

  PT_WAIT_UNTIL(pt, conn->in_packet.packet_received || timer_expired(&conn->t));

  reset_packet(&conn->in_packet);

  conn->waiting_for_pingresp = 0;

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static void
handle_connack(struct mqtt_connection *conn)
{
  DBG("MQTT - Got CONNACK\n");

  if(conn->in_packet.payload[1] != 0) {
    PRINTF("MQTT - Connection refused with Return Code %i\n",
           conn->in_packet.payload[1]);
    call_event(conn,
               MQTT_EVENT_CONNECTION_REFUSED_ERROR,
               &conn->in_packet.payload[1]);
    abort_connection(conn);
    return;
  }

  conn->out_packet.qos_state = MQTT_QOS_STATE_GOT_ACK;

  ctimer_set(&conn->keep_alive_timer, conn->keep_alive * CLOCK_SECOND,
             keep_alive_callback, conn);

  /* Always reset packet before callback since it might be used directly */
  conn->state = MQTT_CONN_STATE_CONNECTED_TO_BROKER;
  call_event(conn, MQTT_EVENT_CONNECTED, NULL);
}
/*---------------------------------------------------------------------------*/
static void
handle_pingresp(struct mqtt_connection *conn)
{
  DBG("MQTT - Got RINGRESP\n");
}
/*---------------------------------------------------------------------------*/
static void
handle_suback(struct mqtt_connection *conn)
{
  struct mqtt_suback_event suback_event;

  DBG("MQTT - Got SUBACK\n");

  /* Only accept SUBACKS with X topic QoS response, assume 1 */
  if(conn->in_packet.remaining_length > MQTT_MID_SIZE +
     MQTT_MAX_TOPICS_PER_SUBSCRIBE * MQTT_QOS_SIZE) {
    DBG("MQTT - Error, SUBACK with > 1 topic, not supported.\n");
  }

  conn->out_packet.qos_state = MQTT_QOS_STATE_GOT_ACK;

  suback_event.mid = (conn->in_packet.payload[0] << 8) |
    (conn->in_packet.payload[1]);
  suback_event.qos_level = conn->in_packet.payload[2];
  conn->in_packet.mid = suback_event.mid;

  if(conn->in_packet.mid != conn->out_packet.mid) {
    DBG("MQTT - Warning, got SUBACK with none matching MID. Currently there is"
        "no support for several concurrent SUBSCRIBE messages.\n");
  }

  /* Always reset packet before callback since it might be used directly */
  call_event(conn, MQTT_EVENT_SUBACK, &suback_event);
}
/*---------------------------------------------------------------------------*/
static void
handle_unsuback(struct mqtt_connection *conn)
{
  DBG("MQTT - Got UNSUBACK\n");

  conn->out_packet.qos_state = MQTT_QOS_STATE_GOT_ACK;
  conn->in_packet.mid = (conn->in_packet.payload[0] << 8) |
    (conn->in_packet.payload[1]);

  if(conn->in_packet.mid != conn->out_packet.mid) {
    DBG("MQTT - Warning, got UNSUBACK with none matching MID. Currently there is"
        "no support for several concurrent UNSUBSCRIBE messages.\n");
  }

  call_event(conn, MQTT_EVENT_UNSUBACK, &conn->in_packet.mid);
}
/*---------------------------------------------------------------------------*/
static void
handle_puback(struct mqtt_connection *conn)
{
  DBG("MQTT - Got PUBACK\n");

  conn->out_packet.qos_state = MQTT_QOS_STATE_GOT_ACK;
  conn->in_packet.mid = (conn->in_packet.payload[0] << 8) |
    (conn->in_packet.payload[1]);

  call_event(conn, MQTT_EVENT_PUBACK, &conn->in_packet.mid);
}
/*---------------------------------------------------------------------------*/
static void
handle_publish(struct mqtt_connection *conn)
{
  DBG("MQTT - Got PUBLISH, called once per manageable chunk of message.\n");
  DBG("MQTT - Handling publish on topic '%s'\n", conn->in_publish_msg.topic);

  DBG("MQTT - This chunk is %i bytes\n", conn->in_packet.payload_pos);

  if(((conn->in_packet.fhdr & 0x09) >> 1) > 0) {
    PRINTF("MQTT - Error, got incoming PUBLISH with QoS > 0, not supported atm!\n");
  }

  call_event(conn, MQTT_EVENT_PUBLISH, &conn->in_publish_msg);

  if(conn->in_publish_msg.first_chunk == 1) {
    conn->in_publish_msg.first_chunk = 0;
  }

  /* If this is the last time handle_publish will be called, reset packet. */
  if(conn->in_publish_msg.payload_left == 0) {

    /* Check for QoS and initiate the reply, do not rely on the data in the
     * in_packet being untouched. */

    DBG("MQTT - (handle_publish) resetting packet.\n");
    reset_packet(&conn->in_packet);
  }
}
/*---------------------------------------------------------------------------*/
static void
parse_publish_vhdr(struct mqtt_connection *conn,
                   uint32_t *pos,
                   const uint8_t *input_data_ptr,
                   int input_data_len)
{
  uint16_t copy_bytes;

  /* Read out topic length */
  if(conn->in_packet.topic_len_received == 0) {
    conn->in_packet.topic_len = (input_data_ptr[(*pos)++] << 8);
    conn->in_packet.byte_counter++;
    if(*pos >= input_data_len) {
      return;
    }
    conn->in_packet.topic_len |= input_data_ptr[(*pos)++];
    conn->in_packet.byte_counter++;
    conn->in_packet.topic_len_received = 1;

    DBG("MQTT - Read PUBLISH topic len %i\n", conn->in_packet.topic_len);
    /* WARNING: Check here if TOPIC fits in payload area, otherwise error */
  }

  /* Read out topic */
  if(conn->in_packet.topic_len_received == 1 &&
     conn->in_packet.topic_received == 0) {
    copy_bytes = MIN(conn->in_packet.topic_len - conn->in_packet.topic_pos,
                     input_data_len - *pos);
    DBG("MQTT - topic_pos: %i copy_bytes: %i", conn->in_packet.topic_pos,
        copy_bytes);
    memcpy(&conn->in_publish_msg.topic[conn->in_packet.topic_pos],
           &input_data_ptr[*pos],
           copy_bytes);
    (*pos) += copy_bytes;
    conn->in_packet.byte_counter += copy_bytes;
    conn->in_packet.topic_pos += copy_bytes;

    if(conn->in_packet.topic_len - conn->in_packet.topic_pos == 0) {
      DBG("MQTT - Got topic '%s'", conn->in_publish_msg.topic);
      conn->in_packet.topic_received = 1;
      conn->in_publish_msg.topic[conn->in_packet.topic_pos] = '\0';
      conn->in_publish_msg.payload_length =
        conn->in_packet.remaining_length - conn->in_packet.topic_len - 2;
      conn->in_publish_msg.payload_left = conn->in_publish_msg.payload_length;
    }

    /* Set this once per incomming publish message */
    conn->in_publish_msg.first_chunk = 1;
  }
}
/*---------------------------------------------------------------------------*/
static int
tcp_input(struct tcp_socket *s,
          void *ptr,
          const uint8_t *input_data_ptr,
          int input_data_len)
{
  struct mqtt_connection *conn = ptr;
  uint32_t pos = 0;
  uint32_t copy_bytes = 0;
  uint8_t byte;

  if(input_data_len == 0) {
    return 0;
  }

  if(conn->in_packet.packet_received) {
    reset_packet(&conn->in_packet);
  }

  DBG("tcp_input with %i bytes of data:\n", input_data_len);

  /* Read the fixed header field, if we do not have it */
  if(!conn->in_packet.fhdr) {
    conn->in_packet.fhdr = input_data_ptr[pos++];
    conn->in_packet.byte_counter++;

    DBG("MQTT - Read VHDR '%02X'\n", conn->in_packet.fhdr);

    if(pos >= input_data_len) {
      return 0;
    }
  }

  /* Read the Remaining Length field, if we do not have it */
  if(!conn->in_packet.has_remaining_length) {
    do {
      if(pos >= input_data_len) {
        return 0;
      }

      byte = input_data_ptr[pos++];
      conn->in_packet.byte_counter++;
      conn->in_packet.remaining_length_bytes++;
      DBG("MQTT - Read Remaining Length byte\n");

      if(conn->in_packet.byte_counter > 5) {
        call_event(conn, MQTT_EVENT_ERROR, NULL);
        DBG("Received more then 4 byte 'remaining lenght'.");
        return 0;
      }

      conn->in_packet.remaining_length +=
        (byte & 127) * conn->in_packet.remaining_multiplier;
      conn->in_packet.remaining_multiplier *= 128;
    } while((byte & 128) != 0);

    DBG("MQTT - Finished reading remaining length byte\n");
    conn->in_packet.has_remaining_length = 1;
  }

  /*
   * Check for unsupported payload length. Will read all incoming data from the
   * server in any case and then reset the packet.
   *
   * TODO: Decide if we, for example, want to disconnect instead.
   */
  if((conn->in_packet.remaining_length > MQTT_INPUT_BUFF_SIZE) &&
     (conn->in_packet.fhdr & 0xF0) != MQTT_FHDR_MSG_TYPE_PUBLISH) {

    PRINTF("MQTT - Error, unsupported payload size for non-PUBLISH message\n");

    conn->in_packet.byte_counter += input_data_len;
    if(conn->in_packet.byte_counter >=
       (MQTT_FHDR_SIZE + conn->in_packet.remaining_length)) {
      conn->in_packet.packet_received = 1;
    }
    return 0;
  }

  /*
   * Supported payload, reads out both VHDR and Payload of all packets.
   *
   * Note: There will always be at least one byte left to read when we enter
   *       this loop.
   */
  while(conn->in_packet.byte_counter <
        (MQTT_FHDR_SIZE + conn->in_packet.remaining_length)) {

    if((conn->in_packet.fhdr & 0xF0) == MQTT_FHDR_MSG_TYPE_PUBLISH &&
       conn->in_packet.topic_received == 0) {
      parse_publish_vhdr(conn, &pos, input_data_ptr, input_data_len);
    }

    /* Read in as much as we can into the packet payload */
    copy_bytes = MIN(input_data_len - pos,
                     MQTT_INPUT_BUFF_SIZE - conn->in_packet.payload_pos);
    DBG("- Copied %lu payload bytes\n", copy_bytes);
    memcpy(&conn->in_packet.payload[conn->in_packet.payload_pos],
           &input_data_ptr[pos],
           copy_bytes);
    conn->in_packet.byte_counter += copy_bytes;
    conn->in_packet.payload_pos += copy_bytes;
    pos += copy_bytes;

    uint8_t i;
    DBG("MQTT - Copied bytes: \n");
    for(i = 0; i < copy_bytes; i++) {
      DBG("%02X ", conn->in_packet.payload[i]);
    }
    DBG("\n");

    /* Full buffer, shall only happen to PUBLISH messages. */
    if(MQTT_INPUT_BUFF_SIZE - conn->in_packet.payload_pos == 0) {
      conn->in_publish_msg.payload_chunk = conn->in_packet.payload;
      conn->in_publish_msg.payload_chunk_length = MQTT_INPUT_BUFF_SIZE;
      conn->in_publish_msg.payload_left -= MQTT_INPUT_BUFF_SIZE;

      handle_publish(conn);

      conn->in_publish_msg.payload_chunk = conn->in_packet.payload;
      conn->in_packet.payload_pos = 0;
    }

    if(pos >= input_data_len &&
       (conn->in_packet.byte_counter < (MQTT_FHDR_SIZE + conn->in_packet.remaining_length))) {
      return 0;
    }
  }

  /* Debug information */
  DBG("\n");
  /* Take care of input */
  DBG("MQTT - Finished reading packet!\n");
  /* What to return? */
  DBG("MQTT - total data was %i bytes of data. \n",
      (MQTT_FHDR_SIZE + conn->in_packet.remaining_length));

  /* Handle packet here. */
  switch(conn->in_packet.fhdr & 0xF0) {
  case MQTT_FHDR_MSG_TYPE_CONNACK:
    handle_connack(conn);
    break;
  case MQTT_FHDR_MSG_TYPE_PUBLISH:
    /* This is the only or the last chunk of publish payload */
    conn->in_publish_msg.payload_chunk = conn->in_packet.payload;
    conn->in_publish_msg.payload_chunk_length = conn->in_packet.payload_pos;
    conn->in_publish_msg.payload_left = 0;
    handle_publish(conn);
    break;
  case MQTT_FHDR_MSG_TYPE_PUBACK:
    handle_puback(conn);
    break;
  case MQTT_FHDR_MSG_TYPE_SUBACK:
    handle_suback(conn);
    break;
  case MQTT_FHDR_MSG_TYPE_UNSUBACK:
    handle_unsuback(conn);
    break;
  case MQTT_FHDR_MSG_TYPE_PINGRESP:
    handle_pingresp(conn);
    break;

  /* QoS 2 not implemented yet */
  case MQTT_FHDR_MSG_TYPE_PUBREC:
  case MQTT_FHDR_MSG_TYPE_PUBREL:
  case MQTT_FHDR_MSG_TYPE_PUBCOMP:
    call_event(conn, MQTT_EVENT_NOT_IMPLEMENTED_ERROR, NULL);
    PRINTF("MQTT - Got unhandled MQTT Message Type '%i'",
           (conn->in_packet.fhdr & 0xF0));
    break;

  default:
    /* All server-only message */
    PRINTF("MQTT - Got MQTT Message Type '%i'", (conn->in_packet.fhdr & 0xF0));
    break;
  }

  conn->in_packet.packet_received = 1;

  return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * Handles TCP events from Simple TCP
 */
static void
tcp_event(struct tcp_socket *s, void *ptr, tcp_socket_event_t event)
{
  struct mqtt_connection *conn = ptr;

  /* Take care of event */
  switch(event) {

  /* Fall through to manage different disconnect event the same way. */
  case TCP_SOCKET_CLOSED:
  case TCP_SOCKET_TIMEDOUT:
  case TCP_SOCKET_ABORTED: {

    DBG("MQTT - Disconnected by tcp event %d\n", event);
    process_post(&mqtt_process, mqtt_abort_now_event, conn);
    conn->state = MQTT_CONN_STATE_NOT_CONNECTED;
    ctimer_stop(&conn->keep_alive_timer);
    call_event(conn, MQTT_EVENT_DISCONNECTED, &event);
    abort_connection(conn);

    /* If connecting retry */
    if(conn->auto_reconnect == 1) {
      connect_tcp(conn);
    }
    break;
  }
  case TCP_SOCKET_CONNECTED: {
    conn->state = MQTT_CONN_STATE_TCP_CONNECTED;
    conn->out_buffer_sent = 1;

    process_post(&mqtt_process, mqtt_do_connect_mqtt_event, conn);
    break;
  }
  case TCP_SOCKET_DATA_SENT: {
    DBG("MQTT - Got TCP_DATA_SENT\n");

    if(conn->socket.output_data_len == 0) {
      conn->out_buffer_sent = 1;
      conn->out_buffer_ptr = conn->out_buffer;
    }

    ctimer_restart(&conn->keep_alive_timer);
    break;
  }

  default: {
    DBG("MQTT - TCP Event %d is currently not managed by the tcp event callback\n",
        event);
  }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_process, ev, data)
{
  static struct mqtt_connection *conn;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == mqtt_abort_now_event) {
      DBG("MQTT - Abort\n");
      conn = data;
      conn->state = MQTT_CONN_STATE_ABORT_IMMEDIATE;

      abort_connection(conn);
    }
    if(ev == mqtt_do_connect_tcp_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_connect_tcp_event!\n");
      connect_tcp(conn);
    }
    if(ev == mqtt_do_connect_mqtt_event) {
      conn = data;
      conn->socket.output_data_max_seg = conn->max_segment_size;
      DBG("MQTT - Got mqtt_do_connect_mqtt_event!\n");

      if(conn->out_buffer_sent == 1) {
        PT_INIT(&conn->out_proto_thread);
        while(connect_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
              conn->state != MQTT_CONN_STATE_ABORT_IMMEDIATE) {
          PT_MQTT_WAIT_SEND();
        }
      }
    }
    if(ev == mqtt_do_disconnect_mqtt_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_disconnect_mqtt_event!\n");

      /* Send MQTT Disconnect if we are connected */
      if(conn->state == MQTT_CONN_STATE_SENDING_MQTT_DISCONNECT) {
        if(conn->out_buffer_sent == 1) {
          PT_INIT(&conn->out_proto_thread);
          while(disconnect_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
                conn->state != MQTT_CONN_STATE_ABORT_IMMEDIATE) {
            PT_MQTT_WAIT_SEND();
          }
          abort_connection(conn);
          call_event(conn, MQTT_EVENT_DISCONNECTED, &ev);
        } else {
          process_post(&mqtt_process, mqtt_do_disconnect_mqtt_event, conn);
        }
      }
    }
    if(ev == mqtt_do_pingreq_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_pingreq_event!\n");

      if(conn->out_buffer_sent == 1 &&
         conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
        PT_INIT(&conn->out_proto_thread);
        while(pingreq_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
              conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
          PT_MQTT_WAIT_SEND();
        }
      }
    }
    if(ev == mqtt_do_subscribe_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_subscribe_mqtt_event!\n");

      if(conn->out_buffer_sent == 1 &&
         conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
        PT_INIT(&conn->out_proto_thread);
        while(subscribe_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
              conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
          PT_MQTT_WAIT_SEND();
        }
      }
    }
    if(ev == mqtt_do_unsubscribe_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_unsubscribe_mqtt_event!\n");

      if(conn->out_buffer_sent == 1 &&
         conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
        PT_INIT(&conn->out_proto_thread);
        while(unsubscribe_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
              conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
          PT_MQTT_WAIT_SEND();
        }
      }
    }
    if(ev == mqtt_do_publish_event) {
      conn = data;
      DBG("MQTT - Got mqtt_do_publish_mqtt_event!\n");

      if(conn->out_buffer_sent == 1 &&
         conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
        PT_INIT(&conn->out_proto_thread);
        while(publish_pt(&conn->out_proto_thread, conn) < PT_EXITED &&
              conn->state == MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
          PT_MQTT_WAIT_SEND();
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
mqtt_init(void)
{
  static uint8_t inited = 0;
  if(!inited) {
    mqtt_do_connect_tcp_event = process_alloc_event();
    mqtt_event_min = mqtt_do_connect_tcp_event;

    mqtt_do_connect_mqtt_event = process_alloc_event();
    mqtt_do_disconnect_mqtt_event = process_alloc_event();
    mqtt_do_subscribe_event = process_alloc_event();
    mqtt_do_unsubscribe_event = process_alloc_event();
    mqtt_do_publish_event = process_alloc_event();
    mqtt_do_pingreq_event = process_alloc_event();
    mqtt_update_event = process_alloc_event();
    mqtt_abort_now_event = process_alloc_event();
    mqtt_event_max = mqtt_abort_now_event;

    mqtt_continue_send_event = process_alloc_event();

    list_init(mqtt_conn_list);
    process_start(&mqtt_process, NULL);
    inited = 1;
  }
}
/*---------------------------------------------------------------------------*/
mqtt_status_t
mqtt_register(struct mqtt_connection *conn, struct process *app_process,
              char *client_id, mqtt_event_callback_t event_callback,
              uint16_t max_segment_size)
{
  if(strlen(client_id) < 1) {
    return MQTT_STATUS_INVALID_ARGS_ERROR;
  }

  /* Set defaults - Set all to zero to begin with */
  memset(conn, 0, sizeof(struct mqtt_connection));
  string_to_mqtt_string(&conn->client_id, client_id);
  conn->event_callback = event_callback;
  conn->app_process = app_process;
  conn->auto_reconnect = 1;
  conn->max_segment_size = max_segment_size;
  reset_defaults(conn);

  mqtt_init();
  list_add(mqtt_conn_list, conn);

  DBG("MQTT - Registered successfully\n");

  return MQTT_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
/*
 * Connect to MQTT broker.
 *
 * N.B. Non-blocking call.
 */
mqtt_status_t
mqtt_connect(struct mqtt_connection *conn, char *host, uint16_t port,
             uint16_t keep_alive)
{
  uip_ip6addr_t ip6addr;
  uip_ipaddr_t *ipaddr;
  ipaddr = &ip6addr;

  /* Check if we are already trying to connect */
  if(conn->state > MQTT_CONN_STATE_NOT_CONNECTED) {
    return MQTT_STATUS_OK;
  }

  conn->server_host = host;
  conn->keep_alive = keep_alive;
  conn->server_port = port;
  conn->out_buffer_ptr = conn->out_buffer;
  conn->out_packet.qos_state = MQTT_QOS_STATE_NO_ACK;
  conn->connect_vhdr_flags |= MQTT_VHDR_CLEAN_SESSION_FLAG;

  /* convert the string IPv6 address to a numeric IPv6 address */
  uiplib_ip6addrconv(host, &ip6addr);

  uip_ipaddr_copy(&(conn->server_ip), ipaddr);

  /*
   * Initiate the connection if the IP could be resolved. Otherwise the
   * connection will be initiated when the DNS lookup is finished, in the main
   * event loop.
   */
  process_post(&mqtt_process, mqtt_do_connect_tcp_event, conn);

  return MQTT_STATUS_OK;
}
/*----------------------------------------------------------------------------*/
void
mqtt_disconnect(struct mqtt_connection *conn)
{
  if(conn->state != MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
    return;
  }

  conn->state = MQTT_CONN_STATE_SENDING_MQTT_DISCONNECT;

  process_post(&mqtt_process, mqtt_do_disconnect_mqtt_event, conn);
}
/*----------------------------------------------------------------------------*/
mqtt_status_t
mqtt_subscribe(struct mqtt_connection *conn, uint16_t *mid, char *topic,
               mqtt_qos_level_t qos_level)
{
  if(conn->state != MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
    return MQTT_STATUS_NOT_CONNECTED_ERROR;
  }

  DBG("MQTT - Call to mqtt_subscribe...\n");

  /* Currently don't have a queue, so only one item at a time */
  if(conn->out_queue_full) {
    DBG("MQTT - Not accepted!\n");
    return MQTT_STATUS_OUT_QUEUE_FULL;
  }
  conn->out_queue_full = 1;
  DBG("MQTT - Accepted!\n");

  conn->out_packet.mid = INCREMENT_MID(conn);
  conn->out_packet.topic = topic;
  conn->out_packet.topic_length = strlen(topic);
  conn->out_packet.qos = qos_level;
  conn->out_packet.qos_state = MQTT_QOS_STATE_NO_ACK;

  process_post(&mqtt_process, mqtt_do_subscribe_event, conn);
  return MQTT_STATUS_OK;
}
/*----------------------------------------------------------------------------*/
mqtt_status_t
mqtt_unsubscribe(struct mqtt_connection *conn, uint16_t *mid, char *topic)
{
  if(conn->state != MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
    return MQTT_STATUS_NOT_CONNECTED_ERROR;
  }

  DBG("MQTT - Call to mqtt_unsubscribe...\n");
  /* Currently don't have a queue, so only one item at a time */
  if(conn->out_queue_full) {
    DBG("MQTT - Not accepted!\n");
    return MQTT_STATUS_OUT_QUEUE_FULL;
  }
  conn->out_queue_full = 1;
  DBG("MQTT - Accepted!\n");

  conn->out_packet.mid = INCREMENT_MID(conn);
  conn->out_packet.topic = topic;
  conn->out_packet.topic_length = strlen(topic);
  conn->out_packet.qos_state = MQTT_QOS_STATE_NO_ACK;

  process_post(&mqtt_process, mqtt_do_unsubscribe_event, conn);
  return MQTT_STATUS_OK;
}
/*----------------------------------------------------------------------------*/
mqtt_status_t
mqtt_publish(struct mqtt_connection *conn, uint16_t *mid, char *topic,
             uint8_t *payload, uint32_t payload_size,
             mqtt_qos_level_t qos_level, mqtt_retain_t retain)
{
  if(conn->state != MQTT_CONN_STATE_CONNECTED_TO_BROKER) {
    return MQTT_STATUS_NOT_CONNECTED_ERROR;
  }

  DBG("MQTT - Call to mqtt_publish...\n");

  /* Currently don't have a queue, so only one item at a time */
  if(conn->out_queue_full) {
    DBG("MQTT - Not accepted!\n");
    return MQTT_STATUS_OUT_QUEUE_FULL;
  }
  conn->out_queue_full = 1;
  DBG("MQTT - Accepted!\n");

  conn->out_packet.mid = INCREMENT_MID(conn);
  conn->out_packet.retain = retain;
  conn->out_packet.topic = topic;
  conn->out_packet.topic_length = strlen(topic);
  conn->out_packet.payload = payload;
  conn->out_packet.payload_size = payload_size;
  conn->out_packet.qos = qos_level;
  conn->out_packet.qos_state = MQTT_QOS_STATE_NO_ACK;

  process_post(&mqtt_process, mqtt_do_publish_event, conn);
  return MQTT_STATUS_OK;
}
/*----------------------------------------------------------------------------*/
void
mqtt_set_username_password(struct mqtt_connection *conn, char *username,
                           char *password)
{
  /* Set strings, NULL string will simply set length to zero */
  string_to_mqtt_string(&conn->credentials.username, username);
  string_to_mqtt_string(&conn->credentials.password, password);

  /* Set CONNECT VHDR flags */
  if(username != NULL) {
    conn->connect_vhdr_flags |= MQTT_VHDR_USERNAME_FLAG;
  } else {
    conn->connect_vhdr_flags &= ~MQTT_VHDR_USERNAME_FLAG;
  }
  if(password != NULL) {
    conn->connect_vhdr_flags |= MQTT_VHDR_PASSWORD_FLAG;
  } else {
    conn->connect_vhdr_flags &= ~MQTT_VHDR_PASSWORD_FLAG;
  }
}
/*----------------------------------------------------------------------------*/
void
mqtt_set_last_will(struct mqtt_connection *conn, char *topic, char *message,
                   mqtt_qos_level_t qos)
{
  /* Set strings, NULL string will simply set length to zero */
  string_to_mqtt_string(&conn->will.topic, topic);
  string_to_mqtt_string(&conn->will.message, message);

  /* Currently not used! */
  conn->will.qos = qos;

  if(topic != NULL) {
    conn->connect_vhdr_flags |= MQTT_VHDR_WILL_FLAG |
      MQTT_VHDR_WILL_RETAIN_FLAG;
  }
}
/*----------------------------------------------------------------------------*/
/** @} */
