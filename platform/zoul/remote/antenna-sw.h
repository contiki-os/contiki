/*
 * Copyright (c) 2015, Zolertia
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
 */
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup remote
 * @{
 *
 * \defgroup remote-antenna RE-Mote Antenna switch
 *
 * Driver for the RE-Mote RF antenna switch, to enable either the internal
 * ceramic antenna or an external one connected to the uFL connector
 * @{
 *
 * \file
 * Header file for the RE-Mote RF antenna switch
 */
/* -------------------------------------------------------------------------- */
#ifndef ANTENNA_SW_H_
#define ANTENNA_SW_H_
/* -------------------------------------------------------------------------- */
#include <stdint.h>
/* -------------------------------------------------------------------------- */
#define ANTENNA_SW_SELECT_2_4GHZ  0
#define ANTENNA_SW_SELECT_SUBGHZ  (1 << ANTENNA_RF_SW_PIN)
#define ANTENNA_SW_SELECT_ERROR   (-1)
/* -------------------------------------------------------------------------- */
/**
 * \brief Init function for the antenna switch
 *
 * The RE-Mote platform allows to programatically select between the 2.4GHz
 * RF interface of the CC2538, or the Sub-1GHz RF interface of the CC1200.
 * The function is set to enable the Sub-1GHz as default,
 * it should be called from the contiki-main initialization process.
 *
 * \return ignored
 */
void antenna_sw_config(void);

/**
 * \brief Function to select between the 2.4GHz or Sub-1GHz RF interface
 *
 * \param val Select antenna.
 *            2.4GHz  : ANTENNA_SW_SELECT_2_4GHZ or
 *            Sub-1GHz: ANTENNA_SW_SELECT_SUBGHZ
 * \return the selected antenna position, or ANTENNA_SW_SELECT_ERROR if not
 *         previously configured
 */
int antenna_sw_select(uint8_t val);

/**
 * \brief Function to read the current status of the RF switch
 *
 *
 * \return the selected antenna position, or ANTENNA_SW_SELECT_ERROR if not
 *         previously configured
 */
int antenna_sw_get(void);
/* -------------------------------------------------------------------------- */
#endif /* ifndef ANTENNA_SW_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
