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
 * @(#)$Id: vib-sensor.c,v 1.3 2007/03/28 09:58:47 nifi Exp $
 */

#include "contiki-esb.h"

const struct sensors_sensor vib_sensor;

static unsigned int vib;
static unsigned char flags;

HWCONF_PIN(VIB, 1, 4);
HWCONF_IRQ(VIB, 1, 4);

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  if(VIB_CHECK_IRQ()) {
    ++vib;
    if(flags & VIB_ENABLE_EVENT) {
      sensors_changed(&vib_sensor);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  flags = VIB_ENABLE_EVENT;
  vib = 0;
  VIB_SELECT();
  VIB_MAKE_INPUT();
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  sensors_add_irq(&vib_sensor, VIB_IRQ_PORT());
  VIB_ENABLE_IRQ();
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  VIB_DISABLE_IRQ();
  sensors_remove_irq(&vib_sensor, VIB_IRQ_PORT());
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return VIB_IRQ_ENABLED();
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  return vib;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  if(c) {
    flags |= type & 0xff;
  } else {
    flags &= ~type & 0xff;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return (void *) (((int) (flags & type)) & 0xff);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(vib_sensor, VIB_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
