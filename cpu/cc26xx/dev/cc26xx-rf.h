/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-rf CC26xx RF driver
 *
 * The CC26xx RF has dual capability: It can operate in IEEE 802.15.4 mode at
 * 2.4GHz, but it can also operate in BLE mode. This driver provides a fully
 * contiki-compliant .15.4 functionality, but it also provides some very basic
 * BLE capability.
 *
 * @{
 */
/**
 * \file
 * Header file for the CC26xx RF driver
 */
#ifndef CC26XX_RF_H_
#define CC26XX_RF_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cc26xx-model.h"
#include "dev/radio.h"
/*---------------------------------------------------------------------------*/
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifdef CC26XX_RF_CONF_CHANNEL
#define CC26XX_RF_CHANNEL CC26XX_RF_CONF_CHANNEL
#else
#define CC26XX_RF_CHANNEL 18
#endif /* CC26XX_RF_CONF_CHANNEL */

#ifdef CC26XX_RF_CONF_AUTOACK
#define CC26XX_RF_AUTOACK CC26XX_RF_CONF_AUTOACK
#else
#define CC26XX_RF_AUTOACK 1
#endif /* CC26XX_RF_CONF_AUTOACK */

#if (CC26XX_RF_CONF_BLE_SUPPORT) && (CC26XX_MODEL_CPU_VARIANT == 2650)
#define CC26XX_RF_BLE_SUPPORT CC26XX_RF_CONF_BLE_SUPPORT
#else
#define CC26XX_RF_BLE_SUPPORT 0
#endif
/*---------------------------------------------------------------------------
 * RF Config
 *---------------------------------------------------------------------------*/
/* Constants */
#define CC26XX_RF_CHANNEL_MIN            11
#define CC26XX_RF_CHANNEL_MAX            26
#define CC26XX_RF_CHANNEL_SPACING         5
#define CC26XX_RF_CHANNEL_SET_ERROR      -1
#define CC26XX_RF_MAX_PACKET_LEN        127
#define CC26XX_RF_MIN_PACKET_LEN          4
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the device name to use with the BLE advertisement/beacon daemon
 * \param interval The interval (ticks) between two consecutive beacon bursts
 * \param name The device name to advertise
 *
 * If name is NULL it will be ignored. If interval==0 it will be ignored. Thus,
 * this function can be used to configure a single parameter at a time if so
 * desired.
 */
void cc26xx_rf_ble_beacond_config(clock_time_t interval, const char *name);

/**
 * \brief Start the BLE advertisement/beacon daemon
 * \return 1: Success, 0: Failure
 *
 * Before calling this function, the name to advertise must first be set by
 * calling cc26xx_rf_ble_beacond_set_adv_name(). Otherwise, this function will
 * return an error.
 */
uint8_t cc26xx_rf_ble_beacond_start(void);

/**
 * \brief Stop the BLE advertisement/beacon daemon
 */
void cc26xx_rf_ble_beacond_stop(void);
/*---------------------------------------------------------------------------*/
#endif /* CC26XX_RF_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
