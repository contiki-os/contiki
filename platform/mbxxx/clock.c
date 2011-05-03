/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			Clock for STM32W.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/

/*
 * File customized for mbxxx platform. It uses systick timer to control button
 * status without interrupts, as well as for system clock.
 */

#include PLATFORM_HEADER
#include "hal/error.h"
#include "hal/hal.h"
#include "dev/stm32w_systick.h"

#include "sys/clock.h"
#include "sys/etimer.h"
#include "dev/button-sensor.h"
#include "uart1.h"
#include "dev/leds.h"
#include "dev/stm32w-radio.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

// The value that will be load in the SysTick value register.
#define RELOAD_VALUE 24000-1   // 1 ms with a 24 MHz clock

static volatile clock_time_t count;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

/*---------------------------------------------------------------------------*/
void SysTick_Handler(void)
{

  count++;

  if(button_sensor.status(SENSORS_READY)){
    button_sensor.value(0); // sensors_changed is called inside this function.
  }

  if(etimer_pending()) {
    etimer_request_poll();
  }
  
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  
}

/*---------------------------------------------------------------------------*/

void clock_init(void)
{ 
  
  ATOMIC(
  
  //Counts the number of ticks.  
  count = 0;
  
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_SetReload(RELOAD_VALUE);
  SysTick_ITConfig(ENABLE);
  SysTick_CounterCmd(SysTick_Counter_Enable);
  
  )
}

/*---------------------------------------------------------------------------*/

clock_time_t clock_time(void)
{
  return count;
}

/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of TODO
 */
void clock_delay(unsigned int i)
{
  for (; i > 0; i--) {		/* Needs fixing XXX */
    unsigned j;
    for (j = 50; j > 0; j--)
      asm ("nop");
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 ms.
 *
 */
void clock_wait(int i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/

unsigned long clock_seconds(void)
{
  return current_seconds;
}

void sleep_seconds(int seconds)
{
  int32u quarter_seconds = seconds * 4;
  uint8_t radio_on;


  halPowerDown();
  radio_on = stm32w_radio_is_on();
  stm32w_radio_driver.off();

  halSleepForQsWithOptions(&quarter_seconds, 0);


  ATOMIC(

  halPowerUp();

  // Update OS system ticks.
  current_seconds += seconds - quarter_seconds / 4 ; // Passed seconds
  count += seconds * CLOCK_SECOND - quarter_seconds * CLOCK_SECOND / 4 ;

  if(etimer_pending()) {
    etimer_request_poll();
  }

  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_SetReload(RELOAD_VALUE);
  SysTick_ITConfig(ENABLE);
  SysTick_CounterCmd(SysTick_Counter_Enable);

  )

  stm32w_radio_driver.init();
  if(radio_on){
	  stm32w_radio_driver.on();
  }

  uart1_init(115200);
  leds_init();
  rtimer_init();

  PRINTF("WakeInfo: %04x\r\n", halGetWakeInfo());


}
