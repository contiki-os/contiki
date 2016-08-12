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
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISE OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Dag Björklund, Robert Olsson
 * Created : 2005-11-01
 * Updated : $Date: 2010/08/25 19:30:52 $
 *           $Revision: 1.11 $
 */

#include "contiki.h"
#include "dev/battery-sensor.h"
#include <util/delay_basic.h>
#define delay_us(us)   (_delay_loop_2(1 + (us * F_CPU) / 4000000UL))

const struct sensors_sensor battery_sensor;

/* Returns the MCU voltage in mV read from the BATMON MCU register
 * See AtMega chip docs for BATMON details.
 */

static int
value(int type)
{
  uint16_t mv;
  int i;
  /*  Resolution is 75mV if V>=2.55V; and 50mV if V<=2.45V */
  mv = 3675;

  for(i = 0x1f; 1; i--) {

    BATMON = i;
    delay_us(100);
    if(BATMON & 0x20) {
      break;
    }

    if(i == 0x10) { /* Range hi or lo */
      mv = 2500;
    }
    if(i > 0x10) {
      mv -= 75;
    } else {
      mv -= 50;
    }
  }
  return (int)((float)mv);
}
static int
configure(int type, int c)
{
  return 0;
}
static int
status(int type)
{
  return 1;
}
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR, value, configure, status);

