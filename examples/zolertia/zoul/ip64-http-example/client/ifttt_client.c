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
#include "http-socket.h"
#include "ip64-addr.h"
#include "dev/leds.h"
#include "rpl.h"
#include "dev/button-sensor.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
static struct http_socket s;
static char data_received[HTTP_CLIENT_BUFFER_LEN];
static char url_buffer[HTTP_CLIENT_BUFFER_LEN];
static int bytes_received = 0;
static int restarts;
static struct ctimer reconnect_timer;
/*---------------------------------------------------------------------------*/
static void callback(struct http_socket *s, void *ptr,
                     http_socket_event_t e,
                     const uint8_t *data, uint16_t datalen);
/*---------------------------------------------------------------------------*/
PROCESS(http_example_process, "IFTTT HTTP Example");
AUTOSTART_PROCESSES(&http_example_process);
/*---------------------------------------------------------------------------*/
static void
reconnect(void *dummy)
{
  rpl_set_mode(RPL_MODE_MESH);
  leds_on(LEDS_GREEN);
  http_socket_post(&s, url_buffer, NULL, 0, NULL, callback, NULL);
}
/*---------------------------------------------------------------------------*/
static void
restart(void)
{
  int scale;
  restarts++;
  printf("Number of restarts %d\n", restarts);
  leds_off(LEDS_GREEN);

  scale = restarts;
  if(scale > 5) {
    scale = 5;
  }

  ctimer_set(&reconnect_timer, random_rand() % ((CLOCK_SECOND * 10) << scale),
              reconnect, NULL);
}
/*---------------------------------------------------------------------------*/
static void
callback(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{
  uint8_t i;

  leds_off(LEDS_GREEN);

  if(e == HTTP_SOCKET_ERR) {
    printf("HTTP socket error\n");
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    printf("HTTP socket error: timed out\n");
    restart();
  } else if(e == HTTP_SOCKET_ABORTED) {
    printf("HTTP socket error: aborted\n");
    restart();
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    printf("HTTP socket error: hostname not found\n");
    restart();
  } else if(e == HTTP_SOCKET_CLOSED) {

    if(bytes_received) {
      printf("HTTP socket received data:\n\n");
      for(i=0; i<bytes_received; i++) {
        printf("%c", data_received[i]);
      }
      printf("\n");
      bytes_received = 0;
    }

  } else if(e == HTTP_SOCKET_DATA) {
    strcat(data_received, data);
    bytes_received += datalen;
    printf("HTTP socket received %d bytes of data\n", datalen);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(http_example_process, ev, data)
{
  // static char buf_ptr[64];
  static struct etimer et;
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  uip_ipaddr(&ip4addr, 8, 8, 8, 8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);

  leds_on(LEDS_RED);

  etimer_set(&et, CLOCK_SECOND * 35);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  leds_off(LEDS_RED);
  leds_on(LEDS_BLUE);

  memset(url_buffer, 0, HTTP_CLIENT_BUFFER_LEN);
  snprintf(url_buffer, HTTP_CLIENT_BUFFER_LEN,
           "http://maker.ifttt.com/trigger/%s/with/key/%s",
           IFTTT_EVENT, IFTTT_KEY);

  // memset(buf_ptr, 0, 64);
  // snprintf(buf_ptr, 64, "{\"value1\":\"%u\"}", 8);

  http_socket_init(&s);

  restarts = 0;

  while(1) {
    PROCESS_YIELD();
    if((ev == sensors_event) && (data == &button_sensor)) {
      if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
        BUTTON_SENSOR_PRESSED_LEVEL) {
        leds_on(LEDS_GREEN);
        printf("Button pressed! sending a POST to IFTTT\n");
        http_socket_post(&s, url_buffer, NULL, 0, NULL, callback, NULL);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
