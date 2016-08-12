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
 * Driver for the stm32nucleo-spirit1 User Button
 */
/*---------------------------------------------------------------------------*/
#include "dev/button-sensor.h"
#include "lib/sensors.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
static int _active = 0;
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /* See spirit1_appli.c for the Callback: it triggers the relevant
   * sensors_changed event
   */
  st_lib_bsp_pb_init(BUTTON_USER, BUTTON_MODE_EXTI);
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
  return active;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return st_lib_bsp_pb_get_state(BUTTON_USER);
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
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
