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
*			Button sensor.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/

#include "dev/button-sensor.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#include BOARD_HEADER

#define DEBOUNCE 1

/**
 * \brief Port and pin for BUTTON0.
 */

/*
#undef  BUTTON_S1
#define BUTTON_S1             PORTA_PIN(7)
#define BUTTON_S1_INPUT_GPIO  BUTTON_INPUT_GPIO(PORTA)
#define BUTTON_S1_GPIO_PIN    7
#define BUTTON_S1_OUTPUT_GPIO GPIO_PAOUT
*/

#undef  BUTTON_S1
#define BUTTON_S1             PORTx_PIN(boardDescription->io->buttons[0].gpioPort, boardDescription->io->buttons[0].gpioPin)
#define BUTTON_S1_INPUT_GPIO  BUTTON_INPUT_GPIO(boardDescription->io->buttons[0].gpioPort)
#define BUTTON_S1_GPIO_PIN    boardDescription->io->buttons[0].gpioPin
#define BUTTON_S1_OUTPUT_GPIO GPIO_PAOUT

/**
 * \brief Point the proper IRQ at the desired pin for BUTTON0.
 */
#define BUTTON_S1_SEL()       do { GPIO_IRQCSEL = BUTTON_S1; } while(0)
/**
 * \brief The interrupt service routine for BUTTON_S1.
 */
#define BUTTON_S1_ISR         halIrqCIsr
/**
 * \brief The interrupt configuration register for BUTTON_S1.
 */
#define BUTTON_S1_INTCFG      GPIO_INTCFGC
/**
 * \brief The interrupt bit for BUTTON_S1.
 */
#define BUTTON_S1_INT_EN_BIT  INT_IRQC
/**
 * \brief The interrupt bit for BUTTON_S1.
 */
#define BUTTON_S1_FLAG_BIT    INT_IRQCFLAG
/**
 * \brief The missed interrupt bit for BUTTON_S1.
 */
#define BUTTON_S1_MISS_BIT    INT_MISSIRQC

#if DEBOUNCE
static struct timer debouncetimer;
#endif

#define FALSE 0
#define TRUE  1

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  #if DEBOUNCE
  timer_set(&debouncetimer, 0);
  #endif
  
  /* Configure GPIO for BUTTONSs */
  
  //Input, pulled up or down (selected by GPIO_PxOUT: 0 = pull-down, 1 = pull-up).  
  halGpioConfig(BUTTON_S1,GPIOCFG_IN_PUD);
  BUTTON_S1_OUTPUT_GPIO |= GPIOOUT_PULLUP << BUTTON_S1_GPIO_PIN;
  
  
  BUTTON_S1_SEL();
  BUTTON_S1_INTCFG = 0x40;  // Falling edge triggered.  
  
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  INT_CFGSET = BUTTON_S1_INT_EN_BIT;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  INT_CFGCLR = BUTTON_S1_INT_EN_BIT;
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return (INT_CFGSET & BUTTON_S1_INT_EN_BIT) ? TRUE : FALSE ;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
#if DEBOUNCE
  return (BUTTON_S1_INPUT_GPIO & (1<<BUTTON_S1_GPIO_PIN)) || !timer_expired(&debouncetimer);
#else
  return BUTTON_S1_INPUT_GPIO & (1<<BUTTON_S1_GPIO_PIN);
#endif
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
      if(value)        
        activate();
      else
        deactivate();
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
      return active();
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void BUTTON_S1_ISR(void)
{
  
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  
  //sensors_handle_irq(IRQ_BUTTON);
  
   if(INT_GPIOFLAG & BUTTON_S1_FLAG_BIT) {
    
#if DEBOUNCE
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 5);
      sensors_changed(&button_sensor);
    }
#else
    sensors_changed(&button_sensor);
#endif
    
  }
  
  INT_GPIOFLAG = BUTTON_S1_FLAG_BIT;  
  
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);  
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);

/** @} */
