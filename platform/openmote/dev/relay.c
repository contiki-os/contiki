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
 *         Device simple driver for generic relay for openmote
 * \author
 *         Manoj Sony, <manojsony@gmail.com>
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

/*---------------------------------------------------------------------------
Function Name : relay_enable
INPUT Parmaters : port_address  Possible values :  { GPIO_A_BASE , GPIO_B_BASE ,GPIO_C_BASE, GPIO_D_BASE }
	          pin      	Possible values :  { Pin 0: 0, Pin 1: 1, Pin 2: 2... Pin 7: 7 }
OUTPUT Paramaters : None
---------------------------------------------------------------------------*/

void relay_enable(unsigned long port_addr, unsigned char pin)
{

  if(!(_RELAY_STATUS & INITED)) {

    _RELAY_STATUS |= INITED;
    /* Selects the pin to be configure as the relay pin of the relay module */
    GPIO_SET_OUTPUT(port_addr,(1<<pin));
    printf("RELAY PIN INITIALIZED SUCCESSFULLY FOR PORTBASE_ADDR : 0x%x , PIN : %d  \r\n",port_addr,pin );
  }

}

/*---------------------------------------------------------------------------
Function Name : relay_on
INPUT Parmaters : port_address  Possible values :  { GPIO_A_BASE , GPIO_B_BASE ,GPIO_C_BASE, GPIO_D_BASE }
	          pin      	Possible values :  { Pin 0: 0, Pin 1: 1, Pin 2: 2... Pin 7: 7 }
OUTPUT Paramaters : None
---------------------------------------------------------------------------*/
void relay_on(unsigned long port_addr, unsigned char pin)
{
  if((_RELAY_STATUS & INITED)) {
    
    GPIO_SET_PIN(port_addr,(1<<pin));
    
   if (GPIO_READ_PIN(port_addr,(1<<pin)) == (1<<pin))
    	printf("relay_on() value for PORTBASE_ADDR : 0x%x , PIN : %d  is ON\r\n",port_addr,pin );
   else
	printf("relay_on() value for PORTBASE_ADDR : 0x%x , PIN : %d  mismatch\r\n",port_addr,pin );
  }

}

/*---------------------------------------------------------------------------
Function Name : relay_off     
INPUT Parmaters : port_address  Possible values :  { GPIO_A_BASE , GPIO_B_BASE ,GPIO_C_BASE, GPIO_D_BASE }
	          pin      	Possible values :  { Pin 0: 0, Pin 1: 1, Pin 2: 2... Pin 7: 7 }
OUTPUT Paramaters : None
---------------------------------------------------------------------------*/
void relay_off(unsigned long port_addr, unsigned char pin)
{
  if((_RELAY_STATUS & INITED)) {
  
     GPIO_CLR_PIN(port_addr,(1<<pin));
     if ( GPIO_READ_PIN(port_addr,(1<<pin)) == 0 )
     	printf("relay_off() value for PORTBASE_ADDR : 0x%x , PIN : %d  is OFF\r\n",port_addr,pin );
     else
	printf("relay_off() value for PORTBASE_ADDR : 0x%x , PIN : %d  mismatch\r\n",port_addr,pin );    
    
  }
  
}
/*---------------------------------------------------------------------------*/
