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
 * \addtogroup rf-core
 * @{
 *
 * \defgroup rf-core-ble CC13xx/CC26xx BLE driver
 *
 * @{
 *
 * \file
 * Header file for the CC13xx/CC26xx BLE driver
 */
/*---------------------------------------------------------------------------*/
#ifndef RF_BLE_H_
#define RF_BLE_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "rf-core/rf-core.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifdef RF_BLE_CONF_ENABLED
#define RF_BLE_ENABLED RF_BLE_CONF_ENABLED
#else
#define RF_BLE_ENABLED 1
#endif
/*---------------------------------------------------------------------------*/
#define RF_BLE_IDLE   0
#define RF_BLE_ACTIVE 1
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
void rf_ble_beacond_config(clock_time_t interval, const char *name);

/**
 * \brief Start the BLE advertisement/beacon daemon
 * \return RF_CORE_CMD_OK: Success, RF_CORE_CMD_ERROR: Failure
 *
 * Before calling this function, the name to advertise must first be set by
 * calling rf_ble_beacond_config(). Otherwise, this function will return an
 * error.
 */
uint8_t rf_ble_beacond_start(void);

/**
 * \brief Stop the BLE advertisement/beacon daemon
 */
void rf_ble_beacond_stop(void);

/**
 * \brief Check whether the BLE beacond is currently active
 * \retval 1 The radio is in BLE mode
 * \retval 0 The BLE daemon is not active, or disabled
 */
uint8_t rf_ble_is_active(void);
/*---------------------------------------------------------------------------*/
#endif /* RF_BLE_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
