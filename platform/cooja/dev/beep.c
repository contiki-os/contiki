/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: beep.c,v 1.1 2006/08/21 12:11:18 fros4943 Exp $
 */

#include "dev/beep.h"
#include "lib/simEnvChange.h"

const struct simInterface beep_interface;

// COOJA variables
char simBeeped;

/*-----------------------------------------------------------------------------------*/
void
beep_alarm(int alarmmode, int len)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_beep(int i)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep(void)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_down(int d)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_on(void)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_off(void)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_spinup(void)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void
beep_quick(int n)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
void beep_long(clock_time_t len)
{
  simBeeped = 1;
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(beep_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
