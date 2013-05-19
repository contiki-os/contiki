/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         A program that collects statistics from a network of Tmote Sky nodes
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/netstack.h"
#include "net/rime.h"
#include "net/rime/collect.h"
#include "net/rime/collect-neighbor.h"
#include "net/rime/timesynch.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"

#include "dev/cc2420.h"
#include <stdio.h>
#include <string.h>
#include "contiki-net.h"

static struct collect_conn tc;

struct sky_collect_msg {
  uint16_t light1;
  uint16_t light2;
  uint16_t temperature;
  uint16_t humidity;
  uint16_t rssi;
  rimeaddr_t best_neighbor;
  uint16_t best_neighbor_etx;
  uint16_t best_neighbor_rtmetric;
  uint32_t energy_lpm;
  uint32_t energy_cpu;
  uint32_t energy_rx;
  uint32_t energy_tx;
  uint32_t energy_rled;

  uint16_t tx, rx;
  uint16_t reliabletx, reliablerx,
    rexmit, acktx, noacktx, ackrx, timedout, badackrx;
  /* Reasons for dropping incoming packets: */
  uint16_t toolong, tooshort, badsynch, badcrc;
  uint16_t contentiondrop, /* Packet dropped due to contention */
    sendingdrop; /* Packet dropped when we were sending a packet */
  uint16_t lltx, llrx;

  rtimer_clock_t timestamp;
};

#define REXMITS 4

/*---------------------------------------------------------------------------*/
PROCESS(test_collect_process, "Test collect process");
PROCESS(depth_blink_process, "Depth indicator");
AUTOSTART_PROCESSES(&test_collect_process, &depth_blink_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(depth_blink_process, ev, data)
{
  static struct etimer et;
  static int count;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    count = collect_depth(&tc);
    if(count == COLLECT_MAX_DEPTH) {
      leds_on(LEDS_BLUE);
    } else {
      leds_off(LEDS_BLUE);
      count /= COLLECT_LINK_ESTIMATE_UNIT;
      while(count > 0) {
	leds_on(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 32);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	leds_off(LEDS_RED);
	etimer_set(&et, CLOCK_SECOND / 8);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	--count;
      }
    }
  }
  
  PROCESS_END();
}
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
static void
recv(const rimeaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  struct sky_collect_msg *msg;
  
  msg = packetbuf_dataptr();
  printf("%u %u %u %u %u %u %u %u %u %u %u %lu %lu %lu %lu %lu ",
	 (originator->u8[1] << 8) + originator->u8[0],
	 seqno, hops,
	 msg->light1, msg->light2, msg->temperature, msg->humidity,
	 msg->rssi,

	 (msg->best_neighbor.u8[0] << 8) + msg->best_neighbor.u8[1],
	  msg->best_neighbor_etx, msg->best_neighbor_rtmetric,
	 msg->energy_lpm, msg->energy_cpu, msg->energy_rx, msg->energy_tx, msg->energy_rled
	 );
  printf("%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u ",
	 msg->tx, msg->rx, msg->reliabletx, msg->reliablerx, msg->rexmit,
	 msg->acktx, msg->noacktx, msg->ackrx, msg->timedout, msg->badackrx,
	 msg->toolong, msg->tooshort, msg->badsynch, msg->badcrc,
	 msg->contentiondrop, msg->sendingdrop, msg->lltx, msg->llrx);
#if TIMESYNCH_CONF_ENABLED
  printf("%u", timesynch_time() - msg->timestamp);
#else
  printf("%u", RTIMER_NOW() - msg->timestamp);
#endif /* TIMESYNCH_CONF_ENABLED */
  printf("\n");

}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_collect_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit;)
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);
  
  collect_open(&tc, 128, COLLECT_ROUTER, &callbacks);
  
  while(1) {
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND * 20);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == sensors_event) {
      if(data == &button_sensor) {
	collect_set_sink(&tc, 1);
      }
    }

    if(etimer_expired(&et)) {
      struct sky_collect_msg *msg;
      struct collect_neighbor *n;
      /*      leds_toggle(LEDS_BLUE);*/

      SENSORS_ACTIVATE(light_sensor);
      SENSORS_ACTIVATE(sht11_sensor);

      packetbuf_clear();
      msg = (struct sky_collect_msg *)packetbuf_dataptr();
      packetbuf_set_datalen(sizeof(struct sky_collect_msg));
      msg->light1 = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
      msg->light2 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
      msg->temperature = sht11_sensor.value(SHT11_SENSOR_TEMP);
      msg->humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
      msg->rssi = do_rssi();

      msg->energy_lpm = energest_type_time(ENERGEST_TYPE_LPM);
      msg->energy_cpu = energest_type_time(ENERGEST_TYPE_CPU);
      msg->energy_rx = energest_type_time(ENERGEST_TYPE_LISTEN);
      msg->energy_tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
      msg->energy_rled = energest_type_time(ENERGEST_TYPE_LED_RED);
      rimeaddr_copy(&msg->best_neighbor, &rimeaddr_null);
      msg->best_neighbor_etx =
	msg->best_neighbor_rtmetric = 0;
      n = collect_neighbor_list_best(&tc.neighbor_list);
      if(n != NULL) {
	rimeaddr_copy(&msg->best_neighbor, &n->addr);
	msg->best_neighbor_etx = collect_neighbor_link_estimate(n);
	msg->best_neighbor_rtmetric = n->rtmetric;
      }

      msg->tx = RIMESTATS_GET(tx);
      msg->rx = RIMESTATS_GET(rx);
      msg->reliabletx = RIMESTATS_GET(reliabletx);
      msg->reliablerx = RIMESTATS_GET(reliablerx);
      msg->rexmit = RIMESTATS_GET(rexmit);
      msg->acktx = RIMESTATS_GET(acktx);
      msg->noacktx = RIMESTATS_GET(noacktx);
      msg->ackrx = RIMESTATS_GET(ackrx);
      msg->timedout = RIMESTATS_GET(timedout);
      msg->badackrx = RIMESTATS_GET(badackrx);
      msg->toolong = RIMESTATS_GET(toolong);
      msg->tooshort = RIMESTATS_GET(tooshort);
      msg->badsynch = RIMESTATS_GET(badsynch);
      msg->badcrc = RIMESTATS_GET(badcrc);
      msg->contentiondrop = RIMESTATS_GET(contentiondrop);
      msg->sendingdrop = RIMESTATS_GET(sendingdrop);
      msg->lltx = RIMESTATS_GET(lltx);
      msg->llrx = RIMESTATS_GET(llrx);
#if TIMESYNCH_CONF_ENABLED
      msg->timestamp = timesynch_time();
#else
      msg->timestamp = RTIMER_NOW();
#endif /* TIMESYNCH_CONF_ENABLED */

      SENSORS_DEACTIVATE(light_sensor);
      SENSORS_DEACTIVATE(sht11_sensor);

      collect_send(&tc, REXMITS);
    }
  }
 exit:
  collect_close(&tc);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
