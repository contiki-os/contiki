/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 */

#include "contiki.h"
#include "dev/spi.h"
#include "cc2420.h"
#include "isr_compat.h"

extern volatile uint8_t cc2420_sfd_counter;
extern volatile uint16_t cc2420_sfd_start_time;
extern volatile uint16_t cc2420_sfd_end_time;

/*---------------------------------------------------------------------------*/
/* SFD interrupt for timestamping radio packets */
ISR(TIMERB1, cc2420_timerb1_interrupt)
{
  int tbiv;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* always read TBIV to clear IFG */
  tbiv = TBIV;
  /* read and discard tbiv to avoid "variable set but not used" warning */
  (void)tbiv;
  if(CC2420_SFD_IS_1) {
    cc2420_sfd_counter++;
    cc2420_sfd_start_time = TBCCR1;
  } else {
    cc2420_sfd_counter = 0;
    cc2420_sfd_end_time = TBCCR1;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
cc2420_arch_sfd_init(void)
{
  /* Need to select the special function! */
  CC2420_SFD_PORT(SEL) = BV(CC2420_SFD_PIN);
  
  /* start timer B - 32768 ticks per second */
  TBCTL = TBSSEL_1 | TBCLR;
  
  /* CM_3 = capture mode - capture on both edges */
  TBCCTL1 = CM_3 | CAP | SCS;
  TBCCTL1 |= CCIE;
  
  /* Start Timer_B in continuous mode. */
  TBCTL |= MC1;

  TBR = RTIMER_NOW();
}
/*---------------------------------------------------------------------------*/
