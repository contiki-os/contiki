/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 *         Implementation of the cc2530 RF driver
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc2530-rf.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "sys/clock.h"
#include "sys/rtimer.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/linkaddr.h"
#include "net/netstack.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define CHECKSUM_LEN 2
/*---------------------------------------------------------------------------*/
#if CC2530_RF_CONF_LEDS
#define CC2530_RF_LEDS CC2530_RF_CONF_LEDS
#else
#define CC2530_RF_LEDS 0
#endif

#if CC2530_RF_LEDS
#include "dev/leds.h"
#define RF_RX_LED_ON()		leds_on(LEDS_RED);
#define RF_RX_LED_OFF()		leds_off(LEDS_RED);
#define RF_TX_LED_ON()		leds_on(LEDS_GREEN);
#define RF_TX_LED_OFF()		leds_off(LEDS_GREEN);
#else
#define RF_RX_LED_ON()
#define RF_RX_LED_OFF()
#define RF_TX_LED_ON()
#define RF_TX_LED_OFF()
#endif
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include "debug.h"
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#endif
/*---------------------------------------------------------------------------*/
/* Local RF Flags */
#define RX_ACTIVE  0x80
#define WAS_OFF    0x10
#define RF_ON      0x01

/* Bit Masks for the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F
/* RSSI Offset */
#define RSSI_OFFSET    73

/* 192 ms, radio off -> on interval */
#define ONOFF_TIME                    RTIMER_ARCH_SECOND / 3125

#define CC2530_RF_CHANNEL_SET_ERROR      -1

#define CC2530_RF_TX_POWER_TXCTRL_MIN_VAL 0x09 /* Value for min TX Power */
#define CC2530_RF_TX_POWER_TXCTRL_DEF_VAL 0x69 /* Reset Value */
/*---------------------------------------------------------------------------*/
#ifdef CC2530_RF_CONF_AUTOACK
#define CC2530_RF_AUTOACK CC2530_RF_CONF_AUTOACK
#else
#define CC2530_RF_AUTOACK 1
#endif
/*---------------------------------------------------------------------------*/
static uint8_t CC_AT_DATA rf_flags;

static int on(void); /* prepare() needs our prototype */
static int off(void); /* transmit() needs our prototype */
static int channel_clear(void); /* transmit() needs our prototype */
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table. Values from SmartRF Studio v1.16.0 */
typedef struct output_config {
  radio_value_t power;
  uint8_t txpower_val;
} output_config_t;

