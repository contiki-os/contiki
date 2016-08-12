/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *      CoAP resource handler for the CC26xx LEDs. Slightly modified copy of
 *      the one found in Contiki's original CoAP example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch> (original)
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
static void
res_post_put_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *color = NULL;
  const char *mode = NULL;
  uint8_t led = 0;
  int success = 1;

  if((len = REST.get_query_variable(request, "color", &color))) {
    if(strncmp(color, "r", len) == 0) {
      led = LEDS_RED;
    } else if(strncmp(color, "g", len) == 0) {
      led = LEDS_GREEN;
#if BOARD_SMARTRF06EB
    } else if(strncmp(color, "y", len) == 0) {
      led = LEDS_YELLOW;
    } else if(strncmp(color, "o", len) == 0) {
      led = LEDS_ORANGE;
#endif
    } else {
      success = 0;
    }
  } else {
    success = 0;
  }

  if(success && (len = REST.get_post_variable(request, "mode", &mode))) {
    if(strncmp(mode, "on", len) == 0) {
      leds_on(led);
    } else if(strncmp(mode, "off", len) == 0) {
      leds_off(led);
    } else {
      success = 0;
    }
  } else {
    success = 0;
  }

  if(!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * A simple actuator example, depending on the color query parameter and post
 * variable mode, corresponding led is activated or deactivated
 */
#if BOARD_SENSORTAG || BOARD_LAUNCHPAD
#define RESOURCE_PARAMS "r|g"
#elif BOARD_SMARTRF06EB
#define RESOURCE_PARAMS "r|g|y|o"
#endif

RESOURCE(res_leds,
         "title=\"LEDs: ?color=" RESOURCE_PARAMS ", POST/PUT mode=on|off\";rt=\"Control\"",
         NULL,
         res_post_put_handler,
         res_post_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
/** @} */
