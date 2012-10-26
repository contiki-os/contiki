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
 */
/**
 * \addtogroup esb
 * @{
 */

/**
 * \defgroup beeper Beeper interface 
 * @{
 */

/**
 * \file
 * Interface to the beeper.
 * \author Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __BEEP_H__
#define __BEEP_H__

#define BEEP_ALARM1 1
#define BEEP_ALARM2 2

#include "sys/clock.h"

/**
 * Beep for a specified time.
 *
 * This function causes the beeper to beep for the specified time. The
 * time is measured in the same units as for the clock_delay()
 * function.
 *
 * \note This function will hang the CPU during the beep.
 *
 * \note This function will stop any beep that was on previously when this 
 * function ends.
 *
 * \note If the beeper is turned off with beep_off() this call will still 
 * take the same time, though it will be silent.
 *
 * \param len The length of the beep.
 *
 */
void beep_beep(int len);

/**
 * Beep an alarm for a specified time.
 *
 * This function causes the beeper to beep for the specified time. The
 * time is measured in the same units as for the clock_delay()
 * function.
 *
 * \note This function will hang the CPU during the beep.
 *
 * \note This function will stop any beep that was on previously when this 
 * function ends.
 *
 * \note If the beeper is turned off with beep_off() this call will still 
 * take the same time, though it will be silent.
 *
 * \param alarmmode The alarm mode (BEEP_ALARM1,BEEP_ALARM2)
 * \param len The length of the beep.
 *
 */
void beep_alarm(int alarmmode, int len);

/**
 * Produces a quick click-like beep.
 *
 * This function produces a short beep that sounds like a click.
 *
 */
void beep(void);

/**
 * A beep with a pitch-bend down.
 *
 * This function produces a pitch-bend sound with deecreasing
 * frequency.
 *
 * \param len The length of the pitch-bend.
 *
 */
void beep_down(int len);

/**
 * Turn the beeper on.
 *
 * This function turns on the beeper. The beeper is turned off with
 * the beep_off() function.
 */
void beep_on(void);

/**
 * Turn the beeper off.
 *
 * This function turns the beeper off after it has been turned on with
 * beep_on().
 */
void beep_off(void);

/**
 * Produce a sound similar to a hard-drive spinup.
 *
 * This function produces a sound that is intended to be similar to
 * the sound a hard-drive makes when it starts.
 *
 */
void beep_spinup(void);

/**
 * Beep for a long time (seconds)
 *
 * This function produces a beep with the specified length and will
 * not return until the beep is complete. The length of the beep is
 * specified using CLOCK_SECOND: a two second beep is CLOCK_SECOND *
 * 2, and a quarter second beep is CLOCK_SECOND / 4.
 *
 * \note If the beeper is turned off with beep_off() this call will still 
 * take the same time, though it will be silent.
 *
 * \param len The length of the beep, measured in units of CLOCK_SECOND
 */
void beep_long(clock_time_t len);

void beep_quick(int num);

/** @} */
/** @} */

#endif /* __BEEP_H__ */
