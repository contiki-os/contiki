/*
 * Copyright (c) 2012, Alex Barclay.
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
 *
 *
 * Author: Alex Barclay <alex@planet-barclay.com>
 *
 */

#include <p33Fxxxx.h>

#include "contiki-conf.h"
#include "dspictimer.h"

volatile uint32_t dspic_tickCounter;

void
dspic_timer_init()
{
  dspic_tickCounter = 0;

  /* Setup for a 100Hz timer given a Fcy of 40MHz */
  /* Fcy = 40000000 */
  /* Prescale = 64 */
  /* For 100Hz need 6250 in PR */
  T1CONbits.TON = 0; /* Disable Timer */
  T1CONbits.TCS = 0; /* Select internal instruction cycle clock */
  T1CONbits.TGATE = 0; /* Disable Gated Timer mode */
  T1CONbits.TCKPS = 0b10; /* Select 1:64 Prescaler */
  TMR1 = 0; /* Clear timer register */
  PR1 = 6250; /* Load the period value */
  IPC0bits.T1IP = 0x04; /* Set Timer1 Interrupt Priority Level */
  IFS0bits.T1IF = 0; /* Clear Timer1 Interrupt Flag */
  IEC0bits.T1IE = 1; /* Enable Timer1 interrupt */
  T1CONbits.TON = 1; /* Start Timer */
}
/* Timer 1 ISR */
void __attribute__((__interrupt__, __no_auto_psv__))
_T1Interrupt()
{
  ++dspic_tickCounter;

  IFS0bits.T1IF = 0; /* Clear Timer1 interrupt flag */
}
