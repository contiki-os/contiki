/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
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
 *         Websense for Z1 mote
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 */

#include "contiki.h"
#include "dev/leds.h"
#include "dev/tmp102.h"
#include "dev/z1-phidgets.h"
#include "lib/sensors.h"
#include "jsontree.h"
#include "json-ws.h"

/* It seems like there normally is an offset of some degrees for the tmp102 */
#define TMP102_OFFSET -150

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(websense_process, "Websense (z1)");
AUTOSTART_PROCESSES(&websense_process);

static int
output_sensor(struct jsontree_context *path)
{
  char buf[10];
  int tmp;
  int tmp2;

  tmp = tmp102_read_temp_x100();
  tmp2 = tmp - TMP102_OFFSET;
  tmp = tmp2 / 100;

  snprintf(buf, sizeof(buf), "%2d.%02d", tmp, tmp2 - (100 * tmp));
  jsontree_write_atom(path, buf);
  return 0;
}
static struct jsontree_callback sensor_callback =
  JSONTREE_CALLBACK(output_sensor, NULL);

/*---------------------------------------------------------------------------*/

static struct jsontree_string desc = JSONTREE_STRING("Zolertia Z1");
static struct jsontree_string unit = JSONTREE_STRING("Celcius");

JSONTREE_OBJECT(node_tree,
                JSONTREE_PAIR("node-type", &desc),
                JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(sensor_tree,
                JSONTREE_PAIR("unit", &unit),
                JSONTREE_PAIR("value", &sensor_callback));

JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("temperature", &sensor_tree),
                JSONTREE_PAIR("leds", &json_leds_callback));

/* complete node tree */
JSONTREE_OBJECT(tree,
                JSONTREE_PAIR("node", &node_tree),
                JSONTREE_PAIR("rsc", &rsc_tree),
                JSONTREE_PAIR("cfg", &json_subscribe_callback));

/*---------------------------------------------------------------------------*/
/* for cosm plugin */
#if WITH_COSM
/* set COSM value callback to be the temp sensor */
struct jsontree_callback cosm_value_callback =
  JSONTREE_CALLBACK(output_sensor, NULL);
#endif

PROCESS_THREAD(websense_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  json_ws_init(&tree);

  tmp102_init();

  json_ws_set_callback("rsc");

  while(1) {
    /* Alive indication with the LED */
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_on(LEDS_RED);
    etimer_set(&timer, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_off(LEDS_RED);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
