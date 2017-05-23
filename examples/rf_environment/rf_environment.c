/*
 * Copyright (c) 2016, Robert Olsson KTH Royal Institute of Technology
 * COS/Kista Stockholm roolss@kth.se
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "sys/process.h"
#include "sys/etimer.h"
#include <dev/watchdog.h>
#include "dev/leds.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define SAMPLES 1000

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(rf_scan_process, "rf_scan process");
AUTOSTART_PROCESSES(&rf_scan_process);

/*
   rf_environment runs clear channel assessment (CCA) test for over 
   all 802.15.4 channels and reports stats per channel. The CCA test 
   is run for different CCA thresholds from -60 to -190 dBm. CCA is a
   non-destructive for the rf-environment it's just listens.

   Best and worst channel is printed as average rf activity.

   Originally developed for the Atmel avr-rss2 platform.

 */

static struct etimer et;
static int cca[16], cca_thresh, chan, i, j, k;
static uint16_t best, best_sum;
static uint16_t worst, worst_sum;
static double ave;

static radio_value_t
get_chan(void)
{
  radio_value_t chan;
  if(NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &chan) ==
     RADIO_RESULT_OK) {
    return chan;
  }
  return 0;
}
static void
set_chan(uint8_t chan)
{
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, chan) ==
     RADIO_RESULT_OK) {
  }
}
static radio_value_t
get_chan_min(void)
{
  radio_value_t chan;
  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &chan) ==
     RADIO_RESULT_OK) {
    return chan;
  }
  return 0;
}
static radio_value_t
get_chan_max(void)
{
  radio_value_t chan;

  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &chan) ==
     RADIO_RESULT_OK) {
    return chan;
  }
  return 0;
}
static radio_value_t
get_cca_thresh(void)
{
  radio_value_t cca;

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_CCA_THRESHOLD, &cca) ==
     RADIO_RESULT_OK) {
    return cca;
  }
  return 0;
}
static radio_value_t
set_cca_thresh(radio_value_t thresh)
{
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CCA_THRESHOLD, thresh) ==
     RADIO_RESULT_OK) {
    return RADIO_RESULT_OK;
  }
  return 0;
}
 
void 
do_all_chan_cca(int *cca, int try)
{
  int j;
  for(j = 0; j < 16; j++) {
    set_chan(j+11);
    cca[j] = 0;
#ifdef CONTIKI_TARGET_AVR_RSS2
    watchdog_periodic();
#endif
    NETSTACK_RADIO.on();
    for(i = 0; i < try; i++) {
      cca[j] += NETSTACK_RADIO.channel_clear();
    }
    NETSTACK_RADIO.off();
  }
}

PROCESS_THREAD(rf_scan_process, ev, data)
{
  PROCESS_BEGIN();

  leds_init();
  leds_on(LEDS_RED);
  leds_on(LEDS_YELLOW);

  printf("Chan min=%d\n", get_chan_min());
  chan = get_chan();
  printf("Chan cur=%d\n", chan);
  printf("Chan max=%d\n", get_chan_max());
  cca_thresh = get_cca_thresh();
  printf("Default CCA thresh=%d\n", cca_thresh);

  etimer_set(&et, CLOCK_SECOND / 2);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    for(k = -90; k <= -60; k += 2) {
      set_cca_thresh(k);

      do_all_chan_cca(cca, SAMPLES);

      printf("cca_thresh=%-3ddBm", get_cca_thresh());

      worst = 0;
      worst_sum = 0xFFFF;
      best = 0;
      best_sum = 0;
      ave = 0;

      for(j = 0; j < 16; j++) {
        ave += cca[j];
        printf(" %3d", 100 - (100 * cca[j]) / SAMPLES);
        if(cca[j] > best_sum) {
          best_sum = cca[j];
          best = j;
        }
        if(cca[j] < worst_sum) {
          worst_sum = cca[j];
          worst = j;
        }
      }
      printf(" Best=%d Worst=%d Ave=%-5.2f\n", (best+11) , (worst+11), 100 - (100 * (ave / 16) / SAMPLES));
    }
    etimer_set(&et, CLOCK_SECOND / 2);
  }
  PROCESS_END();
}
