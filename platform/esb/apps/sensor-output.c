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
 * @(#)$Id: sensor-output.c,v 1.4 2010/01/15 10:37:04 nifi Exp $
 */
#include "contiki-esb.h"

#include <stdio.h>

PROCESS(sensor_output_process, "Sensor output");

AUTOSTART_PROCESSES(&sensor_output_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_output_process, ev, data)
{
  struct sensors_sensor *s;

  PROCESS_BEGIN();

  /* Activate some sensors to get sensor events */
  button_sensor.configure(SENSORS_ACTIVE, 1);
  pir_sensor.configure(SENSORS_ACTIVE, 1);
  vib_sensor.configure(SENSORS_ACTIVE, 1);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

    s = (struct sensors_sensor *)data;
    printf("%s %d\n", s->type, s->value(0));

    if (data == &button_sensor) leds_invert(LEDS_YELLOW);
    if (data == &pir_sensor) leds_invert(LEDS_GREEN);
    if (data == &vib_sensor) leds_invert(LEDS_RED);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
