/*
 * Remote Power Switch Example for the Seed-Eye Board
 * Copyright (c) 2013, Giovanni Pellerano
 *
 * Ownership: Scuola Superiore Sant'Anna (http://www.sssup.it) and
 * Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 * (http://www.cnit.it).
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
 * \addtogroup Remote Power Switch Example for the Seed-Eye Board
 *
 * @{
 */

/**
 * \file   remotepowerswitch.c
 * \brief  Remote Power Switch Example for the Seed-Eye Board
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2013-01-24
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "contiki-net.h"

#include "rest-engine.h"

#include "dev/leds.h"

#include <p32xxxx.h>

void
toggle_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  leds_toggle(LEDS_RED);

  PORTEbits.RE0 = !PORTEbits.RE0;
}
RESOURCE(resource_toggle, "title=\"Red LED\";rt=\"Control\"", toggle_handler, toggle_handler, toggle_handler, NULL);


PROCESS(remote_power_switch, "Remote Power Switch");

AUTOSTART_PROCESSES(&remote_power_switch);

PROCESS_THREAD(remote_power_switch, ev, data)
{
  PROCESS_BEGIN();

  rest_init_engine();

  TRISEbits.TRISE0 = 0;
  PORTEbits.RE0 = 0;

  rest_activate_resource(&resource_toggle, "actuators/powerswitch");

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}

/** @} */
