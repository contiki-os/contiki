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
 * Additional modifications by Kévin Roussel <Kevin.Roussel@inria.fr>
 */

#ifndef RADIO_H_
#define RADIO_H_

#include "contiki-conf.h"

#ifndef RADIO_CONF_EXTENDED_API
#define RADIO_CONF_EXTENDED_API 0
#endif

/** configuration constants (for capabilities) of radio transceivers */
typedef enum radio_const_e {

  /** minimal value for working channel -- usually 11 for 802.15.4 */
  RADIO_MIN_CHANNEL,
  /** maximal value for working channel -- usually 26 for 802.15.4 */
  RADIO_MAX_CHANNEL,
  /** minimal value for emitting power */
  RADIO_MIN_TX_POWER,
  /** maximal value for emitting power */
  RADIO_MAX_TX_POWER,

} radio_const_t;

/** configuration parameters (for capabilities) of radio transceivers */
typedef enum radio_param_e {

  /** working channel (= frequency) used by the radio transceiver */
  RADIO_CHANNEL,
  /** current short (16-bit) address used by the radio transceiver */
  RADIO_SHORT_ADDRESS,
  /** current PAN ID (16-bit) used by the radio transceiver */
  RADIO_PAN_ID,
  /** current IEEE (64-bit) address used by the radio transceiver */
  RADIO_IEEE_ADDRESS,
  /** transmission power */
  RADIO_TX_POWER,
  /** Clear-Channel Assessment threshold */
  RADIO_CCA_THRESHOLD,
  /** set/unset promiscuous mode */
  RADIO_PROMISCUOUS_MODE,
  /** enable_disable automatic ACK on reception */
  RADIO_AUTOACK,

} radio_param_t;


/* Return values of configuration functions */
typedef enum radio_conf_result_e {

  RADIO_CONF_OK                = 0,
  RADIO_CONF_UNKNOWN_CONST     = -100,
  RADIO_CONF_UNKNOWN_PARAM     = -101,
  RADIO_CONF_UNAVAILABLE_CONST = -102,
  RADIO_CONF_UNAVAILABLE_PARAM = -103,
  RADIO_CONF_READ_ONLY_PARAM   = -104,
  RADIO_CONF_WRITE_ONLY_PARAM  = -105,

} radio_conf_result_t ;

#include "contiki-conf.h"

#ifndef RADIO_CONF_EXTENDED_API
#define RADIO_CONF_EXTENDED_API 0
#endif

/** configuration constants (for capabilities) of radio transceivers */
typedef enum radio_const_e {

  /** minimal value for working channel -- usually 11 for 802.15.4 */
  RADIO_MIN_CHANNEL,
  /** maximal value for working channel -- usually 26 for 802.15.4 */
  RADIO_MAX_CHANNEL,
  /** minimal value for emitting power */
  RADIO_MIN_TX_POWER,
  /** maximal value for emitting power */
  RADIO_MAX_TX_POWER,

} radio_const_t;

/** configuration parameters (for capabilities) of radio transceivers */
typedef enum radio_param_e {

  /** working channel (= frequency) used by the radio transceiver */
  RADIO_CHANNEL,
  /** current short (16-bit) address used by the radio transceiver */
  RADIO_SHORT_ADDRESS,
  /** current PAN ID (16-bit) used by the radio transceiver */
  RADIO_PAN_ID,
  /** current IEEE (64-bit) address used by the radio transceiver */
  RADIO_IEEE_ADDRESS,
  /** transmission power */
  RADIO_TX_POWER,
  /** Clear-Channel Assessment threshold */
  RADIO_CCA_THRESHOLD,
  /** set/unset promiscuous mode */
  RADIO_PROMISCUOUS_MODE,
  /** enable_disable automatic ACK on reception */
  RADIO_AUTOACK,

} radio_param_t;


/* Return values of configuration functions */
typedef enum radio_conf_result_e {

  RADIO_CONF_OK                = 0,
  RADIO_CONF_UNKNOWN_CONST     = -100,
  RADIO_CONF_UNKNOWN_PARAM     = -101,
  RADIO_CONF_UNAVAILABLE_CONST = -102,
  RADIO_CONF_UNAVAILABLE_PARAM = -103,
  RADIO_CONF_READ_ONLY_PARAM   = -104,
  RADIO_CONF_WRITE_ONLY_PARAM  = -105,

} radio_conf_result_t ;

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

#if RADIO_CONF_EXTENDED_API

  /**
   * Get configuration constant(s) of the radio transceiver;
   *
   * ATTENTION: caller has to allocate the ad-hoc "placeholder"
   * in memory on which 'value' shall be a pointer.
   */
  radio_conf_result_t (* get_config_const)(radio_const_t cst_id, void *value);

  /**
   * Set a config parameter of the radio transceiver
   *
   * The value argument's semantics depend on the value of param_id:
   * - RADIO_CHANNEL: value must be a pointer to an int and may
   *   take values as defined in the relevant standard. i.e. for .15.4 in the
   *   2.4 GHz band, valid values are in [11, 26], for the sub GHz band, values
   *   are in [0, 10]
   * - RADIO_SHORT_ADDRESS and RADIO_PAN_ID: value must be a uint16_t in
   *   machine byte order
   * - RADIO_IEEE_ADDRESS: value is a pointer to a buffer holding the IEEE
   *   address. value[0] will hold the MSB and value[7] the LSB
   * - RADIO_PROMISCUOUS_MODE and RADIO_AUTOACK: 0: disable the feature, any
   *   other non-zero value: enable
   * - RADIO_TX_POWER: value is a pointer to an int representing dBm. When used
   *   to set TX power, this will represent max allowable value, i.e. set the
   *   max power you can that's not greater than this value. Valid values are
   *   in [-128 , 127]
   * - RADIO_CCA_THRESHOLD: value is a pointer to an int representing dBm
   */
  radio_conf_result_t (* set_param)(radio_param_t param_id, void *value);

  /**
   * Get a config parameter of the radio transceiver;
   *
   * ATTENTION: caller has to allocate the ad-hoc "placeholder"
   * in memory on which 'value' shall be a pointer.
   *
   * The semantics of 'value' are the same as in set_param
   */
  radio_conf_result_t (* get_param)(radio_param_t param_id, void *value);

#endif /* RADIO_CONF_EXTENDED_API */

};

/* Generic radio return values... */
enum {

  /* ... of transmission functions */
  RADIO_TX_OK,
  RADIO_TX_ERR,
  RADIO_TX_COLLISION,
  RADIO_TX_NOACK,

};

};

#endif /* RADIO_H_ */

/** @} */
/** @} */

