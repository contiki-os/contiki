/*
 * Copyright (c) 2013, elarm Inc.
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
/**
 * \addtogroup cc2538-adc
 * @{
 *
 * \file
 * Implementation of the cc2538 ADC driver
 *
 * \author
 *           Adam Rea <areairs@gmail.com>
 */
#include "contiki.h"
#include "sys/energest.h"
#include "dev/sys-ctrl.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/nvic.h"
#include "reg.h"

#include <stdint.h>
#include <string.h>

static int (* input_handler)(int c, uint8_t uid);
static struct etimer adc_tic_timer;
typedef struct adc_element {
  uint8_t active;
  uint8_t tics_to_fire;
  uint8_t resolution;
  uint8_t counter_reset_val;
  uint8_t waiting_for_service;
  int (*callback_fn)(int, uint8_t);
} adc_element_t;

static adc_element_t adc_pin_status [12];
static uint8_t tic_timer_started, current_service, waiting_on_conversion;
/*---------------------------------------------------------------------------*/
PROCESS(adc_process, "ADC process");
AUTOSTART_PROCESSES(&adc_process);
/*---------------------------------------------------------------------------*/
void
adc_init(void)
{

  /* do any GLOBAL setup stuff here */
  tic_timer_started = 0;
  current_service = 0;
  waiting_on_conversion = 0;
  /* Start the counter */

  /* Enable ADC Interrupts */
  nvic_interrupt_enable(NVIC_INT_ADC);
}
/*---------------------------------------------------------------------------*/
uint8_t
adc_register_pin(unit8_t pin, uint8_t samples_per_second, uint8_t decimation_rate,int (*cb)(int c, uint8_t uid) )
{
  uint8_t diff_pin;
  
  /* check arguments only doing AIN0 - AIN67 (single ended and differential) */
  if ((pin < ADC_AIN0) || (pin > ADC_AIN67))
    return 0;

  /* bound by clock seconds for granularity */
  if(samples_per_second > CLOCK_SECOND)
    return 0;

  if(!((decimation_rate == ADC_7_BIT) || (decimation_rate == ADC_9_BIT)
	|| (decimation_rate == ADC_10_BIT) || (decimation_rate == ADC_12_BIT)))
    return 0;

  /* Set up the pads and functions for ADC */
  if(pin < ADC_AIN01) { /* Single ended */
    /* set input */
    REG(GPIO_A_BASE + GPIO_DIR) &= ~(0x01 << pin);
    
    /* set alt function to none */
    REG(GPIO_A_BASE + GPIO_AFSEL) &= ~(0x01 << pin); 
    
    /* set overrides to !OE, !PUE, !PDE, ANA */
    REG(IOC_PA0_OVER + (0x04 * pin)) = 0x01; 
  
  } else if(pin < ADC_GND) { /* differential setting */
    
    diff_pin = (pin - ADC_AIN01) * 2;
    /* set inputs on the 2 differential pads */
    REG(GPIO_A_BASE + GPIO_DIR) &= ~(0x03 << diff_pin);
    
    /* set alt functions on the 2 pads to none */
    REG(GPIO_A_BASE + GPIO_AFSEL) &= ~(0x03 << diff_pin); 
    
    /* set overrides to !OE, !PUE, !PDE, ANA */
    REG(IOC_PA0_OVER + (0x04 * diff_pin)) = 0x01;
    REG(IOC_PA0_OVER + (0x04 * (temp_diff_pin+1))) = 0x01;
    
  }


  /* Set up the array element fields for the registration */
  adc_pin_status[pin].active = 1;
  adc_pin_status[pin].tics_to_fire = 1; /* fire next pass */
  adc_pin_status[pin].counter_reset_val = CLOCK_SECOND / samples_per_second;
  adc_pin_status[pin].waiting_for_service = 0;
  adc_pin_status[pin].resolution = decimation_rate;
  adc_pin_status[pin].callback_fn = cb;

  /* start the timer to make the process thread entrant */
  if(!tic_timer_running){
    etimer_set(&adc_tic_timer,1); 
    tic_timer_running = 1;
  }
  
  /* added 1 to avoid collision with error condition */
  return pin+1; 

}
/*---------------------------------------------------------------------------*/
static void
adc_service_requests(void)
{
  static uint8_t c;
  static unsigned int adccon3_holder;

  for(c = current_service; c < 12; c++){
    
    /* spin lock to aviod function call in isr -- lesser of 2 evils */
    while(waiting_on_conversion);
    if(adc_pin_status[c].waiting_for_service == 1)
      {
	/* clear the waiting status and capture our current iteration */
	adc_pin_status[c].waiting_for_service = 0;
	current_service = c;

	/* fire a single read event and exit ... isr will recall to finish */
	waiting_on_conversion = 1;
	REG(ADC_ADCCON3) = adc_pin_status[c].resolution + c;
	
      }
  }
  
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(adc_process, ev, data)
{
  static uint8_t c, need_service;
  
  PROCESS_BEGIN();
  
  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      if(data == &adc_tic_timer) {
	need_service = 0;
	for(c = 0; c < 16; c++) {
	  if(adc_pin_status[c].active) {
	    adc_pin_status[c].tics_to_fire--;
	    if(adc_pin_status[c].tics_to_fire == 0){
	      adc_pin_status[c].tics_to_fire = adc_pin_status[c].counter_reset_val;
	      adc_pin_status[c].waiting_for_service = 1;
	      need_service = 1;
	    }
	  }
	}
	if(need_service){
	  current_service = 0;
	  adc_service_requests();
	}
	if(tic_timer_running) {
	  etimer_reset(&adc_tic_timer);
	} 
      
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
adc_unregister_pin(uint8_t uid)
{
  uint8_t c, any_active;
  
  adc_pin_status[uid-1].active = 0;

  for(c = 0; c < 12, c++) {
    if(adc_pin_status[c].active) {
      any_active = 1;
    }
  }

  if(!any_active) {
    tic_timer_running = 0;
  }

  return;
}
/*---------------------------------------------------------------------------*/
void
adc_isr(void)
{
  int resultant;
  uint8_t shift, temp_dec; 

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Get the low and the high (reading high last as it clears the int) */
  resultant = (REG(ADC_ADCL) & 0xFF) + ((REG(ADCH) & 0xFF)<<8);
  
  temp_dec = adc_pin_status[current_service].resolution;

  if(temp_dec == ADC_7_BIT) {
    shift = ADC_7_BIT_RSHIFT;
  } else if (temp_dec == ADC_9_BIT) {
    shift = ADC_9_BIT_RSHIFT;
  } else if (temp_dec == ADC_10_BIT) {
    shift = ADC_10_BIT_RSHIFT;
  } else {
    shift = ADC_12_BIT_RSHIFT;
  }

  resultant = resultant >> shift;
  if(adc_pin_status[current_service].callback_fn != NULL) {
    adc_pin_status[current_service].callback_fn(resultant,current_service+1);
  }
  
  waiting_on_conversion = 0;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/** @} */
