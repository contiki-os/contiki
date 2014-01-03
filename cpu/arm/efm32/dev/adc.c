/*
 * Copyright (c) 2013, Kerlink
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
 */

/**
 * \addtogroup efm32-devices
 * @{
 */

/**
 * \file
 *         EFM32 ADC driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/clock.h>
#include "contiki.h"
#include "adc.h"
#include "em_cmu.h"
#include "em_emu.h"

static volatile uint8_t _u8_conv_complete = 0;

/**************************************************************************//**
 * @brief ADC0 interrupt handler. Simply clears interrupt flag.
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
  _u8_conv_complete = 1;
  ADC_IntClear(ADC0, ADC_IF_SINGLE);
}

uint16_t adc_get_value(uint8_t u8_adc_channel, ADC_Ref_TypeDef adcref)
{
  ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;
  uint32_t rawvalue = 0;

  /* Set reference  */
  sInit.reference = adcref;
  sInit.input = u8_adc_channel;
  ADC_InitSingle(ADC0, &sInit);

  _u8_conv_complete = 0;

  ADC_Start(ADC0, adcStartSingle);
  /* Wait in EM1 for ADC to complete */
  EMU_EnterEM1();

  // Make sure it's ADC interrupt
  // TODO : timeout
  while(_u8_conv_complete == 0);

  rawvalue = ADC_DataSingleGet(ADC0);

  return (uint16_t) rawvalue;
}

/**
 * Get internal temperature
 * @return Celsius temperature (in tenth degrees)
 */
int16_t adc_get_inttemp(void)
{
  uint32_t rawvalue = 0;

  float temp;
  /* Factory calibration temperature from device information page. */
  float cal_temp_0 = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
                             >> _DEVINFO_CAL_TEMP_SHIFT);

  float cal_value_0 = (float)((DEVINFO->ADC0CAL2
                               & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
                              >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

  /* Temperature gradient (from datasheet) */
  float t_grad = -6.27;

  rawvalue = adc_get_value(adcSingleInpTemp, adcRef1V25);

  temp = (cal_temp_0 - ((cal_value_0 - rawvalue)  / t_grad));
  //temp = ((cal_value_0 - rawvalue)  / t_grad);

  return (int16_t)(temp*10);
}

uint16_t adc_get_value_mv(uint8_t u8_adc_channel)
{
  uint32_t rawvalue = 0;

  // First try smaller range (more precise)
  rawvalue = adc_get_value(u8_adc_channel, adcRef1V25);
  // Check for 1V25 overflow
  if(rawvalue <= 0xFF0)
  {
    // 4096 -> 1250mv
    return (uint16_t)(rawvalue * 1250 / 4096);
  }
  else
  {
    // Convert again with higher range
    rawvalue = adc_get_value(u8_adc_channel, adcRef2V5);

    if(rawvalue <= 0xFF0)
    {
      // 4096 -> 2500mV
      return (uint16_t)(rawvalue * 2500 / 4096);
    }
    else
    {
      // Final conversion with maximum range
      rawvalue = adc_get_value(u8_adc_channel, adcRef5VDIFF);
      // 4096 -> 5000mV
      return (uint16_t)(rawvalue * 5000 / 4096);
    }
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Initialize ADC Module
 */
void adc_init(void)
{
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;

  /* Enable ADC Clock */
  CMU_ClockEnable(cmuClock_ADC0, true);

  /* Initialize timebases */
  init.timebase = ADC_TimebaseCalc(0);
  init.prescale = ADC_PrescaleCalc(400000,0);
  ADC_Init(ADC0, &init);

  /* Setup interrupt generation on completed conversion. */
  ADC_IntEnable(ADC0, ADC_IF_SINGLE);
  NVIC_EnableIRQ(ADC0_IRQn);
}

/** @} */
