/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * @(#)$$
 */

/**
 * \file
 *         Device drivers implementation for MTS300 sensor board.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include "mts300.h"
/*---------------------------------------------------------------------------*/
void
sounder_on()
{  SOUNDER_DDR |= SOUNDER_MASK;
  SOUNDER_PORT &= ~SOUNDER_MASK;
  SOUNDER_PORT |= SOUNDER_MASK;
}
/*---------------------------------------------------------------------------*/
void 
sounder_off()
{
  SOUNDER_PORT &= ~(SOUNDER_MASK);
  SOUNDER_DDR &= ~(SOUNDER_MASK);
}
/*---------------------------------------------------------------------------*/
void 
adc_init()
{
  ADMUX = 0;
  /* AVCC with external capacitor at AREF pin. */
  //ADMUX |= _BV(REFS0)
  /* Disable ADC interrupts. */   
  ADCSRA &= ~( _BV(ADIE) | _BV(ADIF) );
  /* Set ADC prescaler to 64 and clear interrupt flag. */
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADIE);
  
}
/*---------------------------------------------------------------------------*/
/* Poll based approach. The interrupt based adc is currently not used.
   The ADC result is right adjusted. First 8 bits(from left) are in ADCL and
   other two bits are in ADCH. See Atmega128 datasheet page 228. */
uint16_t 
get_adc(int channel)
{
  uint16_t reading;

  ADMUX |= (channel & 0x1F);

  /* Disable ADC interrupts. */
  ADCSRA &= ~_BV(ADIE);
  /* Clear previous interrupts. */
  ADCSRA |= _BV(ADIF);
  /* Enable ADC and start conversion. */
  ADCSRA |= _BV(ADEN) | _BV(ADSC);
  /* Wait until conversion is completed. */
  while ( ADCSRA & _BV(ADSC) );
  /* Get first 8 bits.  */
  reading = ADCL;
  /* Get last two bits. */
  reading |= (ADCH & 3) << 8;
  /* Disable ADC. */
  ADCSRA &= ~_BV(ADEN);
  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_light()
{
  uint16_t reading;

  /* Enable light sensor. */
  LIGHT_PORT |= LIGHT_PIN_MASK;       
  LIGHT_PORT_DDR |= LIGHT_PIN_MASK;
  /* Disable temperature sensor. */
  TEMP_PORT_DDR &= ~TEMP_PIN_MASK;
  TEMP_PORT &= ~TEMP_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(LIGHT_ADC_CHANNEL);
  /* Disable light sensor. */
  LIGHT_PORT &= ~LIGHT_PIN_MASK;       
  LIGHT_PORT_DDR &= ~LIGHT_PIN_MASK;
    return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_temp()
{
  uint16_t reading;

  /* Disable light sensor. */
  LIGHT_PORT &= ~LIGHT_PIN_MASK;       
  LIGHT_PORT_DDR &= ~LIGHT_PIN_MASK;
  /* Enable temperature sensor. */
  TEMP_PORT_DDR |= TEMP_PIN_MASK;
  TEMP_PORT |= TEMP_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(TEMP_ADC_CHANNEL);
  /* Disable temperature sensor. */
  TEMP_PORT_DDR &= ~TEMP_PIN_MASK;
  TEMP_PORT &= ~TEMP_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_accx()
{
  uint16_t reading;

  /* Enable accelerometer. */
  ACCEL_PORT_DDR |= ACCEL_PIN_MASK;
  ACCEL_PORT |= ACCEL_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(ACCELX_ADC_CHANNEL);
  /* Enable accelerometer. */
  ACCEL_PORT_DDR &= ~ACCEL_PIN_MASK;
  ACCEL_PORT &= ~ACCEL_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_accy()
{
  uint16_t reading;

  /* Enable accelerometer. */
  ACCEL_PORT_DDR |= ACCEL_PIN_MASK;
  ACCEL_PORT |= ACCEL_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(ACCELY_ADC_CHANNEL);
  /* Enable accelerometer. */
  ACCEL_PORT_DDR &= ~ACCEL_PIN_MASK;
  ACCEL_PORT &= ~ACCEL_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_magx()
{
  uint16_t reading;

  /* Enable magnetometer. */
  MAGNET_PORT_DDR |= MAGNET_PIN_MASK;
  MAGNET_PORT |= MAGNET_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(MAGNETX_ADC_CHANNEL);
  /* Enable magnetometer. */
  MAGNET_PORT_DDR &= ~MAGNET_PIN_MASK;
  MAGNET_PORT &= ~MAGNET_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_magy()
{
  uint16_t reading;

  /* Enable magnetometer. */
  MAGNET_PORT_DDR |= MAGNET_PIN_MASK;
  MAGNET_PORT |= MAGNET_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(MAGNETY_ADC_CHANNEL);
  /* Enable magnetometer. */
  MAGNET_PORT_DDR &= ~MAGNET_PIN_MASK;
  MAGNET_PORT &= ~MAGNET_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/
uint16_t 
get_mic()
{
  uint16_t reading;

    /* Enable mic. */
  MIC_PORT_DDR |= MIC_PIN_MASK;
  MIC_PORT |= MIC_PIN_MASK;
  /* Read ADC. */
  reading = get_adc(MIC_ADC_CHANNEL);
  /* Enable mic. */
  MIC_PORT_DDR &= ~MIC_PIN_MASK;
  MIC_PORT &= ~MIC_PIN_MASK;

  return reading;
}
/*---------------------------------------------------------------------------*/

