/*
 * Copyright (c) 2017, Indian Institute of Science
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
/*---------------------------------------------------------------------------*/
/**
 * \file
 *    TI ADS7828 header file
 * \author
 *    Sumankumar Panchal <suman@ece.iisc.ernet.in>
 *    Akshay P M <akshaypm@ece.iisc.ernet.in>
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-ads7828 TI ADS7828
 *
 * Driver for TI ADS7828 12-bit, 8-Channel Sampling ADC conveter with I2C
 * interface.
 *
 * @{
 *
 * \file
 * Header file for TI ADS7828
 */
/*---------------------------------------------------------------------------*/
#ifndef ADS7828_H_
#define ADS7828_H_

#define ADS7828_ADDR 0x48

#define ADS7828_CH0 0x00
#define ADS7828_CH2 0x10
#define ADS7828_CH4 0x20
#define ADS7828_CH6 0x30
#define ADS7828_CH1 0x40
#define ADS7828_CH3 0x50
#define ADS7828_CH5 0x60
#define ADS7828_CH7 0x70

#define ADS7828_MODE_SE     0x80
#define ADS7828_MODE_DIFF   0x00

#define ADS7828_MODE_POWER_DOWN_BETWEEN_CONVERSIONS  0x00
#define ADS7828_MODE_INT_REF_OFF_ADC_ON 0x04
#define ADS7828_MODE_INT_REF_ON_ADC_OFF 0x08
#define ADS7828_MODE_INT_REF_ON_ADC_ON 0x0C

#define ADS7828_ERROR   -1
#define ADS7828_SUCCESS 0x00
/*---------------------------------------------------------------------------*/
#endif /* ifndef ADS7828_H_ */
/*---------------------------------------------------------------------------*/
/** \brief Initialiser for TI ADS7828*/
void ads7828_init();

/** \brief Get sensor reading connected with TI ADS7828 */
uint16_t ads7828_read_channel(uint8_t reg, uint16_t *data);
/**
 * @}
 * @}
 */
