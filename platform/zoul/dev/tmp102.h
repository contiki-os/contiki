/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-tmp102-sensor TMP102 Sensor
 *
 * Driver for the TMP102 sensor
 *
 * The TMP102 driver returns the converted temperature value in centiCelsius
 * with 2 digits precision, to get Celsius just divide by 100.
 * @{
 *
 * \file
 * Header file for the TMP102 Sensor Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef TMP102_H_
#define TMP102_H_
#include <stdio.h>
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name Generic TMP102 sensor
 * @{
 */
/* -------------------------------------------------------------------------- */
#define TMP102_ADDR           0x48 /**< TMP102 slave address */
#define TMP102_TEMP           0x00 /**< TMP102 temperature data register */
/** @} */
/* -------------------------------------------------------------------------- */
#endif /* ifndef TMP102_H_ */
/*---------------------------------------------------------------------------*/

/** \brief Initialiser for the TMP102 sensor driver */
void tmp102_init(void);

/** \brief Get a temperature reading from the TMP102 sensor */
uint8_t tmp102_read(uint16_t *data);

/**
 * @}
 * @}
 */
