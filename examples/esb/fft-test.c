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
 * $Id: fft-test.c,v 1.3 2010/01/14 18:18:51 nifi Exp $
 *
 * -----------------------------------------------------------------
 * fft-test - a test program that use the sound sensor for sampling
 * some data and then performs an FFT on that. Then flashes leds based
 * on energy-level and position of highest frequency band.
 *
 * Author  : Joakim Eriksson
 * Created : 2008-04-04
 * Updated : $Date: 2010/01/14 18:18:51 $
 *           $Revision: 1.3 $
 */

#include "contiki-esb.h"
#include "dev/sound-sensor.h"
#include <stdio.h>
#include "lib/ifft.h"

#define FFT_TEST_SIZE 128

PROCESS(fft_process, "Fft");

AUTOSTART_PROCESSES(&fft_process);

PROCESS_THREAD(fft_process, ev, data)
{
  static struct etimer etimer;
  static int on = 0;
  static int i = 0;
  static int mode = 0;
  static int16_t fftBuf[FFT_TEST_SIZE];
  static int16_t tmpBuf[FFT_TEST_SIZE];
  static int max_value = 0;
  static int max_pos = 0;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  etimer_set(&etimer, CLOCK_SECOND * 4);
  button_sensor.configure(SENSORS_ACTIVE, 1);
  /* start and configure the sound sensor for sampling */
  sound_sensor.configure(SENSORS_ACTIVE, 1);
  sound_sensor_set_buffer(fftBuf, FFT_TEST_SIZE, 1);
  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == sensors_event) {
      if(data == &button_sensor) {
	beep_beep(100);
	mode = !mode;
      } else if(data == &sound_sensor) {
	for(i = 0; i < FFT_TEST_SIZE; i++) {
	  fftBuf[i] = -128 + (fftBuf[i] >> 4);
	  if(!mode) {
	    /* just print the samples if mode = 0 */
	    if(i > 0) {
	      printf(" ");
	    }
	    printf("%d", fftBuf[i]);
	  }
	}
	if (mode) {
	  /* Calculate fft, print result, and find index of
             highest energy level */
	  ifft(fftBuf, tmpBuf, FFT_TEST_SIZE);
	  max_value = 0;
	  max_pos = 0;
	  for (i = 0; i < FFT_TEST_SIZE/2; i++) {
	    if(i > 0) {
	      printf(" ");
	    }
	    printf("%d", fftBuf[i]);

	    if(max_value < fftBuf[i]) {
	      max_value = fftBuf[i];
	      max_pos = i;
	    }
	  }
	  /* flash leds based on energy level and position of max */
	  if(max_value < 400 || max_pos < 2) {
	    leds_off(LEDS_ALL);
	  } else if (max_pos > 1 && max_pos < 15) {
	    leds_on(LEDS_GREEN);
	  } else if(max_pos < 25) {
	    leds_on(LEDS_YELLOW);
	  } else {
	    leds_on(LEDS_RED);
	  }
	}
	printf("\n");
	printf("#Maxval: %d, maxpos: %d\n", max_value, max_pos);
      }
    } else if(ev == PROCESS_EVENT_TIMER) {
      if(data == &etimer) {
	if(on) {
	  sound_sensor_start_sample();
	  leds_off(LEDS_ALL);
	}
	etimer_set(&etimer, CLOCK_SECOND / 2);
	on = !on;
      }
    }
  }

 exit:
  leds_off(LEDS_ALL);
  PROCESS_END();
}
