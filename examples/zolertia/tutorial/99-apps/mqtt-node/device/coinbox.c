/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/mp3-wtv020sd.h"
#include "dev/led-strip.h"
#include "coinbox.h"
#include "dev/sys-ctrl.h"
#include "mqtt-res.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_APP
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
command_values_t coinbox_commands;
/*---------------------------------------------------------------------------*/
PROCESS(coinbox_blinking_process, "coinbox blinkg process");
PROCESS(coinbox_sensors_process, "coinbox actuator process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coinbox_blinking_process, ev, data)
{
  static uint8_t counter;

  PROCESS_BEGIN();

  counter = 0;
  mp3_wtv020sd_gpio_play();
  etimer_set(&et, CLOCK_SECOND / 4);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    counter % 2 ? led_strip_switch(LED_STRIP_ON) : led_strip_switch(LED_STRIP_OFF);
    if(counter > 28) {
      mp3_wtv020sd_gpio_stop();
      mp3_wtv020sd_gpio_next();
      led_strip_switch(LED_STRIP_OFF);
     break;
    }
    counter++;
    etimer_reset(&et);
  }

  /* FIXME: restart the device as currently is not properly stopping the mp3 */
  sys_ctrl_reset();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
activate_coinbox(int arg)
{
  process_start(&coinbox_blinking_process, NULL);
  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coinbox_sensors_process, ev, data)
{
  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load commands default */
  coinbox_commands.num = 1;
  memcpy(coinbox_commands.command[COINBOX_COMMAND].command_name,
         DEFAULT_COMMAND_EVENT_COINBOX, strlen(DEFAULT_COMMAND_EVENT_COINBOX));
  coinbox_commands.command[COINBOX_COMMAND].cmd = activate_coinbox;

  /* Sanity check */
  if(coinbox_commands.num != DEFAULT_COMMANDS_NUM) {
    printf("Coinbox commands: error! number of commands mismatch!\n");
    PROCESS_EXIT();
  }

  led_strip_config();
  mp3_wtv020sd_config(MP3_WTV020SD_GPIO_MODE);
  
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(0);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
