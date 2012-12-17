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

//Variables to store the status of the LEDs
static unsigned char led2_state;
static unsigned char led3_state;

void leds_arch_init(){
  //Set the LED pins as output
  GPIO_SetDir(PIN_LED2, LED2, 1);
  GPIO_SetDir(PIN_LED3, LED3, 1);
  //Turn off the leds
  GPIO_ClearValue(PIN_LED2, LED2);
  GPIO_ClearValue(PIN_LED3, LED3);
}

//Set the LEDs indicated by the variable leds
//The mapping between Contiki leds and the board is the f
//LEDS_GREEN=LED1
//LEDS_RED=LED2
//LEDS_ALL=LED1 + LED2
void leds_arch_set(unsigned char leds){
  if(leds&LEDS_GREEN){
    GPIO_SetValue(PIN_LED2, LED2);
    led2_state=1;
  }
  else{
    GPIO_ClearValue(PIN_LED2, LED2);
    led2_state=1;
  }

  if(leds&LEDS_RED){
    GPIO_SetValue(PIN_LED3, LED3);
    led3_state=1;
  }
  else{
    GPIO_ClearValue(PIN_LED3, LED3);
    led3_state=0;
  }
}

//Returns the leds that are on right now
unsigned char leds_arch_get(){
  unsigned char led_state=0;
  if(led2_state)
    led_state|=LEDS_GREEN;
  if(led3_state)
    led_state|=LEDS_RED;

  return led_state;
}

void toggleLeds(){
  uint32_t led_value = GPIO_ReadValue(PIN_LED2)&LED2;

  if(led_value)
    GPIO_ClearValue(PIN_LED2, LED2);
  else
    GPIO_SetValue(PIN_LED2, LED2);

  led_value = GPIO_ReadValue(PIN_LED3)&LED3;

  if(led_value)
    GPIO_ClearValue(PIN_LED3, LED3);
  else
    GPIO_SetValue(PIN_LED3, LED3);
}


