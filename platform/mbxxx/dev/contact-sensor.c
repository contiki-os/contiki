/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */
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
*			Contact sensor.
* \author
*			Stefano Pascali <stefano.pascali@st.com>
*			Marco Grella    <marco.grella@st.com>
*/
/*---------------------------------------------------------------------------*/

#include PLATFORM_HEADER
#include BOARD_HEADER

#include "hal/error.h"
#include "hal/hal.h"
#include "dev/leds.h"

#include "dev/contact-sensor.h"
void halIrqAIsr(void);

static void
init(void)
{  
 
  //PC0 configuration: push-pull output
  halGpioConfig(PORTC_PIN(0), GPIOCFG_OUT);
  GPIO_PCSET |= PC0;
   
  //PB0 (IRQA)configuration: input Pull Down. 
  //halGpioConfig(PORTB_PIN(0), GPIOCFG_IN_PUD);
  //GPIO_PBOUT &= ~PB0;

  //PB0 (IRQA)configuration: input Pull Down. 
  halGpioConfig(PORTB_PIN(0), GPIOCFG_IN);
    
  //configure IRQA mode: Rising and Falling edge triggered, digital filter enabled
  //  GPIO_INTCFGA =0x00000160;
  
  GPIO_INTCFGA = GPIO_INTCFGA_RESET;
  GPIO_INTCFGA|=GPIO_INTFILT;
  GPIO_INTCFGA|=(0x03 <<GPIO_INTMOD_BIT);     
    
  halIrqAIsr();
  //Enable IRQA interrupt
  INT_CFGSET = INT_IRQA;


 // printf("\r\n[Contact Sensor] Initialization Done\r\n");
  
}/* end switch_init */


/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
void halIrqAIsr(void)
{
  //Clear Interrupt Pending Bit

  INT_GPIOFLAG = INT_IRQAFLAG;
 
  if ((GPIO_PBIN & (1)) == 0){
    leds_on(LEDS_RED);
    leds_off(LEDS_GREEN);
  }else{
    leds_on(LEDS_GREEN);
    leds_off(LEDS_RED);
   // switch_closed=TRUE;
  }
}


/*---------------------------------------------------------------------------*/
static int
value(int type)
{
 //printf("[Contact Sensor] Value=%d\r\n", GPIO_PBIN & (1));
  return (GPIO_PBIN & (1));
  
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type){
    case SENSORS_HW_INIT:
      init();
      return 1;
    case SENSORS_ACTIVE:
      return 1;
  }
       
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
    
    case SENSORS_READY:
      return 1;
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(contact_sensor, CONTACT_SENSOR,
         value, configure, status);



/** @} */
