/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 *         SHT25 temperature and humidity sensor driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
#include "lib/sensors.h"

#ifndef SHT25_H_
#define SHT25_H_

/* -------------------------------------------------------------------------- */
#define SHT25_ADDR            0x40
#define SHT25_TEMP_HOLD       0xE3
#define SHT25_HUM_HOLD        0xE5
#define SHT25_TEMP_NO_HOLD    0xF3
#define SHT25_HUM_NO_HOLD     0xF5
#define SHT2X_UREG_WRITE      0xE6
#define SHT2X_UREG_READ       0xE7
#define SHT2X_SOFT_RESET      0XFE
#define SHT2X_NULL            0x00
/* -------------------------------------------------------------------------- */
#define SHT2X_RES_14T_12RH    0x00
#define SHT2X_RES_12T_08RH    0x01
#define SHT2X_RES_13T_10RH    0x80
#define SHT2X_RES_11T_11RH    0x81
#define SHT2X_HEATER_ON       0x04
#define SHT2X_HEATER_OFF      0x00
#define SHT2X_OTP_RELOAD_EN   0x00
#define SHT2X_OTP_RELOAD_DIS  0x02
/* -------------------------------------------------------------------------- */
#define SHT25_VAL_TEMP        SHT25_TEMP_HOLD
#define SHT25_VAL_HUM         SHT25_HUM_HOLD
#define SHT25_ERROR           -1
/* -------------------------------------------------------------------------- */
#define SHT25_SENSOR "SHT25 Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor sht25;
/* -------------------------------------------------------------------------- */
#endif /* ifndef SHT25_H_ */
