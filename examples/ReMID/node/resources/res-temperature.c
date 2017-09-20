/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 */

#include "contiki.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "res-interface.h"
#include "dev/temperature-sensor.h"


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

#define MAX_AGE      60
#define INTERVAL_MIN 5
#define INTERVAL_MAX (MAX_AGE - 1)
#define CHANGE       1 

static int32_t interval_counter = INTERVAL_MIN;
static int temperature_old = INT_MIN;

PERIODIC_RESOURCE(res_temperature,
         "title=\"Temperature\";rt=\"Temperature\";obs",
         res_get_handler,
         NULL,
         NULL,
         NULL,
         CLOCK_SECOND,
         res_periodic_handler);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{


 int temperature = temperature_sensor.value(0);

 unsigned int accept = -1;
 REST.get_header_accept(request, &accept);

 if(accept == REST.type.APPLICATION_JSON) {
    CONTENT_PRINTF("{\"Temperature\":%d}", temperature);
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    REST.set_response_payload(response, content, content_len);
    content_len = 0;
    return;
  } else { /* if the client accepts a response payload format other than json, return 406 */
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-type application/json only";
    REST.set_response_payload(response, msg, strlen(msg));
    return;
  }

}


static void
res_periodic_handler()
{
  int temperature = temperature_sensor.value(0);

  ++interval_counter;

  if((abs(temperature - temperature_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
     interval_counter >= INTERVAL_MAX) {
     interval_counter = 0;
     temperature_old = temperature;
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_temperature);
  }
}

