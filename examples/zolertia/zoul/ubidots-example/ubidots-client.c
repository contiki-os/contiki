/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki-net.h"
#include "dev/leds.h"
#include "rpl.h"
#include "ubidots.h"
#include "dev/button-sensor.h"
#include "dev/sht25.h"
#include "ip64-addr.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(ubidots_example_process, "Ubidots example");
AUTOSTART_PROCESSES(&ubidots_example_process);
/*---------------------------------------------------------------------------*/
static const char *headers[] = {
  "Vary",
  NULL
};
/*---------------------------------------------------------------------------*/
static void
post_collection(void)
{
  uint16_t aux;
  char variable_buffer[VARIABLE_BUF_LEN];

  if(ubidots_prepare_post(NULL) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
    return;
  }

  aux = sht25.value(SHT25_VAL_TEMP);
  memset(variable_buffer, 0, VARIABLE_BUF_LEN);
  snprintf(variable_buffer, VARIABLE_BUF_LEN, "%u", aux);

  if(ubidots_enqueue_value(VARKEY_TEMPERATURE, variable_buffer) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }

  aux = sht25.value(SHT25_VAL_HUM);
  memset(variable_buffer, 0, VARIABLE_BUF_LEN);
  snprintf(variable_buffer, VARIABLE_BUF_LEN, "%u", aux);

  if(ubidots_enqueue_value(VARKEY_HUMIDITY, variable_buffer) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }

  if(ubidots_post() == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
print_reply(ubidots_reply_part_t *r)
{
  switch(r->type) {
  case UBIDOTS_REPLY_TYPE_HTTP_STATUS:
    printf("HTTP Status: %ld\n", *((long int *)r->content));
    break;
  case UBIDOTS_REPLY_TYPE_HTTP_HEADER:
    printf("H: '%s'\n", (char *)r->content);
    break;
  case UBIDOTS_REPLY_TYPE_PAYLOAD:
    printf("P: '%s'\n", (char *)r->content);
    break;
  default:
    printf("Unknown reply type\n");
    break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ubidots_example_process, ev, data)
{
  static struct etimer et;
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(sht25);

  ubidots_init(&ubidots_example_process, headers);

  uip_ipaddr(&ip4addr, 8, 8, 8, 8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  while(1) {

    PROCESS_YIELD();

    if(ev == ubidots_event_established ||
       (ev == PROCESS_EVENT_TIMER && data == &et)) {
      leds_on(LEDS_GREEN);

      post_collection();

    } else if(ev == ubidots_event_post_sent) {
      leds_off(LEDS_GREEN);
      etimer_set(&et, POST_PERIOD);

    } else if(ev == ubidots_event_post_reply_received) {
      print_reply((ubidots_reply_part_t *)data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
