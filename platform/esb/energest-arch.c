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
 * $Id: energest-arch.c,v 1.2 2007/10/25 09:30:39 adamdunkels Exp $
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

#define DEC2FIX(h,d) ((h << 6) + (unsigned long)(0.64 * d))
/*#define DEC2FIX(h, d) (h << 8)*/

/*---------------------------------------------------------------------------*/
unsigned long
energest_arch_current_estimate(void)
{
  return
    energest_type_time(ENERGEST_TYPE_CPU)        * DEC2FIX(1,1) +
    energest_type_time(ENERGEST_TYPE_LPM)        * DEC2FIX(1,0) +
    energest_type_time(ENERGEST_TYPE_LED_GREEN)  * DEC2FIX(6,36)  +
    energest_type_time(ENERGEST_TYPE_LED_YELLOW) * DEC2FIX(3,46) +
    energest_type_time(ENERGEST_TYPE_LED_RED)    * DEC2FIX(3,69) +
    energest_type_time(ENERGEST_TYPE_SENSORS)    * DEC2FIX(3,76) +
    energest_type_time(ENERGEST_TYPE_TRANSMIT)   * DEC2FIX(5,1) +
    energest_type_time(ENERGEST_TYPE_LISTEN)     * DEC2FIX(4,8);
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
