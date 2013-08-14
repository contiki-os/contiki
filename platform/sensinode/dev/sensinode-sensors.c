/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         This module centrally controls all sensors on sensinode devices
 *
 *         It respects configuration in contiki-conf.h
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "dev/sensinode-sensors.h"
#include "sys/energest.h"

const struct sensors_sensor *sensors[] = {
#if ADC_SENSOR_ON
  &adc_sensor,
#endif
#if BUTTON_SENSOR_ON
  &button_1_sensor,
  &button_2_sensor,
#endif
  0
};

unsigned char sensors_flags[(sizeof(sensors) / sizeof(struct sensors_sensor *))];

/*---------------------------------------------------------------------------*/
void
sensinode_sensors_activate()
{
  struct sensors_sensor *sensor;
  sensor = sensors_first();
  while(sensor) {
    sensor->configure(SENSORS_ACTIVE, 1);
    sensor = sensors_next(sensor);
  }
  ENERGEST_ON(ENERGEST_TYPE_SENSORS);
}
/*---------------------------------------------------------------------------*/
void
sensinode_sensors_deactivate()
{
  struct sensors_sensor *sensor;
  sensor = sensors_first();
  while(sensor) {
    sensor->configure(SENSORS_ACTIVE, 0);
    sensor = sensors_next(sensor);
  }
  ENERGEST_OFF(ENERGEST_TYPE_SENSORS);
}
