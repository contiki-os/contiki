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
 * $Id: acc-test.c,v 1.1 2008/03/27 13:08:35 nvt-se Exp $
 */

#include "contiki.h"
#include "net/rime.h"
#include "dev/sd.h"

#include <io.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(test_acc_process, "Accelerometer test");
AUTOSTART_PROCESSES(&test_acc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_acc_process, ev, data)
{
  static struct etimer et;
  static int x, y, z;
  static uint32_t xa, ya, za;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&et, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT();
    if (etimer_expired(&et)) {
      adc_on();
      x = ADC12MEM0;
      y = ADC12MEM1;
      z = ADC12MEM2;
      xa = xa + x - (xa >> 4);
      ya = ya + y - (ya >> 4);
      za = za + z - (za >> 4);
      adc_off();
      printf("x=%ld, y=%ld, z=%ld\n",
	(xa >> 4) - x, (ya >> 4) - y, (za >> 4) - z);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
