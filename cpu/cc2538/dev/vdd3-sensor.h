/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
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
 *
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
 * \addtogroup cc2538-sensors
 * @{
 *
 * \defgroup cc2538-vdd3-sensor CC2538 VDD3 Sensor
 *
 * Driver for the CC2538 VDD3 sensor
 *
 * This driver can return the raw as well as the converted value of the sensor
 * reading. This is controlled by the type argument of the sensor driver's
 * value() function. The choices for the type argument are:
 * - REMOTE_SENSORS_VALUE_TYPE_RAW (value() returns the raw reading)
 * - REMOTE_SENSORS_VALUE_TYPE_CONVERTED (value() returns mV)
 * @{
 *
 * \file
 * Header file for the CC2538 VDD3 Sensor Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef VDD3_SENSOR_H_
#define VDD3_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
/**
 * \name VDD3 sensors
 * @{
 */
#define VDD3_SENSOR "VDD3"
/** @} */
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor vdd3_sensor;
/*---------------------------------------------------------------------------*/
#endif /* VDD3_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
