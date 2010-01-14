/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * $Id: esb-sensors.c,v 1.3 2010/01/14 17:39:35 nifi Exp $
 */

/**
 * \file
 *         Functions for turning the ESB sensors on or off
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 * This file will eventually be changed into a better API. This is
 * sufficient for now.
 */

#include "dev/hwconf.h"
#include "dev/irq.h"
#include "sys/energest.h"

HWCONF_PIN(SENSORSWITCH, 5, 5);

/*---------------------------------------------------------------------------*/
void
esb_sensors_init(void)
{
  SENSORSWITCH_SELECT();
  SENSORSWITCH_MAKE_OUTPUT();

  irq_init();
}
/*---------------------------------------------------------------------------*/
void
esb_sensors_on(void)
{
  SENSORSWITCH_CLEAR();
  ENERGEST_ON(ENERGEST_TYPE_SENSORS);
}
/*---------------------------------------------------------------------------*/
void
esb_sensors_off(void)
{
  SENSORSWITCH_SET();
  ENERGEST_OFF(ENERGEST_TYPE_SENSORS);
}
/*---------------------------------------------------------------------------*/
