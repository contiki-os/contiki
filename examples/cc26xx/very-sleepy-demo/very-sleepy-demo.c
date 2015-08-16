/*
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
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
#include "sys/process.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "net/netstack.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "rest-engine.h"
#include "er-coap.h"

#include "ti-lib.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
/* Normal mode duration params in seconds */
#define NORMAL_OP_DURATION_DEFAULT 10
#define NORMAL_OP_DURATION_MIN     10
#define NORMAL_OP_DURATION_MAX     60
/*---------------------------------------------------------------------------*/
/* Observer notification period params in seconds */
#define PERIODIC_INTERVAL_DEFAULT  30
#define PERIODIC_INTERVAL_MIN      30
#define PERIODIC_INTERVAL_MAX      86400 /* 1 day */
/*---------------------------------------------------------------------------*/
#define VERY_SLEEPY_MODE_OFF 0
#define VERY_SLEEPY_MODE_ON  1
/*---------------------------------------------------------------------------*/
#define MAC_CAN_BE_TURNED_OFF  0
#define MAC_MUST_STAY_ON       1

#define KEEP_MAC_ON_MIN_PERIOD 10 /* secs */
/*---------------------------------------------------------------------------*/
#define PERIODIC_INTERVAL         CLOCK_SECOND
/*---------------------------------------------------------------------------*/
#define POST_STATUS_BAD           0x80
#define POST_STATUS_HAS_MODE      0x40
#define POST_STATUS_HAS_DURATION  0x20
#define POST_STATUS_HAS_INTERVAL  0x10
#define POST_STATUS_NONE          0x00
/*---------------------------------------------------------------------------*/
typedef struct sleepy_config_s {
  unsigned long interval;
  unsigned long duration;
  uint8_t mode;
} sleepy_config_t;