static const output_config_t output_power[] = {
  {  5, 0xF5 }, /* 4.5 */
  {  3, 0xE5 }, /* 2.5 */
  {  1, 0xD5 },
  {  0, 0xC5 }, /* -0.5 */
  { -1, 0xB5 }, /* -1.5 */
  { -3, 0xA5 },
  { -4, 0x95 },
  { -6, 0x85 },
  { -8, 0x75 },
  {-10, 0x65 },
  {-12, 0x55 },
  {-14, 0x45 },
  {-16, 0x35 },
  {-18, 0x25 },
  {-20, 0x15 },
  {-22, 0x05 },
  {-28, 0x05 }, /* TXCTRL must be set to 0x09 */
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN    (output_power[OUTPUT_CONFIG_COUNT - 1].power)
#define OUTPUT_POWER_MAX    (output_power[0].power)
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the current operating channel
 * \return Returns a value in [11,26] representing the current channel
 */
static uint8_t
get_channel()
{
  return (uint8_t)((FREQCTRL + 44) / 5);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the current operating channel
 * \param channel The desired channel as a value in [11,26]
 * \return Returns a value in [11,26] representing the current channel
 *         or a negative value if \e channel was out of bounds
 */
static int8_t
set_channel(uint8_t channel)
{
  PUTSTRING("RF: Set Chan\n");

  if((channel < CC2530_RF_CHANNEL_MIN) || (channel > CC2530_RF_CHANNEL_MAX)) {
    return CC2530_RF_CHANNEL_SET_ERROR;
  }

  /* Changes to FREQCTRL take effect after the next recalibration */
  /* Changes to FREQCTRL take effect after the next recalibration */
  off();
  FREQCTRL = (CC2530_RF_CHANNEL_MIN
      + (channel - CC2530_RF_CHANNEL_MIN) * CC2530_RF_CHANNEL_SPACING);
  on();

  return (int8_t)channel;
}
/*---------------------------------------------------------------------------*/
static radio_value_t
get_pan_id(void)
{
  return (radio_value_t)(PAN_ID1 << 8 | PAN_ID0);
}
/*---------------------------------------------------------------------------*/
static void
set_pan_id(uint16_t pan)
{
  PAN_ID0 = pan & 0xFF;
  PAN_ID1 = pan >> 8;
}
/*---------------------------------------------------------------------------*/
static radio_value_t
get_short_addr(void)
{
  return (radio_value_t)(SHORT_ADDR1 << 8 | SHORT_ADDR0);
}
/*---------------------------------------------------------------------------*/
static void
set_short_addr(uint16_t addr)
{
  SHORT_ADDR0 = addr & 0xFF;
  SHORT_ADDR1 = addr >> 8;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Reads the current signal strength (RSSI)
 * \return The current RSSI in dBm
 *
 * This function reads the current RSSI on the currently configured
 * channel.
 */
static radio_value_t
get_rssi(void)
{
  int8_t rssi;

  /* If we are off, turn on first */
  if(RXENABLE == 0) {
    rf_flags |= WAS_OFF;
    on();
  }

  /* Wait on RSSI_VALID */
  while((RSSISTAT & RSSISTAT_RSSI_VALID) == 0);

  rssi = (radio_value_t)RSSI - RSSI_OFFSET;

  /* If we were off, turn back off */
  if((rf_flags & WAS_OFF) == WAS_OFF) {
    rf_flags &= ~WAS_OFF;
    off();
  }

  return rssi;
}
/*---------------------------------------------------------------------------*/
/* Returns the current CCA threshold in dBm */
static radio_value_t
get_cca_threshold(void)
{
  return (int8_t)CCACTRL0 - RSSI_OFFSET;
}
/*---------------------------------------------------------------------------*/
/* Sets the CCA threshold in dBm */
static void
set_cca_threshold(radio_value_t value)
{
  CCACTRL0 = (value + RSSI_OFFSET) & 0xFF;
}
/*---------------------------------------------------------------------------*/
/* Returns the current TX power in dBm */
static radio_value_t
get_tx_power(void)
{
  int i;
  uint8_t reg_val = TXPOWER;

  if(TXCTRL == CC2530_RF_TX_POWER_TXCTRL_MIN_VAL) {
    return OUTPUT_POWER_MIN;
  }

  /*
   * Find the TXPOWER value in the lookup table
   * If the value has been written with set_tx_power, we should be able to
   * find the exact value. However, in case the register has been written in
   * a different fashion, we return the immediately lower value of the lookup
   */
  for(i = 0; i < OUTPUT_CONFIG_COUNT; i++) {
    if(reg_val >= output_power[i].txpower_val) {
      return output_power[i].power;
    }
  }
  return OUTPUT_POWER_MIN;
}
/*---------------------------------------------------------------------------*/
/*
 * Set TX power to 'at least' power dBm
 * This works with a lookup table. If the value of 'power' does not exist in
 * the lookup table, TXPOWER will be set to the immediately higher available
 * value
 */
static void
set_tx_power(radio_value_t power)
{
  int i;

  if(power <= output_power[OUTPUT_CONFIG_COUNT - 1].power) {
    TXCTRL = CC2530_RF_TX_POWER_TXCTRL_MIN_VAL;
    TXPOWER = output_power[OUTPUT_CONFIG_COUNT - 1].txpower_val;
    return;
  }

  for(i = OUTPUT_CONFIG_COUNT - 2; i >= 0; --i) {
    if(power <= output_power[i].power) {
      /* Perhaps an earlier call set TXCTRL to 0x09. Restore */
      TXCTRL = CC2530_RF_TX_POWER_TXCTRL_DEF_VAL;
      TXPOWER = output_power[i].txpower_val;
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_frame_filtering(uint8_t enable)
{
  if(enable) {
    FRMFILT0 |= FRMFILT0_FRAME_FILTER_EN;
  } else {
    FRMFILT0 &= ~FRMFILT0_FRAME_FILTER_EN;
  }
}
/*---------------------------------------------------------------------------*/
static void
set_auto_ack(uint8_t enable)
{
  if(enable) {
    FRMCTRL0 |= FRMCTRL0_AUTOACK;
  } else {
    FRMCTRL0 &= ~FRMCTRL0_AUTOACK;
  }
}
/*---------------------------------------------------------------------------*/
/* Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  PUTSTRING("RF: Init\n");

  if(rf_flags & RF_ON) {
    return 0;
  }

#if CC2530_RF_LOW_POWER_RX
  /* Reduce RX power consumption current to 20mA at the cost of sensitivity */
  RXCTRL = 0x00;
  FSCTRL = 0x50;
#else
  RXCTRL = 0x3F;
  FSCTRL = 0x55;
#endif /* CC2530_RF_LOW_POWER_RX */

  CCACTRL0 = CC2530_RF_CCA_THRES;

  /*
   * According to the user guide, these registers must be updated from their
   * defaults for optimal performance
   *
   * Table 23-6, Sec. 23.15.1, p. 259
   */
  TXFILTCFG = 0x09; /* TX anti-aliasing filter */
  AGCCTRL1 = 0x15;  /* AGC target value */
  FSCAL1 = 0x00;    /* Reduce the VCO leakage */

  /* Auto ACKs and CRC calculation, default RX and TX modes with FIFOs */
  FRMCTRL0 = FRMCTRL0_AUTOCRC;
#if CC2530_RF_AUTOACK
  FRMCTRL0 |= FRMCTRL0_AUTOACK;
#endif

  /* Disable source address matching and autopend */
  SRCMATCH = 0; /* investigate */

  /* MAX FIFOP threshold */
  FIFOPCTRL = CC2530_RF_MAX_PACKET_LEN;

  TXPOWER = CC2530_RF_TX_POWER;

  RF_TX_LED_OFF();
  RF_RX_LED_OFF();

  rf_flags |= RF_ON;

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  uint8_t i;

  PUTSTRING("RF: Prepare 0x");
  PUTHEX(payload_len + CHECKSUM_LEN);
  PUTSTRING(" bytes\n");

  /*
   * When we transmit in very quick bursts, make sure previous transmission
   * is not still in progress before re-writing to the TX FIFO
   */
  while(FSMSTAT1 & FSMSTAT1_TX_ACTIVE);

  if((rf_flags & RX_ACTIVE) == 0) {
    on();
  }

  CC2530_CSP_ISFLUSHTX();

  PUTSTRING("RF: data = ");
  /* Send the phy length byte first */
  RFD = payload_len + CHECKSUM_LEN; /* Payload plus FCS */
  for(i = 0; i < payload_len; i++) {
    RFD = ((unsigned char *)(payload))[i];
    PUTHEX(((unsigned char *)(payload))[i]);
  }
  PUTSTRING("\n");

  /* Leave space for the FCS */
  RFD = 0;
  RFD = 0;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  uint8_t counter;
  int ret = RADIO_TX_ERR;
  rtimer_clock_t t0;
  transmit_len; /* hush the warning */

  if(!(rf_flags & RX_ACTIVE)) {
    t0 = RTIMER_NOW();
    on();
    rf_flags |= WAS_OFF;
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ONOFF_TIME));
  }

  if(channel_clear() == CC2530_RF_CCA_BUSY) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /*
   * prepare() double checked that TX_ACTIVE is low. If SFD is high we are
   * receiving. Abort transmission and bail out with RADIO_TX_COLLISION
   */
  if(FSMSTAT1 & FSMSTAT1_SFD) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /* Start the transmission */
  RF_TX_LED_ON();
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  CC2530_CSP_ISTXON();

  counter = 0;
  while(!(FSMSTAT1 & FSMSTAT1_TX_ACTIVE) && (counter++ < 3)) {
    clock_delay_usec(6);
  }

  if(!(FSMSTAT1 & FSMSTAT1_TX_ACTIVE)) {
    PUTSTRING("RF: TX never active.\n");
    CC2530_CSP_ISFLUSHTX();
    ret = RADIO_TX_ERR;
  } else {
    /* Wait for the transmission to finish */
    while(FSMSTAT1 & FSMSTAT1_TX_ACTIVE);
    ret = RADIO_TX_OK;
  }
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  if(rf_flags & WAS_OFF) {
    off();
  }

  RIMESTATS_ADD(lltx);

  RF_TX_LED_OFF();

  /* OK, sent. We are now ready to send more */
  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
read(void *buf, unsigned short bufsize)
{
  uint8_t i;
  uint8_t len;
  uint8_t crc_corr;
  int8_t rssi;

  PUTSTRING("RF: Read\n");

  /* Check the length */
  len = RFD;

  /* Check for validity */
  if(len > CC2530_RF_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    PUTSTRING("RF: bad sync\n");

    RIMESTATS_ADD(badsynch);
    CC2530_CSP_ISFLUSHRX();
    return 0;
  }

  if(len <= CC2530_RF_MIN_PACKET_LEN) {
    PUTSTRING("RF: too short\n");

    RIMESTATS_ADD(tooshort);
    CC2530_CSP_ISFLUSHRX();
    return 0;
  }

  if(len - CHECKSUM_LEN > bufsize) {
    PUTSTRING("RF: too long\n");

    RIMESTATS_ADD(toolong);
    CC2530_CSP_ISFLUSHRX();
    return 0;
  }

  RF_RX_LED_ON();

  PUTSTRING("RF: read (0x");
  PUTHEX(len);
  PUTSTRING(" bytes) = ");
  len -= CHECKSUM_LEN;
  for(i = 0; i < len; ++i) {
    ((unsigned char *)(buf))[i] = RFD;
    PUTHEX(((unsigned char *)(buf))[i]);
  }
  PUTSTRING("\n");

  /* Read the RSSI and CRC/Corr bytes */
  rssi = ((int8_t) RFD) - RSSI_OFFSET;
  crc_corr = RFD;

  /* MS bit CRC OK/Not OK, 7 LS Bits, Correlation value */
  if(crc_corr & CRC_BIT_MASK) {
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, crc_corr & LQI_BIT_MASK);
    RIMESTATS_ADD(llrx);
  } else {
    RIMESTATS_ADD(badcrc);
    CC2530_CSP_ISFLUSHRX();
    RF_RX_LED_OFF();
    return 0;
  }

  /* If FIFOP==1 and FIFO==0 then we had a FIFO overflow at some point. */
  if((FSMSTAT1 & (FSMSTAT1_FIFO | FSMSTAT1_FIFOP)) == FSMSTAT1_FIFOP) {
    /*
     * If we reach here means that there might be more intact packets in the
     * FIFO despite the overflow. This can happen with bursts of small packets.
     *
     * Only flush if the FIFO is actually empty. If not, then next pass we will
     * pick up one more packet or flush due to an error.
     */
    if(!RXFIFOCNT) {
      CC2530_CSP_ISFLUSHRX();
    }
  }

  RF_RX_LED_OFF();

  return (len);
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  if(FSMSTAT1 & FSMSTAT1_CCA) {
    return CC2530_RF_CCA_CLEAR;
  }
  return CC2530_RF_CCA_BUSY;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  PUTSTRING("RF: Receiving\n");

  /*
   * SFD high while transmitting and receiving.
   * TX_ACTIVE high only when transmitting
   *
   * FSMSTAT1 & (TX_ACTIVE | SFD) == SFD <=> receiving
   */
  return (FSMSTAT1 & (FSMSTAT1_TX_ACTIVE | FSMSTAT1_SFD) == FSMSTAT1_SFD);
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return (FSMSTAT1 & FSMSTAT1_FIFOP);
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  if(!(rf_flags & RX_ACTIVE)) {
    CC2530_CSP_ISFLUSHRX();
    CC2530_CSP_ISRXON();

    rf_flags |= RX_ACTIVE;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  CC2530_CSP_ISRFOFF();
  CC2530_CSP_ISFLUSHRX();

  rf_flags &= ~RX_ACTIVE;

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    *value = RXENABLE == 0 ? RADIO_POWER_MODE_OFF : RADIO_POWER_MODE_ON;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = (radio_value_t)get_channel();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    *value = get_pan_id();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    *value = get_short_addr();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    if(FRMFILT0 & FRMFILT0_FRAME_FILTER_EN) {
      *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    }
    if(FRMCTRL0 & FRMCTRL0_AUTOACK) {
      *value |= RADIO_RX_MODE_AUTOACK;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = get_tx_power();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = get_cca_threshold();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = CC2530_RF_CHANNEL_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = CC2530_RF_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = OUTPUT_POWER_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      on();
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      off();
      return RADIO_RESULT_OK;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    if(value < CC2530_RF_CHANNEL_MIN || value > CC2530_RF_CHANNEL_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    if(set_channel(value) == CC2530_RF_CHANNEL_SET_ERROR) {
      return RADIO_RESULT_ERROR;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    set_pan_id(value & 0xffff);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    set_short_addr(value & 0xffff);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
                 RADIO_RX_MODE_AUTOACK)) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_frame_filtering((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0);
    set_auto_ack((value & RADIO_RX_MODE_AUTOACK) != 0);

    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    if(value < OUTPUT_POWER_MIN || value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_tx_power(value);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    set_cca_threshold(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  uint8_t *target;
  int i;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    target = dest;
    for(i = 0; i < 8; i++) {
      target[i] = ((uint8_t *)&EXT_ADDR0)[7 - i] & 0xFF;
    }

    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  int i;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    for(i = 0; i < 8; i++) {
      ((uint8_t *)&EXT_ADDR0)[i] = ((uint8_t *)src)[7 - i];
    }

    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver cc2530_rf_driver = {
  init,
  prepare,
  transmit,
  send,
  read,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object
};
/*---------------------------------------------------------------------------*/
