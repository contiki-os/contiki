/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2005-11-01
 */

#include "dev/acc-sensor.h"

const struct sensors_sensor acc_sensor;
static uint8_t active;

/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /* This assumes that some other sensor system already did setup the ADC
  /* (in the case of the sky platform it is sensors_light_init that does it)

  P6SEL |= 0x70;
  P6DIR = 0x00;
  P6OUT = 0x00;

  P2DIR |= 0x48;
  P2OUT |= 0x48;


  /* stop converting immediately
  ADC12CTL0 &= ~ENC;
  ADC12CTL1 &= ~CONSEQ_3;

  /* Configure ADC12_2 to sample channel 11 (voltage) and use
  /* the Vref+ as reference (SREF_1) since it is a stable reference
  ADC12MCTL2 = (INCH_4 + SREF_1);
  ADC12MCTL3 = (INCH_5 + SREF_1);
  ADC12MCTL4 = (INCH_6 + SREF_1);
  /* internal temperature can be read as value(3)
  ADC12MCTL5 = (INCH_10 + SREF_1);

  ADC12CTL1 |= CONSEQ_3;
  ADC12CTL0 |= ENC | ADC12SC;

  /*  Irq_adc12_activate(&acc_sensor, 6, (INCH_11 + SREF_1)); */
  active = 1;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  /*  irq_adc12_deactivate(&acc_sensor, 6);
      acc_value = 0;*/
  active = 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
/*
  switch(type) {
  case 0:
    return ADC12MEM2;
  case 1:
    return ADC12MEM3;
  case 2:
    return ADC12MEM4;
  case 3:
    return ADC12MEM5;
  }*/
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == SENSORS_ACTIVE) {
    if(value) {
      activate();
    } else {
      deactivate();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return active;
  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(acc_sensor, ACC_SENSOR,
	       value, configure, status);
