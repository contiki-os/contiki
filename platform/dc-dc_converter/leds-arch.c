/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 * This file is part of the Contiki operating system.
 *
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
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


