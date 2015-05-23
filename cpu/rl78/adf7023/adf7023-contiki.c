/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
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
/**
 * \author Ian Martin <martini@redwirellc.com>
 */

#include <string.h>          /* for memcpy(). */

#include "radio.h"

#include "ADF7023.h"
#include "adf7023-contiki.h"
#include "contiki.h"         /* for LED definitions. */

#define ADF7023_MAX_PACKET_SIZE 255

static unsigned char tx_buf[ADF7023_MAX_PACKET_SIZE];
static unsigned char rx_buf[ADF7023_MAX_PACKET_SIZE];

/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver adf7023_driver = {

  .init = adf7023_init,

  /** Prepare the radio with a packet to be sent. */
  .prepare = adf7023_prepare,

  /** Send the packet that has previously been prepared. */
  .transmit = adf7023_transmit,

  /** Prepare & transmit a packet. */
  .send = adf7023_send,

  /** Read a received packet into a buffer. */
  .read = adf7023_read,

  /** Perform a Clear-Channel Assessment (CCA) to find out if there is
     a packet in the air or not. */
  .channel_clear = adf7023_channel_clear,

  /** Check if the radio driver is currently receiving a packet */
  .receiving_packet = adf7023_receiving_packet,

  /** Check if the radio driver has just received a packet */
  .pending_packet = adf7023_pending_packet,

  /** Turn the radio on. */
  .on = adf7023_on,

  /** Turn the radio off. */
  .off = adf7023_off,

  .get_value = get_value,
  .set_value = set_value,
  .get_object = get_object,
  .set_object = set_object
};

int
adf7023_init(void)
{
  ADF7023_Init();
  return 1;
}
int
adf7023_prepare(const void *payload, unsigned short payload_len)
{
  /* Prepare the radio with a packet to be sent. */
  memcpy(tx_buf, payload, (payload_len <= sizeof(tx_buf)) ? payload_len : sizeof(tx_buf));
  return 0;
}
int
adf7023_transmit(unsigned short transmit_len)
{
  /* Send the packet that has previously been prepared. */

  RADIO_TX_LED = 1;
  ADF7023_TransmitPacket(tx_buf, transmit_len);
  RADIO_TX_LED = 0;

  /* TODO: Error conditions (RADIO_TX_ERR, RADIO_TX_COLLISION, RADIO_TX_NOACK)? */
  return RADIO_TX_OK;
}
int
adf7023_send(const void *payload, unsigned short payload_len)
{
  /* Prepare & transmit a packet. */

  RADIO_TX_LED = 1;
  ADF7023_TransmitPacket((void *)payload, payload_len);
  RADIO_TX_LED = 0;

  /* TODO: Error conditions (RADIO_TX_ERR, RADIO_TX_COLLISION, RADIO_TX_NOACK)? */
  return RADIO_TX_OK;
}
int
adf7023_read(void *buf, unsigned short buf_len)
{
  unsigned char num_bytes;
  /* Read a received packet into a buffer. */

  RADIO_RX_LED = 1;
  ADF7023_ReceivePacket(rx_buf, &num_bytes);
  RADIO_RX_LED = 0;

  memcpy(buf, rx_buf, (num_bytes <= buf_len) ? num_bytes : buf_len);
  return num_bytes;
}
int
adf7023_channel_clear(void)
{
  /* Perform a Clear-Channel Assessment (CCA) to find out if there is a packet in the air or not. */
  return 1;
}
int
adf7023_receiving_packet(void)
{
  /* Check if the radio driver is currently receiving a packet. */
  return 0;
}
int
adf7023_pending_packet(void)
{
  /* Check if the radio driver has just received a packet. */
  return ADF7023_ReceivePacketAvailable();
}
int
adf7023_on(void)
{
  /* Turn the radio on. */
  return 1;
}
int
adf7023_off(void)
{
  /* Turn the radio off. */
  return 0;
}
