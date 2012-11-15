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
#include "hal.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#include BOARD_HEADER

#define DEBOUNCE 1

#if DEBOUNCE
static struct timer debouncetimer;
#endif

#define FALSE 0
#define TRUE  1

uint8_t button_flags = 0;

#define BUTTON_ACTIVE_FLG 0x01
#define BUTTON_PRESSED_FLG 0x02

#define BUTTON_HAS_BEEN_PRESSED() (button_flags & BUTTON_PRESSED_FLG)
#define BUTTON_HAS_BEEN_RELEASED() (!(button_flags & BUTTON_PRESSED_FLG))
#define BUTTON_SET_PRESSED() (button_flags |= BUTTON_PRESSED_FLG)
#define BUTTON_SET_RELEASED() (button_flags &= ~BUTTON_PRESSED_FLG)

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  #if DEBOUNCE
  timer_set(&debouncetimer, 0);
  #endif
  
  /* Configure GPIO for BUTTONSs */
  halInitButton();
  
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  button_flags |= BUTTON_ACTIVE_FLG;
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  button_flags &= ~BUTTON_ACTIVE_FLG;
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return (button_flags & BUTTON_ACTIVE_FLG)? 1 : 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(!active()){
    return 0;
  }


#if DEBOUNCE
  if(timer_expired(&debouncetimer)) {

    if(halGetButtonStatus(BUTTON_S1) == BUTTON_PRESSED){

      timer_set(&debouncetimer, CLOCK_SECOND / 10);
      if(BUTTON_HAS_BEEN_RELEASED()){ // Button has been previously released.
        sensors_changed(&button_sensor);
      }
      BUTTON_SET_PRESSED();

      return 1;
    }
    else {
      BUTTON_SET_RELEASED();
      return 0;
    }
  }
  else {
    return 0;
  }
#else
  if(halGetButtonStatus(BUTTON_S1) == BUTTON_PRESSED){
    sensors_changed(&button_sensor);
    return 1;
  }
  else {
    return 0;
  }
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
#if 0
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
#endif
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);

