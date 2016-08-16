/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include <string.h>
#include <stdint.h>

#include <aducrf101-contiki.h>

#include "contiki.h"
#include "contiki-net.h"
#include "net/netstack.h"
#include "radio.h"

#define MAX_PACKET_LEN 240

static uint8_t tx_buf[MAX_PACKET_LEN];

#ifndef ADUCRF101_RADIO_BASE_CONFIG
#define ADUCRF101_RADIO_BASE_CONFIG DR_38_4kbps_Dev20kHz
#endif

static RIE_BaseConfigs base_config = ADUCRF101_RADIO_BASE_CONFIG;
static int current_channel = 915000000;
static int current_power = 31;
static int radio_is_on = 0;
/*---------------------------------------------------------------------------*/
/* "Channel" is really frequency, and can be within the bands:
   431000000 Hz to 464000000 Hz
   862000000 Hz to 928000000 Hz
 */
#define MIN_CHANNEL 431000000
#define MAX_CHANNEL 928000000
static int
_set_channel(int freq)
{
  if(freq < 431000000) {
    freq = 431000000;
  } else if(freq > 464000000 && freq < 663000000) {
    freq = 464000000;
  } else if(freq >= 663000000 && freq < 862000000) {
    freq = 862000000;
  } else if(freq > 928000000) {
    freq = 928000000;
  }
  current_channel = freq;
  if(RadioSetFrequency(freq) != RIE_Success) {
    return RADIO_RESULT_ERROR;
  }
  return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
/* "Power" covers both PA type and power level:
    0 through 15 means single-ended, power level 0 through 15
    16 through 31 means differential, power level 0 through 15 */
#define MIN_POWER 0
#define MAX_POWER 31
static int
_set_power(int power)
{
  RIE_Responses ret;
  if(power < 0) {
    power = 0;
  }
  if(power > 31) {
    power = 31;
  }
  if(power <= 15) {
    ret = RadioTxSetPA(SingleEndedPA, power);
  } else {
    ret = RadioTxSetPA(DifferentialPA, power - 16);
  }
  current_power = power;
  if(ret != RIE_Success) {
    return RADIO_RESULT_ERROR;
  }
  return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
PROCESS(aducrf101_rf_process, "ADuCRF101 RF driver");
/*---------------------------------------------------------------------------*/
/** Turn the radio on. */
static int
on(void)
{
  if(radio_is_on) {
    return 1;
  }

  /* Power radio on */
  if(RadioInit(base_config) != RIE_Success) {
    return 0;
  }

  /* Ensure channel and power are set */
  if(_set_channel(current_channel) != RADIO_RESULT_OK) {
    return 0;
  }
  if(_set_power(current_power) != RADIO_RESULT_OK) {
    return 0;
  }

  /* Enter receive mode */
  RadioRxPacketVariableLen();

  radio_is_on = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
/** Turn the radio off. */
static int
off(void)
{
  if(!radio_is_on) {
    return 1;
  }
  if(RadioPowerOff() != RIE_Success) {
    return 0;
  }
  radio_is_on = 0;
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  off();
  on();
  process_start(&aducrf101_rf_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
/** Prepare the radio with a packet to be sent. */
static int
prepare(const void *payload, unsigned short payload_len)
{
  /* Truncate long packets */
  if(payload_len > MAX_PACKET_LEN) {
    payload_len = MAX_PACKET_LEN;
  }
  memcpy(tx_buf, payload, payload_len);
  return 0;
}
/*---------------------------------------------------------------------------*/
/** Send the packet that has previously been prepared. */
static int
transmit(unsigned short transmit_len)
{
  if(!radio_is_on)
    return RADIO_TX_ERR;

  /* Transmit the packet */
  if(transmit_len > MAX_PACKET_LEN) {
    transmit_len = MAX_PACKET_LEN;
  }
  if(RadioTxPacketVariableLen(transmit_len, tx_buf) != RIE_Success) {
    return RADIO_TX_ERR;
  }
  while(!RadioTxPacketComplete())
    continue;

  /* Enter receive mode immediately after transmitting a packet */
  RadioRxPacketVariableLen();

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
/** Prepare & transmit a packet. */
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
/** Read a received packet into a buffer. */
static int
read(void *buf, unsigned short buf_len)
{
  uint8_t packet_len;
  int8_t rssi;

  if(!radio_is_on)
    return 0;

  if(buf_len > MAX_PACKET_LEN) {
    buf_len = MAX_PACKET_LEN;
  }

  /* Read already-received packet */
  if(RadioRxPacketRead(buf_len, &packet_len, buf, &rssi) != RIE_Success) {
    return 0;
  }

  if(packet_len > buf_len) {
    packet_len = buf_len;
  }

  /* Re-enter receive mode immediately after receiving a packet */
  RadioRxPacketVariableLen();

  return packet_len;
}
/*---------------------------------------------------------------------------*/
/** Perform a Clear-Channel Assessment (CCA) to find out if there is
    a packet in the air or not. */
static int
channel_clear(void)
{
  /* Not implemented; assume clear */
  return 1;
}
/*---------------------------------------------------------------------------*/
/** Check if the radio driver is currently receiving a packet */
static int
receiving_packet(void)
{
  /* Not implemented; assume no. */
  return 0;
}
/*---------------------------------------------------------------------------*/
/** Check if the radio driver has just received a packet */
static int
pending_packet(void)
{
  if(RadioRxPacketAvailable()) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/** Get a radio parameter value. */
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_RSSI:
  {
    int8_t dbm;
    if(!radio_is_on || RadioRadioGetRSSI(&dbm) != RIE_Success) {
      return RADIO_RESULT_ERROR;
    }
    *value = dbm;
    return RADIO_RESULT_OK;
  }

  case RADIO_PARAM_CHANNEL:
    *value = current_channel;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = MIN_CHANNEL;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = MAX_CHANNEL;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_TXPOWER:
    *value = current_power;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = MIN_POWER;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = MAX_POWER;
    return RADIO_RESULT_OK;

  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
/** Set a radio parameter value. */
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_CHANNEL:
    return _set_channel(value);

  case RADIO_PARAM_TXPOWER:
    return _set_power(value);

  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Get a radio parameter object. The argument 'dest' must point to a
 * memory area of at least 'size' bytes, and this memory area will
 * contain the parameter object if the function succeeds.
 */
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
/**
 * Set a radio parameter object. The memory area referred to by the
 * argument 'src' will not be accessed after the function returns.
 */
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Implementation of the ADuCRF101 RF driver process
 *
 *        This process is started by init().  It waits for events triggered
 *        by packet reception.
 */
PROCESS_THREAD(aducrf101_rf_process, ev, data)
{
  int len;
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    packetbuf_clear();
    len = read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);

      NETSTACK_RDC.input();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Trigger function called by ADI radio engine upon packet RX.
 */
void
aducrf101_rx_packet_hook(void)
{
  process_poll(&aducrf101_rf_process);
}
/*---------------------------------------------------------------------------*/
const struct radio_driver aducrf101_radio_driver = {
  .init = init,
  .prepare = prepare,
  .transmit = transmit,
  .send = send,
  .read = read,
  .channel_clear = channel_clear,
  .receiving_packet = receiving_packet,
  .pending_packet = pending_packet,
  .on = on,
  .off = off,
  .get_value = get_value,
  .set_value = set_value,
  .get_object = get_object,
  .set_object = set_object,
};
