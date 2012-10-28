/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
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
 *
 * \file
 *	Dummy light-sensor to allow as many programs for sky to compile for Z1
 *
 * \author
 *      Enric M. Calvo <ecalvo@zolertia.com>, adapted from previous work 
 *
 */

#include <stdlib.h>
#include "contiki.h"
#include "dev/light.h"

/*
 * Initialize periodic readings from the 2 photo diodes. The most
 * recent readings will be stored in ADC internal registers/memory.
 */
void
sensors_light_init(void)
{
}

/* Photosynthetically Active Radiation. */
unsigned
sensors_light1(void)
{
  return 0;
}

/* Total Solar Radiation. */
unsigned
sensors_light2(void)
{
  return 0;
}

/*
 * Most of this information taken from
 * http://www.moteiv.com/community/Getting_Data_from_Tmote_Sky%27s_Sensors
 *
 * The Photosynthetically Active Radiation (PAR) sensor as well as the
 * Total Solar Radiation (TSR) sensor uses the 2.5V reference voltage
 * to produce the raw ADC value.

 * The voltage across each sensor is:
 *
 * VsensorPAR = ADCValuePAR/4096 * Vref       (1a)
 * VsensorTSR = ADCValueTSR/4096 * Vref       (1b)
 * where Vref = 2.5V
 *
 * This voltage creates a current through a resistor R=100KOhm and this
 * current has a linear relationship with the light intensity in Lux.
 * IPAR = VsensorPAR / 100,000             (2a) 
 * ITSR = VsensorTSR / 100,000             (2b) 
 *
 * S1087    (PAR) lx = 1e6 * IPAR * 1000   (3a)  
 * S1087-01 (TSR) lx = 1e5 * ITSR * 1000   (3b)
 *
 * lxPAR = 10e9 * ADCValuePAR *(1/4096)* Vref * 10e-5  or
 * lxPAR = 3125* ADCvaluePAR / 512  
 * and 
 * lxTSR = 10e8 * ADCValueTSR *(1/4096)* Vref * 10e-5  or
 * lxTSR = 625* ADCvalueTSR / 1024  
*/

#if 0
/* Photosynthetically Active Radiation in Lux units. */
unsigned
sensors_light1_lux(void)
{
  unsigned temp;
  temp = (uint32_t)ADC12MEM0;

  temp = (temp*3125)>> 9; 
  return (uint16_t)(temp & 0xFFFF); 
}

/* Total Solar Radiation in Lux units. */
unsigned
sensors_light2_lux(void)
{
  unsigned temp;
  temp = (uint32_t)ADC12MEM1;

  temp = (temp*625)>> 10; 
  return (uint16_t)(temp & 0xFFFF); 
}
#endif
