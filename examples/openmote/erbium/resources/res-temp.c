/*
 * Copyright (c) 2013, Johan Bregell, i3tex AB
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
 * \addtogroup openmote-examples 
 * @{
 * 
 * \defgroup openmote-erbium-server Erbium server for the OpenMote OpenBattery platform
 * @{
 * 
 * \file
 *      Temperature Resource Openmote
 * \author
 *      Johan Bregell <johan@bregell.se>
 */

#include <stdlib.h>
#include <string.h>
#include "dev/leds.h"
#include "rest-engine.h"
#include "dev/sht21.h"
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm ("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      asm ("nop");
    }
  }
}

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_temp,
         "title=\"Temperature\";rt=\"temperature\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  float res = 100;
  uint16_t sht21_raw_temp = temp_sensor.value(SHT21_TEMP_VAL);
  float sht21_temp = sht21_convert_temperature(sht21_raw_temp);
  int sht21_temp_h = (int)sht21_temp;
  int sht21_temp_d = (int)((sht21_temp - (float)sht21_temp_h)*res);
  unsigned int accept = -1;

  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN){
    REST.set_header_content_type(request, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%d",sht21_temp_h, sht21_temp_d);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
    fade(LEDS_GREEN);
  } else if(accept == REST.type.APPLICATION_JSON){
    REST.set_header_content_type(request, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'temp':%d.%d}", sht21_temp_h, sht21_temp_d);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
    fade(LEDS_GREEN);
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
    fade(LEDS_RED);
  }
}
/**
 * @}
 * @}
 */
 
