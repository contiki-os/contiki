/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: button.c,v 1.2 2006/08/17 15:41:13 bg- Exp $
 */

#include "contiki.h"
#include "dev/button.h"
#include "isr_compat.h"

#define BUTTON_PORT 2
#define BUTTON_PIN  7

static struct button_msg button_msg;

static struct process *selecting_proc;

void
button_init(struct process *proc)
{
  button_msg.type = BUTTON_MSG_TYPE;

  P2DIR &= ~BV(BUTTON_PIN);
  P2SEL &= ~BV(BUTTON_PIN);

  P2IES |= BV(BUTTON_PIN);
  P2IFG &= ~BV(BUTTON_PIN);

  selecting_proc = proc;
  if(proc != NULL)
    P2IE |= BV(BUTTON_PIN);
  else
    P2IE &= ~BV(BUTTON_PIN);
}

ISR(PORT2, __button_interrupt)
{
  static struct timer debouncetimer;

  P2IFG &= ~BV(BUTTON_PIN);
  if(timer_expired(&debouncetimer)) {
    button_msg.value = P2IN & BV(BUTTON_PIN);
    timer_set(&debouncetimer, CLOCK_SECOND/4);
    if(selecting_proc != NULL) {
      process_post(selecting_proc, PROCESS_EVENT_MSG, &button_msg);
    }
    LPM4_EXIT;
  }
}
