/*
 * Copyright (c) 2013, Jelmer Tiete.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         A simple program for testing the TLC59116 I2C led driver.
 *         All 16 outputs will sequencially light up.
 * \author
 *         Jelmer Tiete, VUB <jelmer@tiete.be>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/tlc59116.h"
/*---------------------------------------------------------------------------*/
#define BLINK_INTERVAL        (CLOCK_SECOND / 25)
/*---------------------------------------------------------------------------*/
PROCESS(tlc59116_process, "Test tlc59116 process");
AUTOSTART_PROCESSES(&tlc59116_process);
/*---------------------------------------------------------------------------*/
/* Main process, setups  */

static struct etimer et;
static uint8_t count = 0;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tlc59116_process, ev, data)
{
  PROCESS_BEGIN();
  {

    /* Start and setup the led driver with default values, eg outputs on and pwm enabled and 0. */
    tlc59116_init();

    while(1) {

      tlc59116_led(count, 0x00);
      tlc59116_led((count + 1) % 16, 0x20);
      tlc59116_led((count + 2) % 16, 0x40);
      tlc59116_led((count + 3) % 16, 0xFF);

      count++;
      if(count > 15) {
        count = 0;
      }

      etimer_set(&et, BLINK_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
