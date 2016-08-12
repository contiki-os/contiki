/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup zoul-sensors 
 * @{
 *
 * \defgroup remote-zonik Zonik sound sensor
 * @{
 *
 * \file
 * Header file for the Zolertia Zonik sound sensor 
 */
/* -------------------------------------------------------------------------- */
#ifndef ZONIK_H_
#define ZONIK_H_
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "i2c.h"
#include "sys/rtimer.h"
/* -------------------------------------------------------------------------- */
/** \name ZONIK address and definitions
 * @{
 */
#define ZONIK_ADDR                 0x68
#define ZONIK_SENSOR               "Zonik Sound Sensor"

#define ZONIK_INITIAL_WAIT_DELAY   11000L
#define ZONIK_FINAL_WAIT_DELAY     22000L

#ifndef ZONIK_INT_CONF_PORT
#define ZONIK_INT_PORT             I2C_INT_PORT
#else
#define ZONIK_INT_PORT             ZONIK_INT_CONF_PORT
#endif

#ifndef ZONIK_INT_CONF_PIN
#define ZONIK_INT_PIN              I2C_INT_PIN
#else
#define ZONIK_INT_PIN              ZONIK_INT_CONF_PIN
#endif

#define ZONIK_FRAME_SIZE           4

#define ZONIK_WAIT_ACQ             (CLOCK_SECOND / 5)

/* Zonik wait sensor delay: ~800ms */
#define ZONIK_SECOND_INTERVAL      106

/** @} */
/* -------------------------------------------------------------------------- */
/** \name ZONIK error values and definitions
 * @{
 */
#define ZONIK_ACTIVE                SENSORS_ACTIVE
#define ZONIK_HW_INIT               SENSORS_HW_INIT
#define ZONIK_ENABLED               1
#define ZONIK_VALUE_DEACTIVATE      0
#define ZONIK_DISABLED              0xD1ED
#define ZONIK_ERROR                 (-1)
#define ZONIK_DBA_LEQ_VALUE         0x00
#define ZONIK_COUNT_VALUE           0x01
/** @} */
/* -------------------------------------------------------------------------- */
/** \name ZONIK command definitions
 * @{
 */
#define ZONIK_CMD_READ              0x01
/** @} */
/* -------------------------------------------------------------------------- */
/** \name ZONIK sensor type
 * @{
 */
extern const struct sensors_sensor zonik;
/** @} */
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
