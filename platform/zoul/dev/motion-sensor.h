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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-motion-sensor Digital motion sensor
 * @{
 *
 * \file
 *         Digital motion sensor header file
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/* -------------------------------------------------------------------------- */
#ifndef MOTION_SENSOR_H_
#define MOTION_SENSOR_H_
/* -------------------------------------------------------------------------- */
/**
 * \name Motion sensor return and operation values
 * @{
 */
#define MOTION_ACTIVE     SENSORS_ACTIVE
#define MOTION_SUCCESS    0
#define MOTION_ERROR      (-1)
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Motion sensor interrupt callback macro
 * @{
 */
#define MOTION_REGISTER_INT(ptr) presence_int_callback = ptr;
extern void (*presence_int_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
#define MOTION_SENSOR "Digital motion sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor motion_sensor;
/* -------------------------------------------------------------------------- */
#endif /* ifndef MOTION_SENSOR_H_ */
/**
 * @}
 * @}
 */
