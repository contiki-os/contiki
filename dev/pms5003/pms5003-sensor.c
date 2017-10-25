/*
 * Copyright (c) 2017, Peter Sjodin, KTH Royal Institute of Technology
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
 * Author  : Peter Sjodin, KTH Royal Institute of Technology
 * Created : 2017-04-21
 */

#include <stdlib.h>

#include "contiki.h"
#include "lib/sensors.h"
#include "pms5003.h"
#include "pms5003-sensor.h"

const struct sensors_sensor pms5003_sensor;

enum {
  ON, OFF
};
static uint8_t state = OFF;

/*---------------------------------------------------------------------------*/
PROCESS(pms5003_sensor_process, "PMS5003 sensor process");
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  switch(type) {
  case PMS5003_SENSOR_PM1:
    return pms5003_pm1();
  case PMS5003_SENSOR_PM2_5:
    return pms5003_pm2_5();
  case PMS5003_SENSOR_PM10:
    return pms5003_pm10();
  case PMS5003_SENSOR_PM1_ATM:
    return pms5003_pm1_atm();
  case PMS5003_SENSOR_PM2_5_ATM:
    return pms5003_pm2_5_atm();
  case PMS5003_SENSOR_PM10_ATM:
    return pms5003_pm10_atm();
  case PMS5003_SENSOR_DB0_3:
    return pms5003_db0_3();
  case PMS5003_SENSOR_DB0_5:
    return pms5003_db0_5();
  case PMS5003_SENSOR_DB1:
    return pms5003_db1();
  case PMS5003_SENSOR_DB2_5:
    return pms5003_db2_5();
  case PMS5003_SENSOR_DB5:
    return pms5003_db5();
  case PMS5003_SENSOR_DB10:
    return pms5003_db10();
  case PMS5003_SENSOR_TIMESTAMP:
    return pms5003_timestamp();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return state == ON;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if(c) {
      if(!status(SENSORS_ACTIVE)) {
        pms5003_init();
        process_start(&pms5003_sensor_process, NULL);
        state = ON;
      }
    } else {
      pms5003_off();
      state = OFF;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(pms5003_sensor, "pms5003",
               value, configure, status);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pms5003_sensor_process, ev, data)
{

  PROCESS_BEGIN();
  while(1) {
    do {
      PROCESS_WAIT_EVENT();
    } while(ev != pms5003_event);
    sensors_changed(&pms5003_sensor);
  }
  PROCESS_END();
}
