/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Example usage of DS18B20 driver
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "contiki.h"

/* 1-wire protocol driver */
#include "onewire.h"
/* DS18B20 temperature sensor driver */
#include "ds18b20.h"

#include "power-control.h"

#include <stdio.h>              /* For printf() */
#include <stdint.h>             /* int16_t et al. */
/*---------------------------------------------------------------------------*/
PROCESS(ds18b20_poll_process, "DS18B20 poller process");
AUTOSTART_PROCESSES(&ds18b20_poll_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ds18b20_poll_process, ev, data)
{
  static struct etimer et;
  static uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
  static ow_rom_code_t id;

  PROCESS_BEGIN();

  printf("\nDS18B20 test\n");

  printf("VSEC ON\n");
  power_control_vsec_set(1);

  /* initialize the DS18B20 hardware */
  printf("Initialize 1-wire\n");
  ow_init();
  printf("1-wire READ ROM\n");
  id = ow_read_rom();
  printf("Initialize DS18B20\n");
  ds18b20_init();
  printf("DS18B20 init done\n");

  /* Poll at 1Hz */
  etimer_set(&et, CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Reset the etimer to trig again */
    etimer_reset(&et);

    ds18b20_read_scratchpad(id, scratchpad);
    ds18b20_convert_temperature(id);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
