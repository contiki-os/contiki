/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/sht11/sht11-sensor.h"
#include "slip-radio.h"
#include "cmd.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
static void
init(void)
{
}
/*---------------------------------------------------------------------------*/
static int
write_percent_float(char *data, int maxlen, int temp)
{
  int t;
  t = temp % 100;
  if(t < 0) {
    t = -t;
  }
  return snprintf(data, maxlen, "%d.%02d", temp / 100, t);
}
/*---------------------------------------------------------------------------*/
static void
send(void)
{
#define MAX_SIZE 40
  char data[MAX_SIZE];
  int temperature;
  int ms;
  long hum;
  int pos = 0;

  /* SENSORS_ACTIVATE(light_sensor); */
  SENSORS_ACTIVATE(sht11_sensor);

  pos += snprintf(data, MAX_SIZE, "!D");
  /* int light1 = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC); */
  /* int light2 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR); */
  temperature = -3970 + sht11_sensor.value(SHT11_SENSOR_TEMP);
  ms = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  /* this is in * 10000 */
  /* -2.0468  + 0.0367 * ms +  -1.5955e-6 * ms * ms ...too small value...  */
  hum = (-20468L + 367L * ms) / 100L;

  /* SENSORS_DEACTIVATE(light_sensor); */
  SENSORS_DEACTIVATE(sht11_sensor);

  pos += snprintf(&data[pos], MAX_SIZE - pos, "temp=");
  pos += write_percent_float(&data[pos], MAX_SIZE - pos, temperature);
  pos += snprintf(&data[pos], MAX_SIZE - pos, ";hum=");
  pos += write_percent_float(&data[pos], MAX_SIZE - pos, hum);

  cmd_send((uint8_t *)data, pos);
}
/* ---------------------------------------------------------------------- */
const struct slip_radio_sensors slip_radio_sky_sensors = {
  init, send
};
/* ---------------------------------------------------------------------- */
