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
 *      dcdc/co2 testing coap with CO2 sensor from SenseAir
 * \author
 *      Voravit Tanyingyong <voravit@kth.se>
 */

/* #include <math.h> */
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "er-dc-test.h"
#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif
#include "dev/co2_sa_kxx-sensor.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_dc_co2, "title=\"co2 reading\"", res_get_handler, NULL, NULL, NULL);

/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *)request;

  int co2 = co2_sa_kxx_sensor.value(CO2_SA_KXX_CO2); // 0 is CO2_SA_KXX_CO2

#if PLATFORM_HAS_LEDS
  /* set red led when receiving a packet */
  leds_on(LEDS_RED);
#endif

  PRINTF("dcdc/co2 GET (%s %u)\n", coap_req->type == COAP_TYPE_CON ? "CON" : "NON", coap_req->mid);

  /* Code 2.05 CONTENT is default. */
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  /* REST.set_header_max_age(response, 30); */
  REST.set_response_payload(
    response,
    buffer,
/*    snprintf((char *)buffer, MAX_COAP_PAYLOAD, "VMX\t%d.000\nIMX\t%d.000\nINT\t%d\n", v_max, i_max, interval)); */
    snprintf((char *)buffer, MAX_COAP_PAYLOAD, "CO2\t%d\n", co2));

#if PLATFORM_HAS_LEDS
  /* set yellow led when sending packet */
  leds_on(LEDS_YELLOW);
#endif
}

