/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
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
#include "contiki.h"
#include "sensniff.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "sys/process.h"
#include "sys/ctimer.h"
#include "lib/ringbuf.h"

#include SENSNIFF_IO_DRIVER_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
PROCESS(sensniff_process, "sensniff process");
AUTOSTART_PROCESSES(&sensniff_process);
/*---------------------------------------------------------------------------*/
/* Timeout handling for incoming characters. */
#define TIMEOUT (CLOCK_SECOND >> 1)
static struct ctimer ct;
/*---------------------------------------------------------------------------*/
#define STATE_WAITING_FOR_MAGIC     0x00
#define STATE_WAITING_FOR_VERSION   0x01
#define STATE_WAITING_FOR_CMD       0x02
#define STATE_WAITING_FOR_LEN_1     0x03
#define STATE_WAITING_FOR_LEN_2     0x04
#define STATE_WAITING_FOR_DATA      0x05

static uint8_t state;
static uint8_t in_ct;
/*---------------------------------------------------------------------------*/
#define CMD_FRAME               0x00
#define CMD_CHANNEL             0x01
#define CMD_CHANNEL_MIN         0x02
#define CMD_CHANNEL_MAX         0x03
#define CMD_ERR_NOT_SUPPORTED   0x7F
#define CMD_GET_CHANNEL         0x81
#define CMD_GET_CHANNEL_MIN     0x82
#define CMD_GET_CHANNEL_MAX     0x83
#define CMD_SET_CHANNEL         0x84
/*---------------------------------------------------------------------------*/
#define PROTOCOL_VERSION           2
/*---------------------------------------------------------------------------*/
#define BUFSIZE 32

static struct ringbuf rxbuf;

typedef struct cmd_in_s {
  uint8_t cmd;
  uint16_t len;
  uint8_t data;
} cmd_in_t;

static cmd_in_t command;

