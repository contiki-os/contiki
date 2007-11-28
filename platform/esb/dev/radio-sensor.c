/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: radio-sensor.c,v 1.3 2007/11/28 21:26:35 nifi Exp $
 */

#include "contiki-esb.h"
#include "dev/irq.h"
#include <io.h>

#include "dev/tr1001.h"

const struct sensors_sensor radio_sensor;

unsigned int radio_sensor_signal;

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /* Initialization of ADC12 done by irq */

  radio_sensor_signal = 0;
}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  radio_sensor_signal = ADC12MEM5;
  /*  sensors_changed(&radio_sensor);*/
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  irq_adc12_activate(&radio_sensor, 5, INCH_5 + SREF_0);
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  irq_adc12_deactivate(&radio_sensor, 5);
  radio_sensor_signal = 0;
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return irq_adc12_active(5);
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  switch(type) {
  case RADIO_SENSOR_LAST_PACKET:
    return tr1001_sstrength();
  case RADIO_SENSOR_LAST_VALUE:
  default:
    return ADC12MEM5; /* radio_sensor_signal; */
  }
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
SENSORS_SENSOR(radio_sensor, RADIO_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
