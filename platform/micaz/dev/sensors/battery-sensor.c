/*
 * Copyright (c) 2010, University of Colombo School of Computing.
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
 * \file
 *         Battery sensor driver.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include <avr/io.h>
#include "dev/battery-sensor.h"
#include "dev/adc.h"


#define BAT_MONITOR_PORT PORTF
#define BAT_MONITOR_PIN_MASK _BV(1)
#define BAT_MONITOR_PORT_DDR DDRF
#define BAT_MONITOR_ADC_CHANNEL 30

const struct sensors_sensor battery_sensor;
static uint8_t active;
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  /* This assumes that some other sensor system already did setup the ADC */
  adc_init();

  /* Enable battery sensor. */
  BAT_MONITOR_PORT |= BAT_MONITOR_PIN_MASK;
  BAT_MONITOR_PORT_DDR |= BAT_MONITOR_PIN_MASK;

  active = 1;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  active = 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return get_adc(BAT_MONITOR_ADC_CHANNEL);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if (c) {
      activate();
    } else {
      deactivate();
    }
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
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR,
	       value, configure, status);
