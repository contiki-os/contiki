/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         Minimalistic channel energy detection.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "cc2430_sfr.h"

#include "debug.h"
#include "dev/cc2430_rf.h"
#include <stdio.h>

static uint8_t channel;
static int8_t j;
static int8_t cmax;
static int8_t rssi;
static struct etimer et;
static rtimer_clock_t t0;

#define RSSI_BASE    -50
#define RSSI_SAMPLES  30
#define SAMPLE_INTERVAL (CLOCK_SECOND)
#define CHANNEL_MIN 11
#define CHANNEL_MAX 26
/* ToDo: Do this in infinite RX. Take more samples */
/*---------------------------------------------------------------------------*/
PROCESS(energy_scan, "Energy Scanner");
AUTOSTART_PROCESSES(&energy_scan);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(energy_scan, ev, data)
{

  PROCESS_BEGIN();

  printf("Energy Scanner\n");
  printf("CCA Threshold: %d\n", (int8_t)RSSIH);
  printf("Channel scan range: [%u , %u]\n", CHANNEL_MIN, CHANNEL_MAX);
  printf("%u samples per channel, interval %u ticks\n",
      RSSI_SAMPLES, SAMPLE_INTERVAL);

  channel = CHANNEL_MIN;
  while(1) {
    cmax = RSSI_BASE;
    cc2430_rf_channel_set(channel);
    clock_delay_usec(200);

    for(j = 0; j < RSSI_SAMPLES; j++) {
      t0 = RTIMER_NOW();
      rssi = RSSIL;
      if(rssi > cmax) {
        cmax = rssi;
      }
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + 25));
    }
    printf("%u [%3d]: ", channel, cmax);
    for(j = RSSI_BASE; j <= cmax; j++) {
      printf("#");
    }
    printf("\n");
    if(channel == CHANNEL_MAX) {
      printf("===============\n");
      channel = CHANNEL_MIN;
    } else {
      channel++;
    }

    etimer_set(&et, SAMPLE_INTERVAL);
    PROCESS_YIELD();

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
