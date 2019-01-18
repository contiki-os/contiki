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
 * $Id: energest-arch.c,v 1.4 2007/12/17 18:47:17 nvt-se Exp $
 */

/**
 * \file
 *         Architecture (board) dependent code for energy estimation
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "lib/energest.h"

#include "sys/clock.h"
#include "sys/rtimer.h"

#define DEC2FIX(h,d) ((h * 64) + (unsigned long)(0.64 * d))

/*---------------------------------------------------------------------------*/
unsigned long
energest_arch_current_estimate(void)
{
  return 3 * /* The voltage is 3 V */            /* The DEC2FIX() stuff is
						    the current */
    (energest_type_time(ENERGEST_TYPE_CPU)        * DEC2FIX(18,0) +
     energest_type_time(ENERGEST_TYPE_LPM)        * DEC2FIX(0,545) +
     energest_type_time(ENERGEST_TYPE_LED_GREEN)  * DEC2FIX(46,0)  + /* Not measured */
     energest_type_time(ENERGEST_TYPE_LED_YELLOW) * DEC2FIX(46,0) +  /* Not measured */
     energest_type_time(ENERGEST_TYPE_LED_RED)    * DEC2FIX(46,0) +
     energest_type_time(ENERGEST_TYPE_SENSORS)    * DEC2FIX(60,0) +  /* Not measured */
     energest_type_time(ENERGEST_TYPE_TRANSMIT)   * DEC2FIX(177,0) +
     energest_type_time(ENERGEST_TYPE_LISTEN)     * DEC2FIX(200,0)) / 10;
}
/*---------------------------------------------------------------------------*/
unsigned short
energest_arch_now(void)
{
  unsigned long time;

#define ENERGEST_USE_CLOCK_TIME 0
  
#if ENERGEST_USE_CLOCK_TIME
  time = clock_time();
#else
  time = rtimer_arch_now();
#endif

  
  return time;
}    
/*---------------------------------------------------------------------------*/
