/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * Author  : Adam Dunkels
 */

#include "contiki.h"
#include "duty-cycle-scroller.h"
#include "lcd.h"
#include "sys/energest.h"

PROCESS(duty_cycle_scroller_process, "Duty cycle scroller");

static clock_time_t interval;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(duty_cycle_scroller_process, ev, data)
{
  static struct etimer interval_timer;
  int height;
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  uint32_t cpu, lpm, transmit, listen;
  uint32_t all_cpu, all_lpm, all_transmit, all_listen;
  uint32_t radio, time;
  PROCESS_BEGIN();

  while(1) {
    etimer_set(&interval_timer, interval);
    PROCESS_WAIT_UNTIL(etimer_expired(&interval_timer));
    lcd_scroll_x();

    all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
    all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
    all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

    cpu = all_cpu - last_cpu;
    lpm = all_lpm - last_lpm;
    transmit = all_transmit - last_transmit;
    listen = all_listen - last_listen;

    last_cpu = all_cpu;
    last_lpm = all_lpm;
    last_transmit = all_transmit;
    last_listen = all_listen;

    radio = transmit + listen;
    time = cpu + lpm;

    height = 1 + (5 + ((1000 * radio) / time)) / 10;
    if(height >= LCD_MAX_SCROLL_AREA) {
      height = LCD_MAX_SCROLL_AREA;
    }
    lcd_draw_vertical_line(height);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
duty_cycle_scroller_start(clock_time_t i)
{
  interval = i;
  process_start(&duty_cycle_scroller_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
duty_cycle_scroller_stop(void)
{
  process_exit(&duty_cycle_scroller_process);
}
/*---------------------------------------------------------------------------*/
