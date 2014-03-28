/**
 * \addtogroup dev
 * @{
 */

/**
 * \defgroup radio Radio API
 *
 * The radio API module defines a set of functions that a radio device
 * driver must implement.
 *
 * @{
 */

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 *
 */

/**
 * \file
 *         Header file for the radio API
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <stddef.h>

typedef int radio_value_t;
typedef unsigned radio_param_t;

enum {
  RADIO_PARAM_POWER_MODE,
  RADIO_PARAM_CHANNEL,
  RADIO_PARAM_PAN_ID,
  RADIO_PARAM_16BIT_ADDR,
  /** Address handler take care of address filtering and sending autoack */
  RADIO_PARAM_ADDRESS_HANDLER,
  /** Transmission power in dBm */
  RADIO_PARAM_TXPOWER,
  /** Received signal strength in dBm */
  RADIO_PARAM_RSSI,

  /** 64 bit addresses need to be used with radio.get_object()/set_object() */
  RADIO_PARAM_64BIT_ADDR,

  /** Constants (read only) */
  RADIO_CONST_CHANNEL_MIN,
  RADIO_CONST_CHANNEL_MAX,
  RADIO_CONST_TXPOWER_MIN,
  RADIO_CONST_TXPOWER_MAX
};

/* Radio power modes */
enum {
  RADIO_POWER_MODE_OFF,
  RADIO_POWER_MODE_ON
};

/* Bit flags for the address handler */
#define RADIO_ADDRESS_HANDLER_FILTER  1
#define RADIO_ADDRESS_HANDLER_AUTOACK 2

/* Radio return values when setting or getting radio parameters. */
typedef enum {
  RADIO_RESULT_OK,
  RADIO_RESULT_NOT_SUPPORTED,
  RADIO_RESULT_INVALID_VALUE,
  RADIO_RESULT_ERROR
} radio_result_t;

/**
 * The structure of a device driver for a radio in Contiki.
 */
struct radio_driver {

  int (* init)(void);

  /** Prepare the radio with a packet to be sent. */
  int (* prepare)(const void *payload, unsigned short payload_len);

  /** Send the packet that has previously been prepared. */
  int (* transmit)(unsigned short transmit_len);

  /** Prepare & transmit a packet. */
  int (* send)(const void *payload, unsigned short payload_len);

  /** Read a received packet into a buffer. */
  int (* read)(void *buf, unsigned short buf_len);

  /** Perform a Clear-Channel Assessment (CCA) to find out if there is
      a packet in the air or not. */
  int (* channel_clear)(void);

  /** Check if the radio driver is currently receiving a packet */
  int (* receiving_packet)(void);

  /** Check if the radio driver has just received a packet */
  int (* pending_packet)(void);

  /** Turn the radio on. */
  int (* on)(void);

  /** Turn the radio off. */
  int (* off)(void);

  /** Get a radio parameter */
  radio_result_t (* get_value)(radio_param_t param, radio_value_t *value);

  /** Set a radio parameter */
  radio_result_t (* set_value)(radio_param_t param, radio_value_t value);

  /** Get a radio object (for example a 64 bit address) */
  radio_result_t (* get_object)(radio_param_t param, void *dest, size_t size);

  /** Set a radio object (for example a 64 bit address). The data
      is copied and the object memory will not be accessed after the call.
  */
  radio_result_t (* set_object)(radio_param_t param, const void *src,
                                size_t size);

};

/* Generic radio return values. */
enum {
  RADIO_TX_OK,
  RADIO_TX_ERR,
  RADIO_TX_COLLISION,
  RADIO_TX_NOACK,
};

#endif /* RADIO_H_ */


/** @} */
/** @} */
