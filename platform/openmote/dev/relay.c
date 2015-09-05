/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 *
 * -----------------------------------------------------------------
 *
 * \file
 *         Device simple driver for generic relay in phidget port of Zolertia Z1
 * \author
 *         Antonio Lignan, Zolertia <alinan@zolertia.com>
 *
 */

#include "contiki.h"
#include "relay.h"

//static uint8_t controlPin;

enum OPENMOTE_RELAY_STATUSTYPES {
  /* must be a bit and not more, not using 0x00. */
  INITED = 0x01,
  RUNNING = 0x02,
  STOPPED = 0x04,
};

static enum OPENMOTE_RELAY_STATUSTYPES _RELAY_STATUS = 0x00;

/*---------------------------------------------------------------------------*/

void
relay_enable(uint8_t pin)
{

  if(!(_RELAY_STATUS & INITED)) {

    _RELAY_STATUS |= INITED;

    /* Selects the pin to be configure as the control pin of the relay module */
    //controlPin = (1 << pin);
    GPIO_SET_OUTPUT(GPIO_D_BASE,0x03);

    /* Configures the control pin */
    //P6SEL &= ~controlPin;
    //P6DIR |= controlPin;
  }
}
/*---------------------------------------------------------------------------*/

void
relay_on()
{
  if((_RELAY_STATUS & INITED)) {
    
    GPIO_SET_PIN(GPIO_D_BASE,0x03);
    if (GPIO_READ_PIN(GPIO_D_BASE,0x03) == 3)
    {
    printf(" Relay_ON value on register is ON" );
    }
    //P6OUT |= controlPin;
  }
}
/*---------------------------------------------------------------------------*/
void
relay_off()
{
  if((_RELAY_STATUS & INITED)) {
  
     GPIO_CLR_PIN(GPIO_D_BASE,0x03);
     if ( GPIO_READ_PIN(GPIO_D_BASE,0x03) == 0 )
     {
     printf(" Relay_OFF value on register is OFF");
     }
    //P6OUT &= ~controlPin;
  }
}
/*---------------------------------------------------------------------------*/
