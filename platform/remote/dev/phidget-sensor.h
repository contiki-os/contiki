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
 * \addtogroup remote-sensors
 * @{
 *
 * \defgroup remote-phidget-sensor Re-Mote Generic Phidget Sensor
 *
 * Driver for the Re-Mote phidget ADC sensor
 *
 * This driver supports phidgets connected to both the ADC2 and AND3 channels.
 * This is controlled by the type argument of the value() function. Possible
 * choices are:
 * - PHIDGET_SENSORS_ADC2 (channel 2)
 * - PHIDGET_SENSORS_ADC3 (channel 3)
 *
 * The decimation rate can be set by passing
 * REMOTE_SENSORS_CONFIGURE_TYPE_DECIMATION_RATE as the type argument to the
 * configure() function and then specifying the rate through the value
 * argument. Valid values are:
 * - SOC_ADC_ADCCON_DIV_64 (64 bit rate)
 * - SOC_ADC_ADCCON_DIV_128 (128 bit rate)
 * - SOC_ADC_ADCCON_DIV_256 (256 bit rate)
 * - SOC_ADC_ADCCON_DIV_512 (512 bit rate)
 * @{
 *
 * \file
 * Header file for the Re-Mote Generic Driver for Phidget/ADC sensors
 */
/*---------------------------------------------------------------------------*/
#ifndef PHIDGET_SENSOR_H_
#define PHIDGET_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/soc-adc.h"
/*---------------------------------------------------------------------------*/
/**
 * \name Generic phidget sensor
 * @{
 */
#define PHIDGET_SENSOR "Phidget ADC"

#define PHIDGET_SENSORS_ADC2      2 /**< 3V3 ADC phidget-like connector ADC2 */
#define PHIDGET_SENSORS_ADC3      3 /**< 3V3 ADC phidget-like connector ADC3 */
/** @} */
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor phidget_sensor;
/*---------------------------------------------------------------------------*/
#endif /* PHIDGET_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
