/*
 * leds.c
 *
 *  Created on: Nov 18, 2012
 *      Author: cazulu
 */

#include "leds-arch.h"
#include "contiki-conf.h"
#include "dev/leds.h"
#include "lpc17xx_gpio.h"

void leds_arch_init(){
  GPIO_SetDir(PIN_LED2, LED2, 1);
  GPIO_SetDir(PIN_LED3, LED3, 1);
}

//For now, let's toggle and forget about the rest
void leds_arch_set(unsigned char leds){
  toggleLeds();
}

unsigned char leds_arch_get(){
  return LEDS_ALL;
}

void toggleLeds(){
  uint32_t ledValue = GPIO_ReadValue(PIN_LED2)&LED2;

  if(ledValue)
    GPIO_ClearValue(PIN_LED2, LED2);
  else
    GPIO_SetValue(PIN_LED2, LED2);

  ledValue = GPIO_ReadValue(PIN_LED3)&LED3;

  if(ledValue)
    GPIO_ClearValue(PIN_LED3, LED3);
  else
    GPIO_SetValue(PIN_LED3, LED3);
}


