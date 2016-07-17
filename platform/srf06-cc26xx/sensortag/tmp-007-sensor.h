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
 * \defgroup sensortag-cc26xx-tmp-sensor SensorTag 2.0 IR thermophile sensor
 *
 * Due to the time required for the sensor to startup, this driver is meant to
 * be used in an asynchronous fashion. The caller must first activate the
 * sensor by calling SENSORS_ACTIVATE(). This will trigger the sensor's startup
 * sequence, but the call will not wait for it to complete so that the CPU can
 * perform other tasks or drop to a low power mode.
 *
 * Once the sensor is stable, the driver will generate a sensors_changed event.
 *
 * The caller should then use value(TMP_007_SENSOR_TYPE_ALL) to read sensor
 * values and latch them. Once completed successfully, individual readings can
 * be retrieved with calls to value(TMP_007_SENSOR_TYPE_OBJECT) or
 * value(TMP_007_SENSOR_TYPE_AMBIENT).
 *
 * Once required readings have been taken, the caller has two options:
 * - Turn the sensor off by calling SENSORS_DEACTIVATE, but in order to take
 *   subsequent readings SENSORS_ACTIVATE must be called again
 * - Leave the sensor on. In this scenario, the caller can simply keep calling
 *   value(TMP_007_SENSOR_TYPE_ALL) to read and latch new values. However
 *   keeping the sensor on will consume more energy
 * @{
 *
 * \file
 * Header file for the Sensortag TI TMP007 infrared thermophile sensor
 */
/*---------------------------------------------------------------------------*/
#ifndef TMP_007_SENSOR_H_
#define TMP_007_SENSOR_H_
/*---------------------------------------------------------------------------*/
#define TMP_007_SENSOR_TYPE_OBJECT    1
#define TMP_007_SENSOR_TYPE_AMBIENT   2
#define TMP_007_SENSOR_TYPE_ALL       3
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor tmp_007_sensor;
/*---------------------------------------------------------------------------*/
#endif /* TMP_007_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