sleepy_config_t config;
/*---------------------------------------------------------------------------*/
#define STATE_NORMAL           0
#define STATE_NOTIFY_OBSERVERS 1
#define STATE_VERY_SLEEPY      2
/*---------------------------------------------------------------------------*/
static struct stimer st_duration;
static struct stimer st_interval;
static struct stimer st_min_mac_on_duration;
static struct etimer et_periodic;
static process_event_t event_new_config;
static uint8_t state;
/*---------------------------------------------------------------------------*/
const char *not_supported_msg = "Supported:text/plain,application/json";
/*---------------------------------------------------------------------------*/
PROCESS(very_sleepy_demo_process, "CC13xx/CC26xx very sleepy process");
AUTOSTART_PROCESSES(&very_sleepy_demo_process);
/*---------------------------------------------------------------------------*/
static void
readings_get_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  int temp;
  int voltage;

  if(request != NULL) {
    REST.get_header_accept(request, &accept);
  }

  temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);

  voltage = batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT);

  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
             "{\"temp\":{\"v\":%d,\"u\":\"C\"},"
             "\"voltage\":{\"v\":%d,\"u\":\"mV\"}}",
             temp, (voltage * 125) >> 5);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Temp=%dC, Voltage=%dmV",
             temp, (voltage * 125) >> 5);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, not_supported_msg,
                              strlen(not_supported_msg));
  }
}
/*---------------------------------------------------------------------------*/
RESOURCE(readings_resource, "title=\"Sensor Readings\";obs",
         readings_get_handler, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
static void
conf_get_handler(void *request, void *response, uint8_t *buffer,
                 uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;

  if(request != NULL) {
    REST.get_header_accept(request, &accept);
  }

  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
             "{\"config\":{\"mode\":%u,\"duration\":%lu,\"interval\":%lu}}",
             config.mode, config.duration, config.interval);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
             "Mode=%u, Duration=%lusecs, Interval=%lusecs",
             config.mode, config.duration, config.interval);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, not_supported_msg,
                              strlen(not_supported_msg));
  }
}
/*---------------------------------------------------------------------------*/
static void
conf_post_handler(void *request, void *response, uint8_t *buffer,
                  uint16_t preferred_size, int32_t *offset)
{
  const char *ptr = NULL;
  char tmp_buf[16];
  unsigned long interval = 0;
  unsigned long duration = 0;
  uint8_t mode = VERY_SLEEPY_MODE_OFF;
  uint8_t post_status = POST_STATUS_NONE;
  int rv;

  rv = REST.get_post_variable(request, "mode", &ptr);
  if(rv && rv < 16) {
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, ptr, rv);
    rv = atoi(tmp_buf);

    if(rv == 1) {
      mode = VERY_SLEEPY_MODE_ON;
      post_status |= POST_STATUS_HAS_MODE;
    } else if(rv == 0) {
      mode = VERY_SLEEPY_MODE_OFF;
      post_status |= POST_STATUS_HAS_MODE;
    } else {
      post_status = POST_STATUS_BAD;
    }
  }

  rv = REST.get_post_variable(request, "duration", &ptr);
  if(rv && rv < 16) {
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, ptr, rv);
    rv = atoi(tmp_buf);

    duration = (unsigned long)rv;
    if(duration < NORMAL_OP_DURATION_MIN || duration > NORMAL_OP_DURATION_MAX) {
      post_status = POST_STATUS_BAD;
    } else {
      post_status |= POST_STATUS_HAS_DURATION;
    }
  }

  rv = REST.get_post_variable(request, "interval", &ptr);
  if(rv && rv < 16) {
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, ptr, rv);
    rv = atoi(tmp_buf);
    interval = (unsigned long)rv;
    if(interval < PERIODIC_INTERVAL_MIN || interval > PERIODIC_INTERVAL_MAX) {
      post_status = POST_STATUS_BAD;
    } else {
      post_status |= POST_STATUS_HAS_INTERVAL;
    }
  }

  if((post_status & POST_STATUS_BAD) == POST_STATUS_BAD ||
     post_status == POST_STATUS_NONE) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
             "mode=0|1&duration=[%u,%u]&interval=[%u,%u]",
             NORMAL_OP_DURATION_MIN, NORMAL_OP_DURATION_MAX,
             PERIODIC_INTERVAL_MIN, PERIODIC_INTERVAL_MAX);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
    return;
  }

  /* Values are sane. Update the config and notify the process */
  if(post_status & POST_STATUS_HAS_MODE) {
    config.mode = mode;
  }

  if(post_status & POST_STATUS_HAS_INTERVAL) {
    config.interval = interval;
  }

  if(post_status & POST_STATUS_HAS_DURATION) {
    config.duration = duration;
  }

  process_post(&very_sleepy_demo_process, event_new_config, NULL);
}
/*---------------------------------------------------------------------------*/
RESOURCE(very_sleepy_conf,
         "title=\"Very sleepy conf: "
         "GET|POST mode=0|1&interval=<secs>&duration=<secs>\";rt=\"Control\"",
         conf_get_handler, conf_post_handler, NULL, NULL);
/*---------------------------------------------------------------------------*/
/*
 * If our preferred parent is not NBR_REACHABLE in the ND cache, NUD will send
 * a unicast NS and wait for NA. If NA fails then the neighbour will be removed
 * from the ND cache and the default route will be deleted. To prevent this,
 * keep the MAC on until the parent becomes NBR_REACHABLE. We also keep the MAC
 * on if we are about to do RPL probing.
 *
 * In all cases, the radio will be locked on for KEEP_MAC_ON_MIN_PERIOD secs
 */
