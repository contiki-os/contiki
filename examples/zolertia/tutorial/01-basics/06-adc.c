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
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         An example showing how to use the ADC ports
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "lib/sensors.h"
#if CONTIKI_TARGET_ZOUL
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#else /* Assumes Z1 mote */
#include "dev/z1-phidgets.h"
#endif
/*---------------------------------------------------------------------------*/
PROCESS(test_adc_process, "Test ADC");
AUTOSTART_PROCESSES(&test_adc_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_adc_process, ev, data)
{
  PROCESS_BEGIN();

#if CONTIKI_TARGET_ZOUL
  /* The ADC zoul library configures the on-board enabled ADC channels, more
   * information is provided in the board.h file of the platform
   */
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

#else /* Assumes Z1 mote */
  /* The phidget library is a wrapper that configures the 4 x ADC exposed in the
   * 3-pin connectors of the Z1 mote.  Two are for 3.3V powered sensors, the
   * remaining two are for 5V ones
   */
  SENSORS_ACTIVATE(phidgets);
#endif

  /* Connect an analogue sensor and measure its value! */

  etimer_set(&et, CLOCK_SECOND);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    leds_toggle(LEDS_GREEN);

#if CONTIKI_TARGET_ZOUL
    printf("ADC1 = %u mV\n", adc_zoul.value(ZOUL_SENSORS_ADC1));
    printf("ADC3 = %u mV\n", adc_zoul.value(ZOUL_SENSORS_ADC3));
#else
    printf("Phidget 5V 1:%d\n", phidgets.value(PHIDGET5V_1));
    printf("Phidget 5V 2:%d\n", phidgets.value(PHIDGET5V_2));
    printf("Phidget 3V 1:%d\n", phidgets.value(PHIDGET3V_1));
    printf("Phidget 3V 2:%d\n\n", phidgets.value(PHIDGET3V_2));
#endif

    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
