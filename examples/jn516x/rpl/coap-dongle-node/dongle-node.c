/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Theo van Daele <theo.van.daele@nxp.com>
 *
 */
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "tools/rpl-tools.h"
#include "rest-engine.h"
#include "sys/ctimer.h"
#include <stdio.h>
#include "dev/leds.h"

static void ct_callback(void *ptr);
static void put_post_led_toggle_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;
static struct ctimer ct;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) { content_len += snprintf(content + content_len, sizeof(content) - content_len, __VA_ARGS__); } }

/* On dongle, LEDs are connected anti-parallel to DIO pins. */

#define TOGGLE_TIME CLOCK_SECOND
/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
AUTOSTART_PROCESSES(&start_app);
/*---------------------------------------------------------------------------*/

/* Call back for led toggle timer to toggle leds */
static void
ct_callback(void *ptr)
{
  static uint8 toggle_status = 0;
  if(toggle_status) {
    leds_set(LEDS_RED);
  } else {
    leds_set(LEDS_GREEN);
  } ctimer_restart(&ct);
  toggle_status ^= 0x01;
}
/*********** CoAP sensor/ resource ************************************************/
RESOURCE(resource_led_toggle,
         "title=\"Led_toggle\"",
         NULL,
         put_post_led_toggle_handler,
         put_post_led_toggle_handler,
         NULL);
static void
put_post_led_toggle_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  static int led_state = 0;
  unsigned int accept = -1;

  /* Given the way the LEDs are connected to the DIO ports, the LEDs are controlled via direct DIO access. */
  content_len = 0;
  switch(led_state) {
  case (0):
    ctimer_stop(&ct);
    leds_set(LEDS_GREEN);            /* Only LEDS_GREEN on */
    CONTENT_PRINTF("Message from resource: Green LED on");
    led_state = 1;
    break;
  case (1):
    leds_set(LEDS_RED);              /* Only LEDS_RED on */
    CONTENT_PRINTF("Message from resource: Red LED on");
    led_state = 2;
    break;
  case (2):
    leds_set(0);                      /* All LEDS off */
    CONTENT_PRINTF("Message from resource: All LEDs off");
    led_state = 3;
    break;
  case 3:
    /* Both leds toggle */
    CONTENT_PRINTF("Message from resource: LEDs toggle");
    ctimer_restart(&ct);
    led_state = 0;
  default:
    break;
  }
  /* Return message */
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();
  static int is_coordinator = 0;

  /* Switch off dongle leds */

  /* Initialise ct timer, but don't let it run yet */
  ctimer_set(&ct, TOGGLE_TIME, ct_callback, NULL);
  ctimer_stop(&ct);

  /* Start net stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  } printf("Starting RPL node\n");

  rest_init_engine();
  rest_activate_resource(&resource_led_toggle, "Dongle/LED-toggle");

  PROCESS_END();
}
