/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISE OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Robert Olsson
 * Created : 2015-10-27
 * Updated : $Date: 2010/08/25 19:30:52 $
 *           $Revision: 1.11 $
 */

#include "contiki.h"
#include "dev/temp_mcu-sensor.h"
#include <util/delay_basic.h>
#define delay_us(us)   (_delay_loop_2(1 + (us * F_CPU) / 4000000UL))

const struct sensors_sensor temp_mcu_sensor;

/* Returns the MCU temp in C*10 read from the BATMON MCU register
 * See AtMega chip docs for BATMON details.
 */

static int
value(int type)
{
  uint16_t v;

  ADCSRB |= (1 << MUX5); /* this bit buffered till ADMUX written to! */
  ADMUX = 0xc9;

  /* ADC on /32 ADC start */
  ADCSRA = (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN); /* Enable the ADC */

  while(!(ADCSRB & (1 << AVDDOK))) ;  /* Wait for AVDD ok */
  while(!(ADCSRB & (1 << REFOK))) ;  /* Wait for ref ok  */

  ADCSRA |= (1 << ADSC);      /* Throwaway conversion */
  while
  (ADCSRA & (1 << ADSC)) ;

  ADCSRA |= (1 << ADSC);       /* Start conversion */
  while
  (ADCSRA & (1 << ADSC)) ;

  v = ADC;

  /* Disable the ADC to save power */
  ADCSRA &= ~_BV(ADEN);
  /* ADMUX = 0;                //turn off internal vref */
  return (int)((double)(v * 1.13 - 272.8) * 10);
}
static int
configure(int type, int c)
{
  return 0;
}
static int
status(int type)
{
  return 1;
}
SENSORS_SENSOR(temp_mcu_sensor, TEMP_MCU_SENSOR, value, configure, status);

