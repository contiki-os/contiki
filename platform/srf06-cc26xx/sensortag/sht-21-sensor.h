/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-sht-sensor SensorTag 2.0 Humidity Sensor
 *
 * Due to the time required between triggering a reading and the reading
 * becoming available, this driver is meant to be used in an asynchronous
 * fashion. The caller must first activate the sensor by calling
 * SENSORS_ACTIVATE(). This will trigger a cycle which will read both
 * temperature and humidity, but the call will not wait for the cycle to
 * complete so that the CPU can perform other tasks or drop to a low power
 * mode.
 *
 * Once readings are available, the driver will generate a sensors_changed
 * event.
 *
 * Calls to .status() will return the driver's state which could indicate that
 * a measurement is in progress (different return values for each) or that
 * readings are ready.
 *
 * Multiple calls to value() will simply return the reading that was latched
 * after the last cycle. In order to obtain fresh readings, a new cycle must be
 * started by a new call to SENSORS_ACTIVATE.
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26ss Sensirion SHT21 Humidity sensor
 */
/*---------------------------------------------------------------------------*/
#ifndef SHT_21_SENSOR_H_
#define SHT_21_SENSOR_H_
/*---------------------------------------------------------------------------*/
#define SHT_21_SENSOR_TYPE_TEMP        1
#define SHT_21_SENSOR_TYPE_HUMIDITY    2
/*---------------------------------------------------------------------------*/
/**
 * \name SHT21 driver states
 * @{
 */
#define SHT_21_SENSOR_STATUS_DISABLED         0 /**< Not initialised */
#define SHT_21_SENSOR_STATUS_INITIALISED      1 /**< Initialised but idle */
#define SHT_21_SENSOR_STATUS_READING_TEMP     2 /**< Temp reading in progress */
#define SHT_21_SENSOR_STATUS_READING_HUMIDITY 3 /**< Humidity reading in progress */
#define SHT_21_SENSOR_STATUS_READINGS_READY   4 /**< Both readings ready */
/** @} */
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor sht_21_sensor;
/*---------------------------------------------------------------------------*/
#endif /* SHT_21_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
