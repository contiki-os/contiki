/*
 * Copyright (c) 2015, ICT/COS/NSLab, KTH Royal Institute of Technology
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
 *      dcdc/status read-only parameters for power monitoring
 * \author
 *      Voravit Tanyingyong <voravit@kth.se>
 */

#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "er-coap-observe.h"
#include "../er-dc-test.h"
#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif
#include "dev/dc-status-sensor.h"

static void res_dc_status_obs_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_dc_status_obs_periodic_handler(void);

PERIODIC_RESOURCE(res_dc_status_obs, "title=\"status parameters\"", res_dc_status_obs_get_handler, NULL, NULL, NULL, 10 * CLOCK_SECOND, res_dc_status_obs_periodic_handler);

static uint32_t observe = 0;
static count = 0;
/*---------------------------------------------------------------------------*/
static void
res_dc_status_obs_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

  int vout_value = dc_status_sensor.value(0);
  int vin_value = dc_status_sensor.value(1);
  int iout_value = dc_status_sensor.value(2);
  int iin_value = dc_status_sensor.value(3);

  /* a request comes from a remote host */
  if(request != NULL) {

#if PLATFORM_HAS_LEDS
    /* set red led when receiving a packet */
    leds_on(LEDS_RED);
#endif

    coap_packet_t *const coap_req = (coap_packet_t *)request;

    PRINTF("dcdc/status  GET (%s %u)\n", coap_req->type == COAP_TYPE_CON ? "CON" : "NON", coap_req->mid);

    /* if comes with observe then register it */
    if(coap_get_header_observe(request, &observe)) {
/* PRINTF("OBSERVE set\n"); */
      /* respond with empty ack */
      REST.set_header_content_type(response, observe);
      REST.set_response_payload(response, 0, 0);
    } else { /* if no observe option, then answer to GET request as normal */
/* PRINTF("OBSERVE NOT set\n"); */
      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      REST.set_response_payload(
        response,
        buffer,
        snprintf((char *)buffer, MAX_COAP_PAYLOAD, "ST\tON\nVO\t%d.000\nIO\t%d.000\nVI\t%d.000\nII\t%d.000\n", vout_value, iout_value, vin_value, iin_value));
    }
  } else {  /* this is a notification: need to set payload */
/* PRINTF("NULL REQUEST = PERIODIC\n"); */
    REST.set_response_payload(
      response,
      buffer,
      snprintf((char *)buffer, MAX_COAP_PAYLOAD, "ST\tON\nVO\t%d.000\nIO\t%d.000\nVI\t%d.000\nII\t%d.000\n", vout_value, iout_value, vin_value, iin_value));
  }
  count++;
  printf("PERIODIC %d: ST:ON VO:%d IO:%d VI:%d II:%d\n", count, vout_value, iout_value, vin_value, iin_value);
#if PLATFORM_HAS_LEDS
  /* set yellow led when sending packet */
  leds_on(LEDS_YELLOW);
#endif
}
/*---------------------------------------------------------------------------*/
static void
res_dc_status_obs_periodic_handler()
{
  /* send periodic notification to the observers */
  REST.notify_subscribers(&res_dc_status_obs);
}
