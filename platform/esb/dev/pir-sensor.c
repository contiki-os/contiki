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
 */

#include "dev/pir-sensor.h"
#include "dev/irq.h"
#include "dev/hwconf.h"

const struct sensors_sensor pir_sensor;

static unsigned int pir;

#define PIR_IRQ() 3
HWCONF_PIN(PIR, 1, PIR_IRQ());
HWCONF_IRQ(PIR, 1, PIR_IRQ());

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  ++pir;
  sensors_changed(&pir_sensor);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return pir;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    pir = 0;
    PIR_SELECT();
    PIR_MAKE_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!PIR_IRQ_ENABLED()) {
        irq_port1_activate(PIR_IRQ(), irq);
        PIR_ENABLE_IRQ();
      }
    } else {
      PIR_DISABLE_IRQ();
      irq_port1_deactivate(PIR_IRQ());
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
    return PIR_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(pir_sensor, PIR_SENSOR,
               value, configure, status);
