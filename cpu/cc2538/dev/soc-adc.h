/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-soc-adc cc2538 ADC and RNG
 *
 * Register declarations for the cc2538 ADC and H/W RNG
 * @{
 *
 * \file
 * Header file with register declarations for the cc2538 ADC and H/W RNG
 */
#ifndef SOC_ADC_H_
#define SOC_ADC_H_
/*---------------------------------------------------------------------------*/
/** \name ADC and RNG Register offset declarations
 * @{
 */
#define SOC_ADC_ADCCON1         0x400D7000 /**< ADC Control 1 */
#define SOC_ADC_ADCCON2         0x400D7004 /**< ADC Control 2 */
#define SOC_ADC_ADCCON3         0x400D7008 /**< ADC Control 3 */
#define SOC_ADC_ADCL            0x400D700C /**< ADC Result, least significant part */
#define SOC_ADC_ADCH            0x400D7010 /**< ADC Result, most significant part */
#define SOC_ADC_RNDL            0x400D7014 /**< RNG low byte */
#define SOC_ADC_RNDH            0x400D7018 /**< RNG high byte */
#define SOC_ADC_CMPCTL          0x400D7024 /**< Analog comparator control and status */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_ADCCON1 register bit masks
 * @{
 */
#define SOC_ADC_ADCCON1_EOC     0x00000080 /**< End of conversion */
#define SOC_ADC_ADCCON1_ST      0x00000040 /**< Start conversion */
#define SOC_ADC_ADCCON1_STSEL   0x00000030 /**< Start select */
#define SOC_ADC_ADCCON1_RCTRL   0x0000000C /**< Controls the 16-bit RNG */
#define SOC_ADC_ADCCON1_RCTRL1  0x00000008 /**< RCTRL high bit */
#define SOC_ADC_ADCCON1_RCTRL0  0x00000004 /**< RCTRL low bit */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_ADCCON2 register bit masks
 * @{
 */
#define SOC_ADC_ADCCON2_SREF    0x000000C0 /**< Reference voltage for sequence */
#define SOC_ADC_ADCCON2_SDIV    0x00000030 /**< Decimation rate for sequence */
#define SOC_ADC_ADCCON2_SCH     0x0000000F /**< Sequence channel select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_ADCCON3 register bit masks
 * @{
 */
#define SOC_ADC_ADCCON3_EREF    0x000000C0 /**< Reference voltage for extra */
#define SOC_ADC_ADCCON3_EDIV    0x00000030 /**< Decimation rate for extra */
#define SOC_ADC_ADCCON3_ECH     0x0000000F /**< Single channel select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_ADCCONx registers field values
 * @{
 */
#define SOC_ADC_ADCCON_REF_INT          (0 << 6) /**< Internal reference */
#define SOC_ADC_ADCCON_REF_EXT_SINGLE   (1 << 6) /**< External reference on AIN7 pin */
#define SOC_ADC_ADCCON_REF_AVDD5        (2 << 6) /**< AVDD5 pin */
#define SOC_ADC_ADCCON_REF_EXT_DIFF     (3 << 6) /**< External reference on AIN6-AIN7 differential input */
#define SOC_ADC_ADCCON_DIV_64           (0 << 4) /**< 64 decimation rate (7 bits ENOB) */
#define SOC_ADC_ADCCON_DIV_128          (1 << 4) /**< 128 decimation rate (9 bits ENOB) */
#define SOC_ADC_ADCCON_DIV_256          (2 << 4) /**< 256 decimation rate (10 bits ENOB) */
#define SOC_ADC_ADCCON_DIV_512          (3 << 4) /**< 512 decimation rate (12 bits ENOB) */
#define SOC_ADC_ADCCON_CH_AIN0          0x0 /**< AIN0 */
#define SOC_ADC_ADCCON_CH_AIN1          0x1 /**< AIN1 */
#define SOC_ADC_ADCCON_CH_AIN2          0x2 /**< AIN2 */
#define SOC_ADC_ADCCON_CH_AIN3          0x3 /**< AIN3 */
#define SOC_ADC_ADCCON_CH_AIN4          0x4 /**< AIN4 */
#define SOC_ADC_ADCCON_CH_AIN5          0x5 /**< AIN5 */
#define SOC_ADC_ADCCON_CH_AIN6          0x6 /**< AIN6 */
#define SOC_ADC_ADCCON_CH_AIN7          0x7 /**< AIN7 */
#define SOC_ADC_ADCCON_CH_AIN0_AIN1     0x8 /**< AIN0-AIN1 */
#define SOC_ADC_ADCCON_CH_AIN2_AIN3     0x9 /**< AIN2-AIN3 */
#define SOC_ADC_ADCCON_CH_AIN4_AIN5     0xA /**< AIN4-AIN5 */
#define SOC_ADC_ADCCON_CH_AIN6_AIN7     0xB /**< AIN6-AIN7 */
#define SOC_ADC_ADCCON_CH_GND           0xC /**< GND */
#define SOC_ADC_ADCCON_CH_TEMP          0xE /**< Temperature sensor */
#define SOC_ADC_ADCCON_CH_VDD_3         0xF /**< VDD/3 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_ADC[L:H] register bit masks
 * @{
 */
#define SOC_ADC_ADCL_ADC        0x000000FC /**< ADC Result, least significant part */
#define SOC_ADC_ADCH_ADC        0x000000FF /**< ADC Result, most significant part */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_RND[L:H] register bit masks
 * @{
 */
#define SOC_ADC_RNDL_RNDL       0x000000FF /**< Random value/seed or CRC result low byte */
#define SOC_ADC_RNDH_RNDH       0x000000FF /**< Random value or CRC result/input data, high byte */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SOC_ADC_CMPCTL register bit masks
 * @{
 */
#define SOC_ADC_CMPCTL_EN       0x00000002 /**< Comparator enable */
#define SOC_ADC_CMPCTL_OUTPUT   0x00000001 /**< Comparator output */
/** @} */

#endif /* SOC_ADC_H_ */

/**
 * @}
 * @}
 */
