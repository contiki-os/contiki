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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-mp3-wtv020sd-test mp3-wtv020sd MP3 player
 *
 * Demonstrates the use of the mp3-wtv020sd mp3 player
 * @{
 *
 * \file
 *  Test file for the external mp3-wtv020sd MP3 player
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include "dev/mp3-wtv020sd.h"
#include "dev/led-strip.h"
/*---------------------------------------------------------------------------*/
PROCESS(remote_mp3wtv020sd_process, "mp3-wtv020sd test process");
AUTOSTART_PROCESSES(&remote_mp3wtv020sd_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_mp3wtv020sd_process, ev, data)
{
  static uint16_t counter;

  PROCESS_BEGIN();

  led_strip_config();
  mp3_wtv020sd_config(MP3_WTV020SD_GPIO_MODE);

  counter = 0;
  mp3_wtv020sd_gpio_play();

  etimer_set(&et, CLOCK_SECOND / 4);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    counter % 2 ? led_strip_switch(LED_STRIP_ON) : led_strip_switch(LED_STRIP_OFF);
    if(counter > 28) {
      mp3_wtv020sd_gpio_stop();
      led_strip_switch(LED_STRIP_OFF);
     break;
    }
    counter++;
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