uint8_t cmd_buf[BUFSIZE];
/*---------------------------------------------------------------------------*/
static const uint8_t magic[] = { 0xC1, 0x1F, 0xFE, 0x72 };
/*---------------------------------------------------------------------------*/
static void
reset_state(void *byte)
{
  state = STATE_WAITING_FOR_MAGIC;
  in_ct = 0;
  memset(&command, 0, sizeof(command));
}
/*---------------------------------------------------------------------------*/
static void
send_header(uint8_t cmd, uint16_t len)
{
  uint16_t i;

  /* Send the magic */
  for(i = 0; i < 4; i++) {
    sensniff_io_byte_out(magic[i]);
  }

  /* Send the protocol version */
  sensniff_io_byte_out(PROTOCOL_VERSION);

  /* Send the command byte */
  sensniff_io_byte_out(cmd);

  /* Send the length, network endianness */
  sensniff_io_byte_out(len >> 8);
  sensniff_io_byte_out(len & 0xFF);
}
/*---------------------------------------------------------------------------*/
static void
send_error(void)
{
  send_header(CMD_ERR_NOT_SUPPORTED, 0);

  sensniff_io_flush();
}
/*---------------------------------------------------------------------------*/
static void
send_channel(void)
{
  radio_value_t chan;

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &chan) ==
     RADIO_RESULT_OK) {
    send_header(CMD_CHANNEL, 1);
    sensniff_io_byte_out(chan & 0xFF);
    sensniff_io_flush();
    return;
  }

  send_error();
}
/*---------------------------------------------------------------------------*/
static void
set_channel(uint8_t channel)
{
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel) ==
     RADIO_RESULT_OK) {
    send_channel();
    return;
  }

  send_error();
}
/*---------------------------------------------------------------------------*/
static void
send_channel_min(void)
{
  radio_value_t chan;

  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &chan) ==
     RADIO_RESULT_OK) {
    send_header(CMD_CHANNEL_MIN, 1);
    sensniff_io_byte_out(chan & 0xFF);
    sensniff_io_flush();
    return;
  }

  send_error();
}
/*---------------------------------------------------------------------------*/
static void
send_channel_max(void)
{
  radio_value_t chan;

  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &chan) ==
     RADIO_RESULT_OK) {
    send_header(CMD_CHANNEL_MAX, 1);
    sensniff_io_byte_out(chan & 0xFF);
    sensniff_io_flush();
    return;
  }

  send_error();
}
/*---------------------------------------------------------------------------*/
static int
char_in(unsigned char c)
{
  /* Bump the timeout counter */
  ctimer_set(&ct, TIMEOUT, reset_state, NULL);

  /* Add the character to our ringbuf and poll the consumer process. */
  ringbuf_put(&rxbuf, c);

  process_poll(&sensniff_process);
  return 1;
}
/*---------------------------------------------------------------------------*/
void
sensniff_output_frame()
{
  int i;
  uint8_t len = packetbuf_datalen() & 0xFF;

  send_header(CMD_FRAME, len + 2);

  for(i = 0; i < len; i++) {
    sensniff_io_byte_out(((uint8_t *)packetbuf_dataptr())[i]);
  }

  sensniff_io_byte_out(packetbuf_attr(PACKETBUF_ATTR_RSSI) & 0xFF);
  sensniff_io_byte_out(0x80 |
                       (packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY) & 0xFF));

  sensniff_io_flush();
}
/*---------------------------------------------------------------------------*/
static void
execute_command(void)
{
  switch(command.cmd) {
  case CMD_GET_CHANNEL:
    send_channel();
    break;
  case CMD_GET_CHANNEL_MIN:
    send_channel_min();
    break;
  case CMD_GET_CHANNEL_MAX:
    send_channel_max();
    break;
  case CMD_SET_CHANNEL:
    set_channel(command.data);
    break;
  default:
    send_error();
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
process_incoming_data(void)
{
  int c = 0;
  uint8_t byte_in;

  c = ringbuf_get(&rxbuf);

  while(c != -1) {
    byte_in = (uint8_t)c;
    switch(state) {
    case STATE_WAITING_FOR_MAGIC:
      if(byte_in == magic[in_ct]) {
        in_ct++;
        if(in_ct == sizeof(magic)) {
          state = STATE_WAITING_FOR_VERSION;
          in_ct = 0;
        }
      } else {
        reset_state(&byte_in);
      }
      break;
    case STATE_WAITING_FOR_VERSION:
      if(byte_in == PROTOCOL_VERSION) {
        state = STATE_WAITING_FOR_CMD;
      } else {
        reset_state(&byte_in);
      }
      break;
    case STATE_WAITING_FOR_CMD:
      command.cmd = byte_in;

      if(command.cmd == CMD_GET_CHANNEL ||
         command.cmd == CMD_GET_CHANNEL_MIN ||
         command.cmd == CMD_GET_CHANNEL_MAX) {
        execute_command();
        reset_state(&byte_in);
      } else {
        state = STATE_WAITING_FOR_LEN_1;
      }
      break;
    case STATE_WAITING_FOR_LEN_1:
      command.len = byte_in << 8;
      state = STATE_WAITING_FOR_LEN_2;
      break;
    case STATE_WAITING_FOR_LEN_2:
      command.len |= byte_in;
      if(command.len == 1) {
        state = STATE_WAITING_FOR_DATA;
      } else {
        reset_state(&byte_in);
      }
      break;
    case STATE_WAITING_FOR_DATA:
      command.data = byte_in;
      execute_command();
      reset_state(&byte_in);
      break;
    default:
      break;
    }
    c = ringbuf_get(&rxbuf);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensniff_process, ev, data)
{
  PROCESS_BEGIN();

  /* Turn off RF frame filtering and H/W ACKs */
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, 0) != RADIO_RESULT_OK) {
    PRINTF("sensniff: Error setting RF in promiscuous mode\n");
    PROCESS_EXIT();
  }

  /* Initialise the ring buffer */
  ringbuf_init(&rxbuf, cmd_buf, sizeof(cmd_buf));

  /* Initialise the state machine */
  reset_state(NULL);

  /* Register for char inputs with the character I/O peripheral */
  sensniff_io_set_input(&char_in);

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_POLL) {
      process_incoming_data();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
