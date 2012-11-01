/*
 * Copyright (c) 2012, George Oikonomou (oikonomou@users.sourceforge.net)
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
 *
 * This file is part of the Contiki operating system.
 */
/**
 * \file
 *         A simple demo project which demonstrates the cc2531 USB dongle
 *         USB (CDC_ACM) functionality.
 *
 *         It will print out periodically. Anything you type in the dongle's
 *         serial console will be echoed back
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "dev/leds.h"
#include "debug.h"
/*---------------------------------------------------------------------------*/
static struct etimer et;
static uint16_t count;
/*---------------------------------------------------------------------------*/
PROCESS(cc2531_usb_demo_process, "cc2531 USB Demo process");
AUTOSTART_PROCESSES(&cc2531_usb_demo_process);
extern process_event_t serial_line_event_message;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2531_usb_demo_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND);
  count = 0;

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      putstring("cc2531 USB Dongle Out: 0x");
      puthex(count >> 8);
      puthex(count & 0xFF);
      putchar('\n');
      count++;

      etimer_reset(&et);
    } else if(ev == serial_line_event_message) {
      putstring("cc2531 USB Dongle In : ");
      putstring((char *)data);
      putchar('\n');
    }
  }

  PROCESS_END();
}
