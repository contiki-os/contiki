/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *
 */

/**
 * \file
 *         Tmote Sky-specific Contiki shell commands
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "sys/cc.h"
#include "shell-sky.h"

#include "dev/watchdog.h"

#include "net/rime/rime.h"
#include "net/netstack.h"
#include "cc2420.h"
#include "dev/leds.h"
#include "dev/sht11/sht11.h"
#include "dev/light-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "net/rime/timesynch.h"

#include "sys/node-id.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_nodeid_process, "nodeid");
SHELL_COMMAND(nodeid_command,
	      "nodeid",
	      "nodeid: set node ID",
	      &shell_nodeid_process);
PROCESS(shell_sense_process, "sense");
SHELL_COMMAND(sense_command,
	      "sense",
	      "sense: print out sensor data",
	      &shell_sense_process);
PROCESS(shell_senseconv_process, "senseconv");
SHELL_COMMAND(senseconv_command,
	      "senseconv",
	      "senseconv: convert 'sense' data to human readable format",
	      &shell_senseconv_process);
PROCESS(shell_txpower_process, "txpower");
SHELL_COMMAND(txpower_command,
	      "txpower",
	      "txpower <power>: change CC2420 transmission power (0 - 31)",
	      &shell_txpower_process);
PROCESS(shell_rfchannel_process, "rfchannel");
SHELL_COMMAND(rfchannel_command,
	      "rfchannel",
	      "rfchannel <channel>: change CC2420 radio channel (11 - 26)",
	      &shell_rfchannel_process);
/*---------------------------------------------------------------------------*/
struct spectrum {
  int channel[16];
};
#define NUM_SAMPLES 4
static struct spectrum rssi_samples[NUM_SAMPLES];
static int
do_rssi(void)
{
  static int sample;
  int channel;
  
  NETSTACK_MAC.off(0);

  cc2420_on();
  for(channel = 11; channel <= 26; ++channel) {
    cc2420_set_channel(channel);
    rssi_samples[sample].channel[channel - 11] = cc2420_rssi() + 53;
  }
  
  NETSTACK_MAC.on();
  
  sample = (sample + 1) % NUM_SAMPLES;

  {
    int channel, tot;
    tot = 0;
    for(channel = 0; channel < 16; ++channel) {
      int max = -256;
      int i;
      for(i = 0; i < NUM_SAMPLES; ++i) {
	max = MAX(max, rssi_samples[i].channel[channel]);
      }
      tot += max / 20;
    }
    return tot;
  }
}
/*---------------------------------------------------------------------------*/
struct sense_msg {
  uint16_t len;
  uint16_t clock;
  uint16_t timesynch_time;
  uint16_t light1;
  uint16_t light2;
  uint16_t temp;
  uint16_t humidity;
  uint16_t rssi;
  uint16_t voltage;
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sense_process, ev, data)
{
  struct sense_msg msg;
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  msg.len = 7;
  msg.clock = clock_time();
#if TIMESYNCH_CONF_ENABLED
  msg.timesynch_time = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
  msg.timesynch_time = 0;
#endif /* TIMESYNCH_CONF_ENABLED */
  msg.light1 = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  msg.light2 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
  msg.temp = sht11_sensor.value(SHT11_SENSOR_TEMP);
  msg.humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  msg.rssi = do_rssi();
  msg.voltage = battery_sensor.value(0);

  msg.rssi = do_rssi();

  SENSORS_DEACTIVATE(light_sensor);
  SENSORS_DEACTIVATE(battery_sensor);
  SENSORS_DEACTIVATE(sht11_sensor);

  shell_output(&sense_command, &msg, sizeof(msg), "", 0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_senseconv_process, ev, data)
{
  struct shell_input *input;
  struct sense_msg *msg;
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
    msg = (struct sense_msg *)input->data1;

    if(msg != NULL) {
      char buf[40];
      snprintf(buf, sizeof(buf),
	       "%d", 10 * msg->light1 / 7);
      shell_output_str(&senseconv_command, "Light 1 ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", 46 * msg->light2 / 10);
      shell_output_str(&senseconv_command, "Light 2 ", buf);
      snprintf(buf, sizeof(buf),
	       "%d.%d", (msg->temp / 10 - 396) / 10,
	       (msg->temp / 10 - 396) % 10);
      shell_output_str(&senseconv_command, "Temperature ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", (int)(-4L + 405L * msg->humidity / 10000L));
      shell_output_str(&senseconv_command, "Relative humidity ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", msg->rssi);
      shell_output_str(&senseconv_command, "RSSI ", buf);
      snprintf(buf, sizeof(buf), /* 819 = 4096 / 5 */
	       "%d.%d", (msg->voltage / 819), (10 * msg->voltage / 819) % 10);
      shell_output_str(&senseconv_command, "Voltage ", buf);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_txpower_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t txpower;
  } msg;
  const char *newptr;
  PROCESS_BEGIN();

  msg.txpower = shell_strtolong(data, &newptr);
  
  /* If no transmission power was given on the command line, we print
     out the current txpower. */
  
  if(newptr == data) {
    msg.txpower = cc2420_get_txpower();
  } else {
    cc2420_set_txpower(msg.txpower);
  }

  msg.len = 1;

  shell_output(&txpower_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rfchannel_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t channel;
  } msg;
  const char *newptr;
  PROCESS_BEGIN();

  msg.channel = shell_strtolong(data, &newptr);
  
  /* If no channel was given on the command line, we print out the
     current channel. */
  if(newptr == data) {
    msg.channel = cc2420_get_channel();
  } else {
    cc2420_set_channel(msg.channel);
  }

  msg.len = 1;

  shell_output(&rfchannel_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_nodeid_process, ev, data)
{
  uint16_t nodeid;
  char buf[20];
  const char *newptr;
  PROCESS_BEGIN();

  nodeid = shell_strtolong(data, &newptr);
  
  /* If no node ID was given on the command line, we print out the
     current channel. Else we burn the new node ID. */
  if(newptr == data) {
    nodeid = node_id;
  } else {
    nodeid = shell_strtolong(data, &newptr);
    watchdog_stop();
    leds_on(LEDS_RED);
    node_id_burn(nodeid);
    leds_on(LEDS_BLUE);
    node_id_restore();
    leds_off(LEDS_RED + LEDS_BLUE);
    watchdog_start();
  }

  snprintf(buf, sizeof(buf), "%d", nodeid);
  shell_output_str(&nodeid_command, "Node ID: ", buf);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_sky_init(void)
{
  shell_register_command(&txpower_command);
  shell_register_command(&rfchannel_command);
  shell_register_command(&sense_command);
  shell_register_command(&senseconv_command);
  shell_register_command(&nodeid_command);

}
/*---------------------------------------------------------------------------*/
