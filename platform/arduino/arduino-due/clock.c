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
*			Clock for Atmel SAM3X8E [Cortex-M3]
* \author
*			Ioannis Glaropoulos
*/
/*---------------------------------------------------------------------------*/



#include "clock.h"
#include "sam3x\sysclk.h"
#include "core_cm3.h"
#include "etimer.h"



static volatile clock_time_t count;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;



/*---------------------------------------------------------------------------*/
void SysTick_Handler(void)
{	
	count++;
	
/*	
	if(button_sensor.status(SENSORS_READY)){
		button_sensor.value(0); // sensors_changed is called inside this function.
	}
*/
	if(etimer_pending()) {
		etimer_request_poll();
	}
	
	if (--second_countdown == 0) {
		current_seconds++;
		second_countdown = CLOCK_SECOND;
	}
	
}

/*---------------------------------------------------------------------------*/
/*
 * This function is called by the main, at the initialization phase
 */
void clock_init(void)
{
	// Counts the number of ticks.
	count = 0;
	
	/* Configure sys-tick for 1 ms */
	printf("DEBUG: Configure system tick to get 1ms tick period.\n");
	printf("DEBUG: The output of the sysclk_get_cpu_hz() is %d Hz.\n",(int)sysclk_get_cpu_hz());
	
	if (SysTick_Config(sysclk_get_cpu_hz() / 1000)) {
		puts("-F- Systick configuration error\r");
		while (1);
	}
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
void clock_wait(clock_time_t i)
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
#if 0 // XXX John: Decide what to do with this
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
#endif 