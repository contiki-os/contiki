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
 * $Id: sky-shell.c,v 1.6 2008/11/10 21:14:20 adamdunkels Exp $
 */

/**
 * \file
 *         Tmote Sky-specific Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"

#include "net/rime/neighbor.h"
#include "dev/watchdog.h"

#include "net/rime.h"
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/light.h"
#include "dev/sht11.h"
#include "dev/battery-sensor.h"

#include "net/rime/timesynch.h"

#include <stdio.h>
#include <string.h>

#include <io.h>
#include <signal.h>

#define DEBUG_SNIFFERS 0

/*---------------------------------------------------------------------------*/
PROCESS(sky_shell_process, "Sky Contiki shell");
AUTOSTART_PROCESSES(&sky_shell_process);
/*---------------------------------------------------------------------------*/
#if DEBUG_SNIFFERS
static void
input_sniffer(void)
{
  int i;
  uint8_t *dataptr;

  printf("x %d ", rimebuf_totlen());
  dataptr = rimebuf_dataptr();
  printf("%02x ", dataptr[0]);
  /*  if(dataptr[0] == 18) {*/
    for(i = 1; i < rimebuf_totlen(); ++i) {
      printf("%02x ", dataptr[i]);
    }
    /*  }*/
  printf("\n");
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(void)
{
  uint8_t *dataptr;
  printf("- %d ", rimebuf_totlen());
  dataptr = rimebuf_dataptr();
  printf("%02x\n", dataptr[0]);
}
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(s, input_sniffer, output_sniffer);
#endif /* DEBUG_SNIFFERS */
/*---------------------------------------------------------------------------*/
PROCESS(shell_sky_alldata_process, "sky-alldata");
SHELL_COMMAND(sky_alldata_command,
	      "sky-alldata",
	      "sky-alldata: sensor data, power consumption, network stats",
	      &shell_sky_alldata_process);
/*---------------------------------------------------------------------------*/
#define MAX(a, b) ((a) > (b)? (a): (b))
#define MIN(a, b) ((a) < (b)? (a): (b))
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
  
  rime_mac->off(0);

  cc2420_on();
  for(channel = 11; channel <= 26; ++channel) {
    cc2420_set_channel(channel);
    rssi_samples[sample].channel[channel - 11] = cc2420_rssi() + 53;
  }
  
  rime_mac->on();
  
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
struct sky_alldata_msg {
  uint16_t len;
  uint16_t clock;
  uint16_t timesynch_time;
  uint16_t light1;
  uint16_t light2;
  uint16_t temp;
  uint16_t humidity;
  uint16_t rssi;
  uint16_t cpu;
  uint16_t lpm;
  uint16_t transmit;
  uint16_t listen;
  uint16_t best_neighbor;
  uint16_t best_neighbor_etx;
  uint16_t best_neighbor_rtmetric;
  uint16_t battery_voltage;
  uint16_t battery_indicator;
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sky_alldata_process, ev, data)
{
  static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
  struct sky_alldata_msg msg;
  struct neighbor *n;
  PROCESS_BEGIN();

  
  msg.len = sizeof(struct sky_alldata_msg) / sizeof(uint16_t);
  msg.clock = clock_time();
  msg.timesynch_time = timesynch_time();
  msg.light1 = sensors_light1();
  msg.light2 = sensors_light2();
  msg.temp = sht11_temp();
  msg.humidity = sht11_humidity();
  msg.rssi = do_rssi();

  energest_flush();
  
  msg.cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
  msg.lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
  msg.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit;
  msg.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen;


  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

  msg.best_neighbor = msg.best_neighbor_etx =
    msg.best_neighbor_rtmetric = 0;
  n = neighbor_best();
  if(n != NULL) {
    msg.best_neighbor = n->addr.u16[0];
    msg.best_neighbor_etx = neighbor_etx(n);
    msg.best_neighbor_rtmetric = n->rtmetric;
  }
  msg.battery_voltage = battery_sensor.value(0);
  msg.battery_indicator = sht11_sreg() & 0x40? 1: 0;
  shell_output(&sky_alldata_command, &msg, sizeof(msg), "", 0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sky_shell_process, ev, data)
{
  PROCESS_BEGIN();

  serial_shell_init();
  shell_blink_init();
  shell_file_init();
  shell_coffee_init();
  shell_ps_init();
  shell_reboot_init();
  shell_rime_init();
  shell_rime_debug_init();
  shell_rime_netcmd_init();
  shell_rime_ping_init();
  shell_rime_sniff_init();
  shell_sky_init();
  shell_text_init();
  shell_time_init();

  shell_register_command(&sky_alldata_command);

#if DEBUG_SNIFFERS
  rime_sniffer_add(&s);
#endif /* DEBUG_SNIFFERS */
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
