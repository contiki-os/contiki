/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 * \addtogroup openmote-sensors
 * @{
 *
 * \defgroup openmote-sht21-sensor SHT21 sensor
 * @{
 *
 * \file
 * Header file for the SHT21 temperature and humidity sensor driver
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef SHT21_H_
#define SHT21_H_
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the SHT21 sensor
 */
void sht21_init(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Reset the SHT21 sensor
 */
void sht21_reset(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Check if the SHT21 sensor is present
 */
uint8_t sht21_is_present(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the temperature from the SHT21 sensor
 */
uint16_t sht21_read_temperature(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert the temperature from the SHT21 sensor
 */
float sht21_convert_temperature(uint16_t temperature);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the relative humidity from the SHT21 sensor
 */
uint16_t sht21_read_humidity(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert the relative humidity from the SHT21 sensor
 */
float sht21_convert_humidity(uint16_t humidity);
/*---------------------------------------------------------------------------*/
#endif /* SHT21_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
