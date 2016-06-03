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

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 * \author
 *      Wojciech Bober <wojciech.bober@nordicsemi.no>
 */

#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static void
res_post_put_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset);

static void
res_get_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset);

static void
res_event_handler();

/*A simple actuator example, depending on the color query parameter and post variable mode, corresponding led is activated or deactivated*/
EVENT_RESOURCE(res_led3,
               "title=\"LED3\"; obs",
               res_get_handler,
               res_post_put_handler,
               res_post_put_handler,
               NULL,
               res_event_handler
               );

static void
res_post_put_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *payload;
  REST.get_request_payload(request, &payload);

  if (*payload == '0' || *payload == '1') {
    if (*payload == '1') {
      leds_on(LEDS_3);
    } else {
      leds_off(LEDS_3);
    }
    REST.notify_subscribers(&res_led3);
    REST.set_response_status(response, REST.status.CHANGED);
  } else {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%d", (leds_get() & LEDS_3) ? 1 : 0));
}

/*
 * Additionally, res_event_handler must be implemented for each EVENT_RESOURCE.
 * It is called through <res_name>.trigger(), usually from the server process.
 */
static void
res_event_handler()
{
  /* Notify the registered observers which will trigger the res_get_handler to create the response. */
  REST.notify_subscribers(&res_led3);
}
