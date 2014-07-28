/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup SeedEye Contiki SEEDEYE Platform
 *
 * @{
 */

/**
 * \file   platform/seedeye/dev/button-sensor.c
 * \brief  Button Sensor
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-04-24
 */

#include <dev/button-sensor.h>

#include <p32xxxx.h>

#include <pic32_irq.h>

#define button_read PORTDbits.RD5

const struct sensors_sensor button_sensor;

static struct timer debouncetimer;

static int status(int type);

static uint8_t sensor_status = 0;

ISR(_CHANGE_NOTICE_VECTOR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(timer_expired(&debouncetimer)) {
    timer_set(&debouncetimer, CLOCK_SECOND / 4);
    sensors_changed(&button_sensor);
  }

  IFS1CLR = _IFS1_CNIF_MASK;


  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/*---------------------------------------------------------------------------*/
void
button_press(void)
{
  sensors_changed(&button_sensor);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return button_read || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch (type) {
  case SENSORS_ACTIVE:
    if(value) {
      if(!status(SENSORS_ACTIVE)) {
        timer_set(&debouncetimer, 0);

        TRISDbits.TRISD5 = 1;

        CNCON = 0;
        CNCONSET =  1 << _CNCON_ON_POSITION | 1 << _CNCON_SIDL_POSITION;
        CNEN = 1 << _CNEN_CNEN14_POSITION;
        CNPUE = 1 << _CNPUE_CNPUE14_POSITION;

        IEC1CLR = _IEC1_CNIE_MASK;
        IFS1CLR = _IFS1_CNIF_MASK;

        IPC6CLR = _IPC6_CNIP_MASK;
        IPC6SET = 6 << _IPC6_CNIP_POSITION;

        IEC1SET = 1 << _IEC1_CNIE_POSITION;

        sensor_status = 1;
      }
    }
    return 1;
  }

  sensor_status = 0;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return sensor_status;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/

/** @} */
