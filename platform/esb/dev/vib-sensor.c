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
 * @(#)$Id: vib-sensor.c,v 1.5 2010/02/08 00:00:45 nifi Exp $
 */

#include "dev/vib-sensor.h"
#include "dev/irq.h"
#include "dev/hwconf.h"

const struct sensors_sensor vib_sensor;

static unsigned int vib;

#define VIB_IRQ() 4
HWCONF_PIN(VIB, 1, VIB_IRQ());
HWCONF_IRQ(VIB, 1, VIB_IRQ());

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  ++vib;
  sensors_changed(&vib_sensor);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return vib;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    vib = 0;
    VIB_SELECT();
    VIB_MAKE_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!VIB_IRQ_ENABLED()) {
        irq_port1_activate(VIB_IRQ(), irq);
        VIB_ENABLE_IRQ();
      }
    } else {
      VIB_DISABLE_IRQ();
      irq_port1_deactivate(VIB_IRQ());
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
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return VIB_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(vib_sensor, VIB_SENSOR,
               value, configure, status);
