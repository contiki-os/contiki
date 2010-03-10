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
 * $Id: testsensors.c,v 1.4 2010/03/10 07:35:09 fros4943 Exp $
 */

#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/vib-sensor.h"

#include <stdio.h>

PROCESS(test_sensors_process, "Test sensors process");
AUTOSTART_PROCESSES(&test_sensors_process);

PROCESS_THREAD(test_sensors_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Starting sensors test process\n");
  button_sensor.configure(SENSORS_ACTIVE, 1);
  pir_sensor.configure(SENSORS_ACTIVE, 1);
  vib_sensor.configure(SENSORS_ACTIVE, 1);

  while(1) {
    PROCESS_WAIT_EVENT();

    if (ev == sensors_event) {
      if (data == &button_sensor) {
        printf("Button\n");
      } else if (data == &vib_sensor) {
        printf("Vibration sensor\n");
      } else if (data == &pir_sensor) {
        printf("Passive IR sensor\n");
      } else {
        printf("Unknown sensor\n");
      }
    } else {
      printf("Non-sensor event triggered\n");
    }

  }

  PROCESS_END();
}
