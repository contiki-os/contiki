/*
 * Copyright (c) 2013, NooliTIC
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
 */

/**
 * \file 1WR.c
 *  Provides the functionality of 1 wire.
 *
 * \author
 *  Ludovic WIART <ludovic.wiart@noolitic.biz>
 *
 *  History:
 *     16/03/2012 L. Wiart - Created
 *     08/02/2013 S. Dawans - Code Style & Integration in Contiki fork
 */

/*----------------------------------------------------------------------------*
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
                   Includes section
 *----------------------------------------------------------------------------*/
#include "contiki.h"
#include "1wr.h"
#include "gpio.h"

/*----------------------------------------------------------------------------*
                   function section
 *----------------------------------------------------------------------------*/

/* provide a delay of 1us */
#define DELAY_1US {asm volatile("nop");asm volatile("nop");asm volatile("nop");asm volatile("nop");asm volatile("nop");asm volatile("nop");asm volatile("nop");asm volatile("nop");}

/* provide a delay in us range min 4us for delay lower than 4us use the DELAY_1US several time    */
void
micro_delay(unsigned int i)
{
  if(i > 3) {
    i -= 3;
  }
  for(; i > 0; i--) {
    asm volatile ("nop");
    asm volatile ("nop");
  }
}

/*----------------------------------------------------------------------------*
                   Implementations section
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 reset the 1 wire.
Parameters:
Returns:
  1 if presence detect.
 *----------------------------------------------------------------------------*/
int
owr_reset(void)
{
  int result;

  cli();
  gpio_set_output(GPIO_1WR);
  gpio_set_off(GPIO_1WR);
  micro_delay(tRSTL);
  gpio_set_input(GPIO_1WR);
  micro_delay(tMSP);
  result = gpio_get(GPIO_1WR);
  micro_delay(tRSTL);
  sei();
  return !result;
}

/*----------------------------------------------------------------------------*
read one byte on 1 wire
Parameters:
Returns:
  value.
 *----------------------------------------------------------------------------*/
unsigned
owr_readb(void)
{
  unsigned result = 0;
  int i;

  for(i = 0; i < 8; i++) {
    result >>= 1;
    gpio_set_output(GPIO_1WR);
    micro_delay(tRL);
    gpio_set_input(GPIO_1WR);
    micro_delay(tMSR);
    if(gpio_get(GPIO_1WR)) {
      result |= 0x80;
    }
    micro_delay(tSLOT - tRL);
  }
  return result;
}

/*----------------------------------------------------------------------------*
write one byte on 1 wire.
Parameters:
   bytes to write.
Returns:
 *----------------------------------------------------------------------------*/
void
owr_writeb(unsigned byte)
{
  int i;

  for(i = 0; i < 8; i++) {
    if(byte & 0x01) {
      gpio_set_output(GPIO_1WR);
      micro_delay(tW1L);
      gpio_set_input(GPIO_1WR);
      micro_delay(tSLOT - tW1L);
    } else {
      gpio_set_output(GPIO_1WR);
      micro_delay(tW0L);
      gpio_set_input(GPIO_1WR);
      micro_delay(tSLOT - tW0L);
    }
    byte >>= 1;
  }
}

/* eof 1WR.c */
