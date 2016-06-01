/*
 * Copyright (c) 2014, Daniele Alessandrelli.
 * Copyright (c) 2015, Nordic Semiconductor
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
 * \addtogroup nrf52dk-examples Demo projects for nRF52 DK
 * @{
 *
 * \defgroup nrf52dk-coap-demo CoAP demo for nRF52 DK
 * @{
 *
 * \file
 *      Erbium (Er) CoAP observe client example.
 * \author
 *      Daniele Alessandrelli <daniele.alessandrelli@gmail.com>
 * \author
 *      Wojciech Bober <wojciech.bober@nordicsemi.no>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*----------------------------------------------------------------------------*/
#define REMOTE_PORT       UIP_HTONS(COAP_DEFAULT_PORT)
#define OBS_RESOURCE_URI  "lights/led3"
#define SUBS_LED          LEDS_4
#define OBS_LED           LEDS_3

/*----------------------------------------------------------------------------*/
static uip_ipaddr_t server_ipaddr[1]; /* holds the server ip address */
static coap_observee_t *obs;
static struct ctimer ct;
/*----------------------------------------------------------------------------*/
PROCESS(er_example_observe_client, "nRF52 DK Coap Observer Client");
AUTOSTART_PROCESSES(&er_example_observe_client);
/*----------------------------------------------------------------------------*/
static void
observe_led_off(void *d)
{
  leds_off(SUBS_LED);
}
/*----------------------------------------------------------------------------*/
/*
 * Handle the response to the observe request and the following notifications
 */
static void
notification_callback(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag)
{
  int len = 0;
  const uint8_t *payload = NULL;


  PRINTF("Notification handler\n");
  PRINTF("Observee URI: %s\n", obs->url);

  if (notification) {
    len = coap_get_payload(notification, &payload);
  }

  (void)len;

  switch (flag) {
    case NOTIFICATION_OK:
      PRINTF("NOTIFICATION OK: %*s\n", len, (char *)payload);
      if (*payload == '1') {
        leds_on(OBS_LED);
      } else {
        leds_off(OBS_LED);
      }
      break;
    case OBSERVE_OK: /* server accepeted observation request */
      PRINTF("OBSERVE_OK: %*s\n", len, (char *)payload);
      if (*payload == '1') {
        leds_on(OBS_LED);
      } else {
        leds_off(OBS_LED);
      }
      leds_on(SUBS_LED);
      ctimer_set(&ct, CLOCK_SECOND, observe_led_off, NULL);
      break;
    case OBSERVE_NOT_SUPPORTED:
      PRINTF("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *)payload);
      obs = NULL;
      break;
    case ERROR_RESPONSE_CODE:
      PRINTF("ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      PRINTF("NO_REPLY_FROM_SERVER: "
             "removing observe registration with token %x%x\n",
             obs->token[0], obs->token[1]);
      obs = NULL;
      break;
  }
}
/*----------------------------------------------------------------------------*/
/*
 * The main (proto-)thread. It starts/stops the observation of the remote
 * resource every time the timer elapses or the button (if available) is
 * pressed
 */
PROCESS_THREAD(er_example_observe_client, ev, data)
{
  PROCESS_BEGIN();

  uiplib_ipaddrconv(SERVER_IPV6_ADDR, server_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

#if PLATFORM_HAS_BUTTON
  SENSORS_ACTIVATE(button_1);
  SENSORS_ACTIVATE(button_2);
#endif

  /* toggle observation every time the timer elapses or the button is pressed */
  while (1) {
    PROCESS_YIELD();
#if PLATFORM_HAS_BUTTON
    if (ev == sensors_event) {
      if (data == &button_1 && button_1.value(BUTTON_SENSOR_VALUE_STATE) == 0) {
        PRINTF("Starting observation\n");
        obs = coap_obs_request_registration(server_ipaddr, REMOTE_PORT,
                                            OBS_RESOURCE_URI, notification_callback,
                                            NULL);
      }
      if (data == &button_2 && button_2.value(BUTTON_SENSOR_VALUE_STATE) == 0) {
        PRINTF("Stopping observation\n");
        coap_obs_remove_observee(obs);
        obs = NULL;
      }
    }
#endif
  }
  PROCESS_END();
}

/**
 * @}
 * @}
 */
