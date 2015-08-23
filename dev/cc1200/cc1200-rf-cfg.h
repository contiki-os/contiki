/*
 * Copyright (c) 2015, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 *
 * This file is part of the Contiki operating system.
 */

#ifndef CC1200_RF_CFG_H
#define CC1200_RF_CFG_H

#include "contiki.h"

#include <stdint.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/
/*
 * We export the register setup from SmartRF using the standard template
 * "trxEB RF Settings Performance Line" and have therefore to typedef
 * the following struct.
 */
typedef struct cc1200_registerSetting {
  uint16_t addr;
  uint8_t val;
} registerSetting_t;
/*---------------------------------------------------------------------------*/
/* Map SmartRF typedef to reflect Contiki's naming conventions */
typedef registerSetting_t cc1200_register_settings_t;
/*---------------------------------------------------------------------------*/
/* This struct holds the complete configuration for a given mode */
typedef struct cc1200_rf_cfg {
  /* A string describing the mode */
  const char *cfg_descriptor;
  /* A pointer to a register setup exported from SmartRF */
  const cc1200_register_settings_t *register_settings;
  /* The size of the register setup */
  size_t size_of_register_settings;
  /*
   * TX packet lifetime. Maximum duration of a TX packet including preamble,
   * synch word + phy header, payload + CRC.
   */
  rtimer_clock_t tx_pkt_lifetime;
  /* Base frequency in kHz */
  uint32_t chan_center_freq0;
  /* Channel spacing in kHz */
  uint16_t chan_spacing;
  /* The minimum channel */
  uint8_t min_channel;
  /* The maximum channel */
  uint8_t max_channel;
  /* The maximum output power in dBm */
  int8_t max_txpower;
  /*
   * The carrier sense level used for CCA in dBm (int8_t). Limited by
   * CC1200_CONST_CCA_THRESHOLD_MIN and CC1200_CONST_CCA_THRESHOLD_MAX.
   */
  int8_t cca_threshold;
} cc1200_rf_cfg_t;
/*---------------------------------------------------------------------------*/
#endif /* CC1200_RF_CFG_H  */
