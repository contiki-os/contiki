/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: clock.c,v 1.2 2008/07/12 14:44:37 oliverschmidt Exp $
 */

#include <time.h>

#include "contiki.h"

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  /* Contiki contains quite some calculations based on clock_time_t so we want
   * to avoid defining clock_time_t to be 32 bit because that would mean a lot
   * of overhead for cc65 targets.
   * On the other hand we want to avoid wrapping around frequently so the idea
   * is to reduce the clock resolution to the bare minimum. This is defined by
   * the TCP/IP stack using a 1/2 second periodic timer. So CLOCK_CONF_SECOND
   * needs to be defined at least as 2.
   * The value 2 works out especially nicely as it allows us to implement the
   * clock frequency devider below purely in (32 bit) integer arithmetic based
   * on the educated guess of CLK_TCK being an even value. */
  return clock() / (CLK_TCK / CLOCK_CONF_SECOND);
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return clock() / CLK_TCK;
}
/*---------------------------------------------------------------------------*/
