/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: beep.c,v 1.4 2006/07/07 06:36:38 nifi Exp $
 */

#include "contiki.h"
#include "contiki-esb.h"
#include "sys/clock.h"

#define ON  1
#define OFF 0

/*
 * Flag to indicate if any of these functions should generate a sound
 * or not. The function beep_off() is used to change this flag so none
 * of the functions will generate sounds and beep_on() to turn sound
 * back on.
 */
static char onoroff = ON;

/*
 * BEEPER_BIT is the bit in the io-register that is connected to the actual 
 * beeper, setting the bit high vill generate a high pitch tone.
 */
#define BEEPER_BIT 0x08

/*-----------------------------------------------------------------------------------*/
void
beep_alarm(int alarmmode, int len)
{
  len = len / 200;
  
  while(len > 0) {
    /*
     * Check here if we should beep or not since if we do it outside the
     * while loop the call to this function would take muck less time, i.e.
     * beep_on()/beep_off() would have side effects that might not be
     * predictable.
     */
    if(onoroff == ON) {
      if((alarmmode == BEEP_ALARM1) && ((len & 7) > 4)) {
	P2OUT |= BEEPER_BIT;
      } else if((alarmmode == BEEP_ALARM2) && ((len & 15) > 12)) {
	P2OUT |= BEEPER_BIT;
      } else {
	P2OUT &= ~BEEPER_BIT;
      }
    }
    clock_delay(200);
    len--;
  }
  P2OUT &= ~BEEPER_BIT;
}
/*-----------------------------------------------------------------------------------*/
void
beep_beep(int i)
{
  if(onoroff == ON) {
    /* Beep. */
    P2OUT |= BEEPER_BIT;
    clock_delay(i);
    P2OUT &= ~BEEPER_BIT;
  }
}
/*-----------------------------------------------------------------------------------*/
void
beep(void)
{
  beep_beep(20);
}
/*-----------------------------------------------------------------------------------*/
void
beep_down(int d)
{
  int i;
  for(i = 8; i < d; i += i / 8) {
    beep_beep(10);
    clock_delay(i);
  }
}
/*-----------------------------------------------------------------------------------*/
void
beep_on(void)
{
  onoroff = ON;
}
/*-----------------------------------------------------------------------------------*/
void
beep_off(void)
{
  onoroff = OFF;
}
/*-----------------------------------------------------------------------------------*/
void
beep_spinup(void)
{
  unsigned int i;

  for(i = 10000; i > 80; i -= i / 20) {
    beep_beep(2);
    clock_delay(i);
  }

  for(i = 4980; i > 2000; i -= 20) {
    leds_on(LEDS_ALL);
    clock_delay(5000 - i);
    leds_off(LEDS_ALL);
    clock_delay(i);
  }

}
/*-----------------------------------------------------------------------------------*/
void
beep_quick(int n)
{
  int i;
  for(i = 0; i < n; ++i) {
    beep_beep(2000);
    clock_delay(20000);
  }
}
/*-----------------------------------------------------------------------------------*/
void beep_long(clock_time_t len) {
  /*
   * Check if the beeper is turned on or off, i.e. if a call should generate 
   * a noise or not.
   */
  if(onoroff == ON) {
    /* Turn on the beeper. */
    P2OUT |= BEEPER_BIT;
  }

  clock_wait(len);

  if(onoroff == ON) {
    /* Turn the beeper off. */
    P2OUT &= ~BEEPER_BIT;
  }
}
/*-----------------------------------------------------------------------------------*/
