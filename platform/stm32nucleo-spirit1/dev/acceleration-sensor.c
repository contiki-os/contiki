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
 * \addtogroup stm32nucleo-spirit1-temperature-sensor
 * @{
 *
 * \file
 * Driver for the stm32nucleo-spirit1 Temperature sensor (on expansion board)
 */
/*---------------------------------------------------------------------------*/
#ifdef X_NUCLEO_IKS01A1
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "acceleration-sensor.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
static int _active = 1;
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /*Acceleration and Gyroscope sensors share the same hw*/
  if(!st_lib_bsp_imu_6axes_is_initialized()) {
    if(IMU_6AXES_OK == st_lib_bsp_imu_6axes_init()) {
      _active = 1;
    }
  }
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
  int32_t ret_val = 0;
  volatile st_lib_axes_raw_typedef axes_raw_data;

  /* NOTE: this is a demo of mapping ST Nucleo sensors on Contiki sensor API.
   * For a real use case of sensors like acceleration, magneto and gyroscope,
   * it is better to directly call the ST lib to get the three value (X/Y/Z)
   * at once.
   */
  st_lib_bsp_imu_6axes_x_get_axes_raw(&axes_raw_data);

  switch(type) {
  case X_AXIS:
    ret_val = axes_raw_data.AXIS_X;
    break;
  case Y_AXIS:
    ret_val = axes_raw_data.AXIS_Y;
    break;
  case Z_AXIS:
    ret_val = axes_raw_data.AXIS_Z;
    break;
  default:
    break;
  }

  return ret_val;
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
SENSORS_SENSOR(acceleration_sensor, ACCELERATION_SENSOR,
               value, configure, status);
/*---------------------------------------------------------------------------*/
#endif /*X_NUCLEO_IKS01A1*/
/*---------------------------------------------------------------------------*/
/** @} */
