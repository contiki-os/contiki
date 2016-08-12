/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup remote-examples
 * @{
 * \defgroup remote-iaq-test
 * 
 *  RE-Mote external IAQ test application
 *  Example of iAQ-Core implementation and simple operation reading the value
 *  of CO2, TVOC sensor and Status. The test checks for a each 5 minutes in 
 *  order to get the first true measurement as datasheet recomendation
 *  (standard result of 0x82 is obtained in first time). 
 *  Then, once initialized, periodically each 5 seconds reads 
 *  the values of the IAQ sensor and shows in the screen, toggling the LED 
 *  red if CO2 was not initialized and LED green if the reading was succeed.
 *
 * @{
 * \file
 * RE-Mote implementation of external IAQ-CORE-C test application
 * \author
 * Aitor Mejias <amejias@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "iaq.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
/* Standard warm up time iAQ Sensor */
#define IAQ_INIT_WAIT         325L
#define LOOP_PERIOD             5L
#define LOOP_INTERVAL         (CLOCK_SECOND * LOOP_PERIOD)
/*---------------------------------------------------------------------------*/
PROCESS(test_remote_iaq_process, "Test IAQ driver process");
AUTOSTART_PROCESSES(&test_remote_iaq_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_iaq_process, ev, data)
{
  static uint16_t count_delay = 0;
  static uint16_t status = 0;

  PROCESS_BEGIN();

  /* Configure IAQ and return structure with all parameters */
  SENSORS_ACTIVATE(iaq);

  while(1) {
    /* Wait a bit */
    etimer_set(&et, (LOOP_INTERVAL));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    status = iaq.status(IAQ_DRIVER_STATUS);

    if(status == IAQ_INIT_STATE) {
      count_delay += LOOP_PERIOD;
      leds_toggle(LEDS_RED);
      printf("Test-IAQ: Initializing Time-elapsed: %u seconds of aprox. %lu sec.\n",
      	      count_delay, IAQ_INIT_WAIT);
    } else if(status == IAQ_ACTIVE) {
      leds_off(LEDS_RED);
      leds_toggle(LEDS_GREEN);

      /* Get data from sensor: VOC, CO2 and internal status */
      printf("CO2 current value is: %d ppm\n",   iaq.value(IAQ_VOC_VALUE));
      printf("TIAQ current value is: %d ppb\n",  iaq.value(IAQ_CO2_VALUE));
      printf("Status is: 0x%0X\n", iaq.value(IAQ_STATUS));
    }
    else {
      printf("iAQ-Core Error: 0x%02X\n", status);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

