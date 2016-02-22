/*
 * Copyright (c) 2015, Copyright Robert Olsson / Radio Sensors AB
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
 * Author  : Robert Olsson robert@radio-sensors.com
 * Created : 2015-11-22
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/pulse-sensor.h"
#include "rss2.h"

const struct sensors_sensor pulse_sensor;

#define NP 2

uint32_t volatile pc[NP];

/*
 * Note interrupt sources can be woken up from sleep mode PWR_SAVE
 * Two interrupt ports, #0 green terminal block. #1 pin header via
 * a comparator.
 */

static void
port_irq_ctrl(uint8_t on)
{
  if(on) {

    DDRD &= ~(1 << PD2);
    PORTD &= ~(1 << PD2);
    EIMSK = 0;
    EICRA |= 0x20;    /* Falling edge INT2 */
    EIMSK |= (1 << PD2);  /* Enable interrupt for pin */

    /* p1 port */
    DDRD &= ~(1 << PD3);
    PORTD &= ~(1 << PD3);
    EIMSK |= (1 << PD3);  /* Enable interrupt for pin */
    EICRA |= 0x80;        /* Falling edge */
    PCICR |= (1 << PCIE0);  /* And enable irq PCINT 7:0 */
  } else {
    EICRA = 0;
    PORTD |= (1 << PD2);
    EIMSK &= ~(1 << PD2);    /* Disable interrupt for pin */

    PORTD |= (1 << PD3);
    EIMSK &= ~(1 << PD3);    /* Disable interrupt for pin */
  }
}
ISR(INT2_vect)
{
  if(!(PCICR & (1 << PCIE0))) {
    return;
  }
  pc[0]++;
}

ISR(INT3_vect)
{
  if(!(PCICR & (1 << PCIE0))) {
    return;
  }
  pc[1]++;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(type == 0) {
    return (int)pc[0];
  }
  if(type == 1) {
    return (int)pc[1];
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  port_irq_ctrl(1);  /* Enable pulse counts */
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(pulse_sensor, "Pulse", value, configure, status);
