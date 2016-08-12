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
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-adc-interface Zoul Generic ADC interface
 *
 * Driver for the Zoul ADC interface
 *
 * This driver supports analogue sensors connected to ADC1, ADC2, ADC3,
 * ADC4, ADC5 and ADC6 inputs. ADC6 is shared with the user button, so disable
 * user button if ADC6 is needed.
 * This is controlled by the type argument of the value() function. Possible
 * choices are:
 *
 * - ZOUL_SENSORS_ADC1
 * - ZOUL_SENSORS_ADC2
 * - ZOUL_SENSORS_ADC3
 * - ZOUL_SENSORS_ADC4
 * - ZOUL_SENSORS_ADC5
 * - ZOUL_SENSORS_ADC6
 *
 * To initialize the ADC sensors use the configure() function, using as first
 * argument SENSORS_HW_INIT, and choose which ADC channels to enable passing as
 * second argument any single or combined (sum) values as below:
 *
 * - Either use multiple values (i.e ZOUL_SENSORS_ADC1 + ZOUL_SENSORS_ADC2)
 * - ZOUL_SENSORS_ADC_ALL (all channels above)
 *
 * Using an invalid combination will return ZOUL_SENSORS_ERROR.
 *
 * The decimation rate can be set by passing
 * ZOUL_SENSORS_CONFIGURE_TYPE_DECIMATION_RATE as the type argument to the
 * configure() function and then specifying the rate through the value
 * argument. Valid values are:
 *
 * - SOC_ADC_ADCCON_DIV_64 (64 bit rate)
 * - SOC_ADC_ADCCON_DIV_128 (128 bit rate)
 * - SOC_ADC_ADCCON_DIV_256 (256 bit rate)
 * - SOC_ADC_ADCCON_DIV_512 (512 bit rate)
 * @{
 *
 * \file
 * Header file for the Zoul ADC interface
 */
/*---------------------------------------------------------------------------*/
#ifndef ADC_ZOUL_H_
#define ADC_ZOUL_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/soc-adc.h"
/*---------------------------------------------------------------------------*/
/**
 * \name Generic ADC sensors
 * @{
 */
#define ADC_ZOUL "ADC sensor interface"
#define ADC_SENSORS_PORT_BASE    GPIO_PORT_TO_BASE(ADC_SENSORS_PORT)

#ifdef ADC_SENSORS_CONF_REFERENCE
#define ADC_SENSORS_REFERENCE ADC_SENSORS_CONF_REFERENCE
#else
#define ADC_SENSORS_REFERENCE SOC_ADC_ADCCON_REF_AVDD5
#endif

/*
 * PA0-PA3 are hardcoded to UART0 and the user button for most Zolertia
 * platforms, the following assumes PA0-1 shall not be used as ADC input, else
 * re-write the below definitions
 */
#define ZOUL_SENSORS_ADC_MIN     2  /**< PA1 pin mask */

/* ADC phidget-like connector ADC1 */
#if ADC_SENSORS_ADC1_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC1        GPIO_PIN_MASK(ADC_SENSORS_ADC1_PIN)
#else
#define ZOUL_SENSORS_ADC1        0
#endif
/* ADC phidget-like connector ADC2 */
#if ADC_SENSORS_ADC2_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC2        GPIO_PIN_MASK(ADC_SENSORS_ADC2_PIN)
#else
#define ZOUL_SENSORS_ADC2        0
#endif
/* ADC phidget-like connector ADC3 */
#if ADC_SENSORS_ADC3_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC3        GPIO_PIN_MASK(ADC_SENSORS_ADC3_PIN)
#else
#define ZOUL_SENSORS_ADC3        0
#endif
/* ADC phidget-like connector ADC4 */
#if ADC_SENSORS_ADC4_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC4        GPIO_PIN_MASK(ADC_SENSORS_ADC4_PIN)
#else
#define ZOUL_SENSORS_ADC4        0
#endif
/* ADC phidget-like connector ADC5 */
#if ADC_SENSORS_ADC5_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC5        GPIO_PIN_MASK(ADC_SENSORS_ADC5_PIN)
#else
#define ZOUL_SENSORS_ADC5        0
#endif

/* ADC phidget-like connector ADC6 */
#if ADC_SENSORS_ADC6_PIN >= ZOUL_SENSORS_ADC_MIN
#define ZOUL_SENSORS_ADC6        GPIO_PIN_MASK(ADC_SENSORS_ADC6_PIN)
#else
#define ZOUL_SENSORS_ADC6        0
#endif
/*
 * This is safe as the disabled sensors should have a zero value thus not
 * affecting the mask operations
 */
#define ZOUL_SENSORS_ADC_ALL     (ZOUL_SENSORS_ADC1 + ZOUL_SENSORS_ADC2 + \
                                  ZOUL_SENSORS_ADC3 + ZOUL_SENSORS_ADC4 + \
                                  ZOUL_SENSORS_ADC5 + ZOUL_SENSORS_ADC6)
/** @} */
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor adc_zoul;
/*---------------------------------------------------------------------------*/
#endif /* ADC_ZOUL_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

