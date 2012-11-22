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
unsigned char isLed2_On;
unsigned char isLed3_On;

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
//LEDS_YELLOW=LED2
//LEDS_ALL=LED1 + LED2
void leds_arch_set(unsigned char leds){
  if(leds&LEDS_GREEN){
    GPIO_SetValue(PIN_LED2, LED2);
    isLed2_On=1;
  }
  else{
    GPIO_ClearValue(PIN_LED2, LED2);
    isLed2_On=1;
  }

  if(leds&LEDS_YELLOW){
    GPIO_SetValue(PIN_LED3, LED3);
    isLed3_On=1;
  }
  else{
    GPIO_ClearValue(PIN_LED3, LED3);
    isLed3_On=0;
  }
}

//Returns the leds that are on right now
unsigned char leds_arch_get(){
  unsigned char ledStatus=0;
  if(isLed2_On)
    ledStatus|=LEDS_GREEN;
  if(isLed3_On)
    ledStatus|=LEDS_YELLOW;

  return ledStatus;
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


