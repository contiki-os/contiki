/*
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
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
/** \addtogroup rf-core
 * @{
 *
 * \defgroup rf-switch RF Switch
 *
 * Header file for RF switch support
 *
 * @{
 *
 * \file
 * Header file with definitions related to RF switch support
 */
/*---------------------------------------------------------------------------*/
#ifndef RF_SWITCH_H_
#define RF_SWITCH_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifdef RF_SWITCH_CONF_PATH_2_4GHZ
#define RF_SWITCH_PATH_2_4GHZ RF_SWITCH_CONF_PATH_2_4GHZ
#else
#define RF_SWITCH_PATH_2_4GHZ 0
#endif

#ifdef RF_SWITCH_CONF_PATH_SUBGHZ
#define RF_SWITCH_PATH_SUBGHZ RF_SWITCH_CONF_PATH_SUBGHZ
#else
#define RF_SWITCH_PATH_SUBGHZ 1
#endif
/*---------------------------------------------------------------------------*/
#ifdef RF_SWITCH_CONF_ENABLE
#define RF_SWITCH_ENABLE RF_SWITCH_CONF_ENABLE
#else
#define RF_SWITCH_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
#if RF_SWITCH_ENABLE
/**
 * \brief Initialise RF switch pin states.
 */
void rf_switch_init(void);

/**
 * \brief Power up the RF switch.
 */
void rf_switch_power_up(void);

/**
 * \brief Power down the RF switch.
 */
void rf_switch_power_down(void);

/**
 * \brief Select RF path
 * \param path The RF path to select on the switch.
 *
 * The path argument can take values RF_SWITCH_PATH_xyz
 */
void rf_switch_select_path(uint8_t path);
#else
#define rf_switch_init()
#define rf_switch_power_up()
#define rf_switch_power_down()
#define rf_switch_select_path(p)
#endif /* RF_SWITCH_ENABLE */
/*---------------------------------------------------------------------------*/
#endif /* RF_SWITCH_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
