/*
 * Copyright (c) 2012, Texas Instruments Inc.
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
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-adc cc2538 ADC
 *
 * Driver for the cc2538 ADC controller
 * @{
 *
 * \file
 * Header file for the cc2538 ADC driver
 *
 * \author
 *  Adam Rea <areairs@gmail.com>
 */
#ifndef ADC_H_
#define ADC_H_

#include "contiki.h"

#include <stdint.h>
#include "reg.h"

/*---------------------------------------------------------------------------*/
/** \name ADC register offsets
 * @{
 */
#define ADC_ADCCON1         0x400D7000  /**< ADC control register 1 */ 
#define ADC_ADCCON2         0x400D7004  /**< ADC control register 2 */ 
#define ADC_ADCCON3         0x400D7008  /**< ADC control register 3 */ 
#define ADC_ADCL            0x400D700C  /**< LSB's of ADC conversion */ 
#define ADC_ADCH            0x400D7010  /**< MSB's of ADC conversion */ 
#define ADC_RNDL            0x400D7014  /**< Lower half of RNG data */
#define ADC_RNDH            0x400D7018  /**< Upper half of RNG data */
#define ADC_CMPCTL          0x400D7024  /**< Analog comparator control register */ 
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADCCON1 bit fields
 * @{
 */
#define ADC_ADCCON1_EOC     0x00000080  /**< End of conversion bit */
#define ADC_ADCCON1_EOC_M   0x00000080  /**< End of conversion bit mask */
#define ADC_ADCCON1_EOC_S   7           /**< End of conversion bit shift */
#define ADC_ADCCON1_ST      0x00000040  /**< Start converstion bit */
#define ADC_ADCCON1_ST_M    0x00000040  /**< Start converstion bit mask */
#define ADC_ADCCON1_ST_S    6           /**< Start converstion bit shift */
#define ADC_ADCCON1_STSEL_M 0x00000030  /**< Start conversion style mask */
#define ADC_ADCCON1_STSEL_S 4           /**< Start conversion style shift */
#define ADC_ADCCON1_RCTRL_M 0x0000000C  /**< RNG control mask */
#define ADC_ADCCON1_RCTRL_S 2           /**< RNG control shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADCCON2 bit fields
 * @{
 */
#define ADC_ADCCON2_SREF_M  0x000000C0  /**< Reference voltage mask */
#define ADC_ADCCON2_SREF_S  6           /**< Reference voltage shift */
#define ADC_ADCCON2_SDIV_M  0x00000030  /**< Decimation rate (ENOBs) mask */
#define ADC_ADCCON2_SDIV_S  4           /**< Decimation rate (ENOBs) shift */
#define ADC_ADCCON2_SCH_M   0x0000000F  /**< Sequence channel select mask */
#define ADC_ADCCON2_SCH_S   0           /**< Sequence channel select mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADCCON3 bit fields
 * @{
 */
#define ADC_ADCCON3_EREF_M  0x000000C0  /**< Reference voltage mask */
#define ADC_ADCCON3_EREF_S  6           /**< Reference voltage shift */
#define ADC_ADCCON3_EDIV_M  0x00000030  /**< Decimation rate (ENOBs) mask */
#define ADC_ADCCON3_EDIV_S  4           /**< Decimation rate (ENOBs) shift */
#define ADC_ADCCON3_ECH_M   0x0000000F  /**< Single channel select mask */
#define ADC_ADCCON3_ECH_S   0           /**< Single channel select shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC_ADCL (resultant LSBs) bit fields
 * @{
 */
#define ADC_ADCL_ADC_M      0x000000FC  /**< LSB's of ADC conversion result mask */
#define ADC_ADCL_ADC_S      2           /**< LSB's of ADC conversion result mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC_ADCH (resultant MSBs) bit fields
 * @{
 */
#define ADC_ADCH_ADC_M      0x000000FF  /**< MSB's of ADC conversion result mask */
#define ADC_ADCH_ADC_S      0           /**< MSB's of ADC conversion result shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC_RNDL (random value LSBs) bit fields
 * @{
 */
#define ADC_RNDL_RNDL_M     0x000000FF  /**< LSB's of RNG or CRC result mask */ 
#define ADC_RNDL_RNDL_S     0           /**< LSB's of RNG or CRC result shift */ 
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC_RNDH (random value MSBs) bit fields
 * @{
 */
#define ADC_RNDH_RNDH_M     0x000000FF  /**< MSB's of RNG or CRC result mask */ 
#define ADC_RNDH_RNDH_S     0           /**< LSB's of RNG or CRC result shift */ 
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC_CMPCTL bit fields
 * @{
 */
#define ADC_CMPCTL_EN       0x00000002  /**< Comparator enable bit */ 
#define ADC_CMPCTL_EN_M     0x00000002  /**< Comparator enable bit mask */ 
#define ADC_CMPCTL_EN_S     1           /**< Comparator enable bit shift */ 
#define ADC_CMPCTL_OUTPUT   0x00000001  /**< Comparator output bit */ 
#define ADC_CMPCTL_OUTPUT_M 0x00000001  /**< Comparator output bit mask */ 
#define ADC_CMPCTL_OUTPUT_S 0           /**< Comparator output bit shift */ 
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC resolution settings
 * @{
 */
#define ADC_7_BIT      0x00000000 /**< ADC 7 bit resolution */
#define ADC_9_BIT      0x00000010 /**< ADC 9 bit resolution */
#define ADC_10_BIT     0x00000020 /**< ADC 10 bit resolution */
#define ADC_12_BIT     0x00000030 /**< ADC 12 bit resolution */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC bit mask definitions and related right shift values
 * @{
 */
#define ADC_7_BIT_MASK     0xfe00 /**< ADC 7 bit bit mask */
#define ADC_9_BIT_MASK     0xff80 /**< ADC 9 bit bit mask */
#define ADC_10_BIT_MASK    0xffc0 /**< ADC 10 bit bit mask */
#define ADC_12_BIT_MASK    0xfff0 /**< ADC 12 bit bit mask */
#define ADC_7_BIT_RSHIFT        9 /**< ADC 7 bit right shift value */
#define ADC_9_BIT_RSHIFT        7 /**< ADC 9 bit right shift value */
#define ADC_10_BIT_RSHIFT       6 /**< ADC 10 bit right shift value */
#define ADC_12_BIT_RSHIFT       4 /**< ADC 12 bit right shift value */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC values for setting Vref reference levels
 * @{
 */
#define ADC_REF_INTERNAL    0x00000000 /**< ADC use internal reference */
#define ADC_REF_EXT_AIN7    0x00000040 /**< ADC use external reference on AIN7 pin */
#define ADC_REF_AVDD5       0x00000080 /**< ADC use AVDD5 pin */
#define ADC_REF_EXT_AIN67   0x000000c0 /**< ADC use external reference on 
					*  AIN6-AIN7 differential input pins
					*/
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC commands for reading single and differential signals 
 * @{
 */
#define ADC_AIN0       0x00000000  /**< ADC single ended read PA0 */
#define ADC_AIN1       0x00000001  /**< ADC single ended read PA1 */
#define ADC_AIN2       0x00000002  /**< ADC single ended read PA2 */
#define ADC_AIN3       0x00000003  /**< ADC single ended read PA3 */
#define ADC_AIN4       0x00000004  /**< ADC single ended read PA4 */
#define ADC_AIN5       0x00000005  /**< ADC single ended read PA5 */
#define ADC_AIN6       0x00000006  /**< ADC single ended read PA6 */
#define ADC_AIN7       0x00000007  /**< ADC single ended read PA7 */
#define ADC_AIN01      0x00000008  /**< ADC differential reads PA0-PA1 */
#define ADC_AIN23      0x00000009  /**< ADC differential reads PA2-PA3 */
#define ADC_AIN45      0x0000000a  /**< ADC differential reads PA4-PA5 */
#define ADC_AIN67      0x0000000b  /**< ADC differential reads PA6-PA7 */
#define ADC_GND        0x0000000c  /**< ADC ground */
#define ADC_TEMP_SENS  0x0000000e  /**< ADC on-chip temperature sensor */
#define ADC_VDD        0x0000000f  /**< ADC Vdd/3 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ADC reading kickoff commands
 * @{
 */
#define ADC_FULLSPEED  0x00000010  /**< Full speed */
#define ADC_TIMER_COMP 0x00000020  /**< GP Timer 0, Timer A compare event */
#define ADC_ONE_SHOT   0x00000030  /**< Single sample */
/** @} */

/*---------------------------------------------------------------------------*/
/** \name ADC external function prototypes
 * @{
 */

/** \brief Initialises the ADC controller, configures base operation
 * and interrupts */
void adc_init(void);

/** \brief registers a pin for periodic ADC readings
 * \param port The port of the registered pin
 * \param pin The pin number of the registered pin
 * \param samples_per_second The desired samples per second on this pin
 * \param input A pointer to the callback function for asynch handling
 *
 * \retval Returns a 'uid' used for identifing/unregistering the pin
 */
uint8_t adc_register_pin(uint8_t pin, uint8_t samples_per_second,
			 uint8_t decimation_rate,
			     int(*input)(int, uint8_t) );


/** \brief Unregisters a pin from sampling
 * \param uid The ID issued by adc_register_service
 */
void adc_unregister_pin(uint8_t uid);

PROCESS_NAME(adc_process);
/** @} */

#endif /* ADC_H_ */

/**
 * @}
 * @}
 */
