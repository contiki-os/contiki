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
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: ext-sensor.c,v 1.1 2009/08/25 16:24:49 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Marcus Lundén
 * Created : 2005-11-01
 * Updated : $Date: 2009/08/25 16:24:49 $
 *           $Revision: 1.1 $
 */

#include "dev/ext-sensor.h"
#include <io.h>
#include "dev/irq.h"

#include <stdio.h>

const struct sensors_sensor ext_sensor;

/*---------------------------------------------------------------------------*/
static void
init(void)
{

}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /* This assumes that some other sensor system already did setup the ADC */
  /* (in the case of the JCreate platform it is sensors_light_init before */
  /* acc.meter that does it) */

  /* P6.0 and P6.1 are inputs and peripheral function (A0, A1) selected */
  /* P6SEL |= (1<<P6.0)|(1<<P6.1);*/
  P6SEL |= 0x03;
  P6DIR = 0x00;
  P6OUT = 0x00;


  /* stop converting immediately in order to be able to write to registers */
  ADC12CTL0 &= ~ENC;
  ADC12CTL1 &= ~CONSEQ_3;

  /* Make sure that previous ADCMEM isn't EOS */
  ADC12MCTL5 &= ~(EOS);
  /* We want full voltage range, therefore +ref=Vcc and -ref=GND */
  /* MemReg6 == P6.0/A0 == port "under" logo */
  ADC12MCTL6 = (INCH_0 | SREF_0);
  /* MemReg7 == P6.1/A1 == port "over" logo , End Of (ADC-)Sequence */
  ADC12MCTL7 = (INCH_1 | SREF_0 | EOS);

  /* Start ADC again */
  ADC12CTL1 |= CONSEQ_3;
  ADC12CTL0 |= ENC | ADC12SC;

  /*  Irq_adc12_activate(&ext_sensor, 6, (INCH_11 + SREF_1)); */
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  /* stop converting immediately in order to be able to write to registers */
  ADC12CTL0 &= ~ENC;
  ADC12CTL1 &= ~CONSEQ_3;

  /* Prev ADCMEM == EOS */
  ADC12MCTL5 |= EOS;

  /* Start ADC again */
  ADC12CTL1 |= CONSEQ_3;
  ADC12CTL0 |= ENC | ADC12SC;

  /*  irq_adc12_deactivate(&ext_sensor, 6);
      ext_value = 0;*/
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  /* If previous ADCmem (5) has not EOS set, then extension ports are active */
  if ((ADC12MCTL5&&EOS) == 0)
  {
    return 1;
  }
  else return (0); /* irq_adc12_active(6);*/
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  /* value(0) corresponds to the port under the logo, value(1) to the port over the logo */
  switch(type) {
  case 0:
    return ADC12MEM6;
  case 1:
    return ADC12MEM7;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ext_sensor, EXT_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
