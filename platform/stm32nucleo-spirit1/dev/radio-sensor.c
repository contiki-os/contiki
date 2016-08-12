/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup stm32nucleo-spirit1-peripherals
 * @{
 *
 * \file
 * Driver for the stm32nucleo-spirit1 Radio Sensor (RSSI, LQI, ...)
 */
/*---------------------------------------------------------------------------*/
#include "dev/radio-sensor.h"
#include "dev/sensor-common.h"
#include "lib/sensors.h"
#include "net/packetbuf.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
static int _active;
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /*Nothing to do at the moment, can be used in the future.*/
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  _active = 1;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  _active = 0;
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return _active;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  int32_t radio_sensor;
  float radio_sensor_value;

  switch(type) {
  case RADIO_SENSOR_LAST_PACKET:
    /*TODO: check which method of getting these value is more appropriate */
    radio_sensor_value = DBM_VALUE(packetbuf_attr(PACKETBUF_ATTR_RSSI));
    /* radio_sensor_value = st_lib_spirit_qi_get_rssi_dbm(); */
    radio_sensor = (int32_t)(radio_sensor_value * 10);
    break;
  case RADIO_SENSOR_LAST_VALUE:
  default:
    /*TODO: check which method of getting these value is more appropriate */
    radio_sensor = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    /* radio_sensor = (int32_t) st_lib_spirit_qi_get_lqi(); */
  }

  return radio_sensor;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    init();
    return 1;
  case SENSORS_ACTIVE:
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
  switch(type) {
  case SENSORS_READY:
    return active();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(radio_sensor, RADIO_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */

