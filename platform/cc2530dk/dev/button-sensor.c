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
 */

/*
 * This file contains ISRs: Keep it in the HOME bank.
 */
#include "dev/port.h"
#include "dev/button-sensor.h"
/*---------------------------------------------------------------------------*/
#if BUTTON_SENSOR_ON
static __data struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
static
int value(int type)
{
  return BUTTON_READ() || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static
int status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED();
    }
  return 0;
}
/*---------------------------------------------------------------------------*/
static
int configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    P0INP |= 2; /* Tri-state */
    BUTTON_IRQ_ON_PRESS();
    BUTTON_FUNC_GPIO();
    BUTTON_DIR_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!BUTTON_IRQ_ENABLED()) {
        timer_set(&debouncetimer, 0);
        BUTTON_IRQ_FLAG_OFF();
        BUTTON_IRQ_ENABLE();
      }
    } else {
      BUTTON_IRQ_DISABLE();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
#if MODEL_CC2531
void
port_1_isr(void) __interrupt(P1INT_VECTOR)
#else
void
port_0_isr(void) __interrupt(P0INT_VECTOR)
#endif
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* This ISR is for the entire port. Check if the interrupt was caused by our
   * button's pin. */
  if(BUTTON_IRQ_CHECK()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 8);
      sensors_changed(&button_sensor);
    }
  }

  BUTTON_IRQ_FLAG_OFF();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, configure, status);
#endif /* BUTTON_SENSOR_ON */