static uint8_t
keep_mac_on(void)
{
  uip_ds6_nbr_t *nbr;
  uint8_t rv = MAC_CAN_BE_TURNED_OFF;

  if(!stimer_expired(&st_min_mac_on_duration)) {
    return MAC_MUST_STAY_ON;
  }

#if RPL_WITH_PROBING
  /* Determine if we are about to send a RPL probe */
  if(CLOCK_LT(etimer_expiration_time(
                &rpl_get_default_instance()->probing_timer.etimer),
              (clock_time() + PERIODIC_INTERVAL))) {
    rv = MAC_MUST_STAY_ON;
  }
#endif

  /* It's OK to pass a NULL pointer, the callee checks and returns NULL */
  nbr = uip_ds6_nbr_lookup(uip_ds6_defrt_choose());

  if(nbr == NULL) {
    /* We don't have a default route, or it's not reachable (NUD likely). */
    rv = MAC_MUST_STAY_ON;
  } else {
    if(nbr->state != NBR_REACHABLE) {
      rv = MAC_MUST_STAY_ON;
    }
  }

  if(rv == MAC_MUST_STAY_ON && stimer_expired(&st_min_mac_on_duration)) {
    stimer_set(&st_min_mac_on_duration, KEEP_MAC_ON_MIN_PERIOD);
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static void
switch_to_normal(void)
{
  state = STATE_NOTIFY_OBSERVERS;

  /*
   * Stay in normal mode for 'duration' secs.
   * Transition back to normal in 'interval' secs, _including_ 'duration'
   */
  stimer_set(&st_duration, config.duration);
  stimer_set(&st_interval, config.interval);
}
/*---------------------------------------------------------------------------*/
static void
switch_to_very_sleepy(void)
{
  state = STATE_VERY_SLEEPY;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(very_sleepy_demo_process, ev, data)
{
  uint8_t mac_keep_on;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(batmon_sensor);

  config.mode = VERY_SLEEPY_MODE_OFF;
  config.interval = PERIODIC_INTERVAL_DEFAULT;
  config.duration = NORMAL_OP_DURATION_DEFAULT;

  state = STATE_NORMAL;

  event_new_config = process_alloc_event();

  rest_init_engine();

  readings_resource.flags += IS_OBSERVABLE;
  rest_activate_resource(&readings_resource, "sen/readings");
  rest_activate_resource(&very_sleepy_conf, "very_sleepy_config");

  printf("Very Sleepy Demo Process\n");

  switch_to_normal();

  etimer_set(&et_periodic, PERIODIC_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == sensors_event && data == &button_left_sensor) {
      switch_to_normal();
    }

    if(ev == event_new_config) {
      stimer_set(&st_interval, config.interval);
      stimer_set(&st_duration, config.duration);
    }

    if((ev == PROCESS_EVENT_TIMER && data == &et_periodic) ||
       (ev == sensors_event && data == &button_left_sensor) ||
       (ev == event_new_config)) {

      /*
       * Determine if the stack is about to do essential network maintenance
       * and, if so, keep the MAC layer on
       */
      mac_keep_on = keep_mac_on();

      if(mac_keep_on == MAC_MUST_STAY_ON || state != STATE_VERY_SLEEPY) {
        leds_on(LEDS_GREEN);
        NETSTACK_MAC.on();
      }

      /*
       * Next, switch between normal and very sleepy mode depending on config,
       * send notifications to observers as required.
       */
      if(state == STATE_NOTIFY_OBSERVERS) {
        REST.notify_subscribers(&readings_resource);
        state = STATE_NORMAL;
      }

      if(state == STATE_NORMAL) {
        if(stimer_expired(&st_duration)) {
          stimer_set(&st_duration, config.duration);
          if(config.mode == VERY_SLEEPY_MODE_ON) {
            switch_to_very_sleepy();
          }
        }
      } else if(state == STATE_VERY_SLEEPY) {
        if(stimer_expired(&st_interval)) {
          switch_to_normal();
        }
      }

      if(mac_keep_on == MAC_CAN_BE_TURNED_OFF && state == STATE_VERY_SLEEPY) {
        leds_off(LEDS_GREEN);
        NETSTACK_MAC.off(0);
      } else {
        leds_on(LEDS_GREEN);
        NETSTACK_MAC.on();
      }

      /* Schedule next pass */
      etimer_set(&et_periodic, PERIODIC_INTERVAL);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
