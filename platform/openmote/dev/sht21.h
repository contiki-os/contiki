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

/**
 * \addtogroup platform
 * @{
 *
 * \defgroup openmote The OpenMote Platform
 *
 * \file
 * Header for the SHT21 temperature and humidity sensor in OpenMote-CC2538.
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */

#ifndef __SHT21_H__
#define __SHT21_H__

#include "lib/sensors.h"

#define SENSORS_CONFIG      0xFF
#define SHT21_TEMP_VAL      0
#define SHT21_HUMIDITY_VAL  1

extern const struct sensors_sensor temp_sensor, humidity_sensor;
/*---------------------------------------------------------------------------*/
void sht21_init(void);
void sht21_set_config(uint8_t config);
void sht21_reset(void);
uint8_t sht21_is_present(void);
uint16_t sht21_read_temperature(void);
float sht21_convert_temperature(uint16_t temperature);
uint16_t sht21_read_humidity(void);
float sht21_convert_humidity(uint16_t humidity);
/*---------------------------------------------------------------------------*/
int sht21_value(int type);
int sht21_config(int type, int value);
int sht21_status(int type);
/*---------------------------------------------------------------------------*/
#endif /* ifndef __SHT21_H__ */
/** @} */
