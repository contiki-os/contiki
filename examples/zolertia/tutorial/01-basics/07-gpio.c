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
 * \file
 *         An example showing how to use the generic I/O pins
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sys/etimer.h"
#if CONTIKI_TARGET_ZOUL
#include "dev/gpio.h"
#endif
/*---------------------------------------------------------------------------*/
PROCESS(test_gpio_process, "Test GPIO");
AUTOSTART_PROCESSES(&test_gpio_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_gpio_process, ev, data)
{
  PROCESS_BEGIN();

  /* Pin operations are done in a mask fashion, so if you want to drive Pin 1
   * then this corresponds to (1<<N), with an actual value of 2.  If the Pin was
   * the number 2 then the mask value would be 4
   */

#if CONTIKI_TARGET_ZOUL
  /* The masks below converts the Port number and Pin number to base and mask values */
  #define EXAMPLE_PORT_BASE  GPIO_PORT_TO_BASE(GPIO_A_NUM)
  #define EXAMPLE_PIN_MASK   GPIO_PIN_MASK(5)

  /* We tell the system the application will drive the pin */
  GPIO_SOFTWARE_CONTROL(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK);

  /* And set as output, starting low */
  GPIO_SET_OUTPUT(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK);
  GPIO_SET_PIN(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK);

#else /* Assume Z1 mote */
  #define EXAMPLE_PIN_MASK  (1<<2)

  /* The MSP430 MCU names the pins as a tupple of PORT + PIN, thus P6.1 refers
   * to the Pin number 1 of the Port 6.
   * We need first to select the operation mode of the Pin, as a Pin can have
   * a special function (UART, ADC, I2C) also.  The GPIO operation is selected
   * by writting the pin's bit as zero to the PxSEL register.  The below snippet
   * changes 00010000 to 11101111, so when writting to the register we only
   * clear our pin, leaving the others untouched (as it is an AND operation)
   */
  P4SEL &= ~EXAMPLE_PIN_MASK;

  /* Next we set the direction of the pin.  Output means the pin can be high
   * (3.3V) or low (0V), while being as Input will allow us to read the digital
   * value the pin has.  To enable the pin as Output, write an 1.
   */
  P4DIR |= EXAMPLE_PIN_MASK;

#endif

  /* Spin the timer */

  etimer_set(&et, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

#if CONTIKI_TARGET_ZOUL
    if(GPIO_READ_PIN(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK)) {
      GPIO_CLR_PIN(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK);
    } else {
      GPIO_SET_PIN(EXAMPLE_PORT_BASE, EXAMPLE_PIN_MASK);
    }
#else
    /* This toggles the pin, if low then sets the pin high, and viceversa.
     * Alternatively to set the pin high, use P4OUT |= EXAMPLE_PIN_MASK, and to
     * drive low use P4OUT &= ~EXAMPLE_PIN_MASK
     */
    P4OUT ^= EXAMPLE_PIN_MASK;
#endif

    leds_toggle(LEDS_GREEN);
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
