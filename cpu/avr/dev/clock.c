/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 *  \brief This module contains AVR-specific code to implement
 *  the Contiki core clock functions.
 *  
 *  \author David Kopf <dak664@embarqmail.com> and others.
 *
*/
/** \addtogroup avr
 * @{
 */
 /**
 *  \defgroup avrclock AVR clock implementation
 * @{
 */
/**
 *  \file
 *  This file contains AVR-specific code to implement the Contiki core clock functions.
 *
 */
/**
 * These routines define the AVR-specific calls declared in /core/sys/clock.h
 * CLOCK_SECOND is the number of ticks per second.
 * It is defined through CONF_CLOCK_SECOND in the contiki-conf.h for each platform.
 * The usual AVR defaults are 128 or 125 ticks per second, counting a prescaled CPU clock
 * using the 8 bit timer0.
 * 
 * clock_time_t is usually declared by the platform as an unsigned 16 bit data type,
 * thus intervals up to 512 or 524 seconds can be measured with ~8 millisecond precision.
 * For longer intervals the 32 bit clock_seconds() is available.
 * 
 * Since a carry to a higer byte can occur during an interrupt, declaring them non-static
 * for direct examination can cause occasional time reversals!
 *
 * clock-avr.h contains the specific setup code for each mcu.
 */
#include "sys/clock.h"
#include "dev/clock-avr.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

/* Two tick counters avoid a software divide when CLOCK_SECOND is not a power of two. */
#if CLOCK_SECOND && (CLOCK_SECOND - 1)
#define TWO_COUNTERS 1
#endif

/* count is usually a 16 bit variable, although the platform can declare it otherwise */
static volatile clock_time_t count;
#if TWO_COUNTERS
/* scount is the 8 bit counter that counts ticks modulo CLOCK_SECONDS */
static volatile uint8_t scount;
#endif
/* seconds is available globally but non-atomic update during interrupt can cause time reversals */
volatile unsigned long seconds;
/* sleepseconds is the number of seconds sleeping since startup, available globally */
long sleepseconds;

/* Set RADIOSTATS to monitor radio on time (must also be set in the radio driver) */
#if RF230BB && AVR_WEBSERVER
#define RADIOSTATS 1
#endif

#if RADIOSTATS
static volatile uint8_t rcount;
volatile unsigned long radioontime;
extern uint8_t RF230_receive_on;
#endif

/* Set RADIO_CONF_CALIBRATE_INTERVAL for periodic calibration of the PLL during extended radio on time.
 * The RF230 data sheet suggests every 5 minutes if the temperature is fluctuating.
 * At present the specified interval is ignored, and an 8 bit counter gives 256 second intervals.
 * Actual calibration is done by the driver on the next transmit request.
 */
#if RADIO_CONF_CALIBRATE_INTERVAL
extern volatile uint8_t rf230_calibrate;
static uint8_t calibrate_interval;
#endif

/*---------------------------------------------------------------------------*/
/**
 * Start the clock by enabling the timer comparison interrupts. 
 */
void
clock_init(void)
{
  cli ();
  OCRSetup();
  sei ();
}
/*---------------------------------------------------------------------------*/
/**
 * Return the tick counter. When 16 bit it typically wraps every 10 minutes.
 * The comparison avoids the need to disable clock interrupts for an atomic
 * read of the multi-byte variable.
 */
clock_time_t
clock_time(void)
{
  clock_time_t tmp;
  do {
    tmp = count;
  } while(tmp != count);
  return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Return seconds, default is time since startup.
 * The comparison avoids the need to disable clock interrupts for an atomic
 * read of the four-byte variable.
 */
unsigned long
clock_seconds(void)
{
  unsigned long tmp;
  do {
    tmp = seconds;
  } while(tmp != seconds);
  return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Set seconds, e.g. to a standard epoch for an absolute date/time.
 */
void
clock_set_seconds(unsigned long sec)
{
  seconds = sec;
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a number of clock ticks.
 */
void
clock_wait(clock_time_t t)
{
  clock_time_t endticks = clock_time() + t;
  if (sizeof(clock_time_t) == 1) {
    while ((signed char )(clock_time() - endticks) < 0) {;}
  } else if (sizeof(clock_time_t) == 2) {
    while ((signed short)(clock_time() - endticks) < 0) {;}
  } else {
    while ((signed long )(clock_time() - endticks) < 0) {;}
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for up to 65535*(4000000/F_CPU) microseconds.
 * Copied from _delay_loop_2 in AVR library delay_basic.h, 4 clocks per loop.
 * For accurate short delays, inline _delay_loop_2 in the caller, use a constant
 * value for the delay, and disable interrupts if necessary.
 */
static inline void my_delay_loop_2(uint16_t __count) __attribute__((always_inline));
void
my_delay_loop_2(uint16_t __count)
{
  __asm__ volatile (
    "1: sbiw %0,1" "\n\t"
    "brne 1b"
    : "=w" (__count)
    : "0" (__count)
  );
}
void
clock_delay_usec(uint16_t howlong)
{
#if 0
/* Accurate delay at any frequency, but introduces a 64 bit intermediate
  * and has a 279 clock overhead.
 */
  if(howlong<=(uint16_t)(279000000UL/F_CPU)) return;
  howlong-=(uint16_t) (279000000UL/F_CPU);
  my_delay_loop_2(((uint64_t)(howlong) * (uint64_t) F_CPU) / 4000000ULL);
  /* Remaining numbers tweaked for the breakpoint CPU frequencies */
  /* Add other frequencies as necessary */
#elif F_CPU>=16000000UL
  if(howlong<1) return;
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/3250000)));
#elif F_CPU >= 12000000UL
  if(howlong<2) return;
  howlong-=(uint16_t) (3*12000000/F_CPU);
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/3250000)));
#elif F_CPU >= 8000000UL
  if(howlong<4) return;
  howlong-=(uint16_t) (3*8000000/F_CPU);
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/2000000))/2);
#elif F_CPU >= 4000000UL
  if(howlong<5) return;
  howlong-=(uint16_t) (4*4000000/F_CPU);
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/2000000))/2);
#elif F_CPU >= 2000000UL
  if(howlong<11) return;
  howlong-=(uint16_t) (10*2000000/F_CPU);
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/1000000))/4);
#elif F_CPU >= 1000000UL
  if(howlong<=17) return;
  howlong-=(uint16_t) (17*1000000/F_CPU);
  my_delay_loop_2((howlong*(uint16_t)(F_CPU/1000000))/4);
#else
  howlong >> 5;
  if (howlong < 1) return;
  my_delay_loop_2(howlong);
#endif
}
#if 0
/*---------------------------------------------------------------------------*/
/**
 * Legacy delay. The original clock_delay for the msp430 used a granularity
 * of 2.83 usec. This approximates that delay for values up to 1456 usec.
 * (The largest core call in leds.c uses 400).
 */
void
clock_delay(unsigned int howlong)
{
  if(howlong<2) return;
  clock_delay_usec((45*howlong)>>4);
}
#endif
/*---------------------------------------------------------------------------*/
/**
 * Delay up to 65535 milliseconds.
 * \param dt   How many milliseconds to delay.
 *
 * Neither interrupts nor the watchdog timer is disabled over the delay.
 * Platforms are not required to implement this call.
 * \note This will break for CPUs clocked above 260 MHz.
 */
void
clock_delay_msec(uint16_t howlong)
{

#if F_CPU>=16000000
  while(howlong--) clock_delay_usec(1000);
#elif F_CPU>=8000000
  uint16_t i=996;
  while(howlong--) {clock_delay_usec(i);i=999;}
#elif F_CPU>=4000000
  uint16_t i=992;
  while(howlong--) {clock_delay_usec(i);i=999;}
#elif F_CPU>=2000000
  uint16_t i=989;
  while(howlong--) {clock_delay_usec(i);i=999;}
#else
  uint16_t i=983;
  while(howlong--) {clock_delay_usec(i);i=999;}
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * Adjust the system current clock time.
 * \param dt   How many ticks to add
 *
 * Typically used to add ticks after an MCU sleep
 * clock_seconds will increment if necessary to reflect the tick addition.
  * Leap ticks or seconds can (rarely) be introduced if the ISR is not blocked.
 */
void
clock_adjust_ticks(clock_time_t howmany)
{
  uint8_t sreg = SREG;cli();
  count  += howmany;
#if TWO_COUNTERS
  howmany+= scount;
#endif
  while(howmany >= CLOCK_SECOND) {
    howmany -= CLOCK_SECOND;
    seconds++;
    sleepseconds++;
#if RADIOSTATS
    if (RF230_receive_on) radioontime += 1;
#endif
  }
#if TWO_COUNTERS
  scount = howmany;
#endif
  SREG=sreg;
}
/*---------------------------------------------------------------------------*/
/* This it the timer comparison match interrupt.
 * It maintains the tick counter, clock_seconds, and etimer updates.
 *
 * If the interrupts derive from an external crystal, the CPU instruction
 * clock can optionally be phase locked to it. This allows accurate rtimer
 * interrupts for strobe detection during radio duty cycling.
 * Phase lock is accomplished by adjusting OSCCAL based on the phase error
 * since the last interrupt.
 */
/*---------------------------------------------------------------------------*/
#if defined(DOXYGEN)
/** \brief ISR for the TIMER0 or TIMER2 interrupt as defined in
 *  clock-avr.h for the particular MCU.
 */
void AVR_OUTPUT_COMPARE_INT(void);
#else
ISR(AVR_OUTPUT_COMPARE_INT)
{
    count++;
#if TWO_COUNTERS
  if(++scount >= CLOCK_SECOND) {
    scount = 0;
#else
  if(count%CLOCK_SECOND==0) {
#endif
    seconds++;

#if RADIO_CONF_CALIBRATE_INTERVAL
   /* Force a radio PLL frequency calibration every 256 seconds */
    if (++calibrate_interval==0) {
      rf230_calibrate=1;
    }
#endif

  }

#if RADIOSTATS
   /* Sample radio on time. Less accurate than ENERGEST but a smaller footprint */
  if (RF230_receive_on) {
    if (++rcount >= CLOCK_SECOND) {
      rcount=0;
      radioontime++;
    }
  }
#endif
 
#if F_CPU == 0x800000 && USE_32K_CRYSTAL
/* Special routine to phase lock CPU to 32768 watch crystal.
 * We are interrupting 128 times per second.
 * If RTIMER_ARCH_SECOND is a multiple of 128 we can use the residual modulo
 * 128 to determine whether the clock is too fast or too slow.
 * E.g. for 8192 the phase should be constant modulo 0x40
 * OSCCAL is started in the lower range at 90, allowed to stabilize, then
 * rapidly raised or lowered based on the phase comparison.
 * It gives less phase noise to do this every tick and doesn't seem to hurt anything.
 */
#include "rtimer-arch.h"
{
volatile static uint8_t lockcount;
volatile static int16_t last_phase;
volatile static uint8_t osccalhigh,osccallow;
  if (seconds < 60) { //give a minute to stabilize
    if(++lockcount >= 8192UL*128/RTIMER_ARCH_SECOND) {
      lockcount=0;
      rtimer_phase = TCNT3 & 0x0fff;
      if (seconds < 2) OSCCAL=100;
      if (last_phase > rtimer_phase) osccalhigh=++OSCCAL; else osccallow=--OSCCAL;
      last_phase = rtimer_phase;
    }
  } else {
    uint8_t error = (TCNT3 - last_phase) & 0x3f;
    if (error == 0) {
    } else if (error<32) {
      OSCCAL=osccallow-1;
    } else {
      OSCCAL=osccalhigh+1;
    }
  }
}
#endif

#if 1
/*  gcc will save all registers on the stack if an external routine is called */
  if(etimer_pending()) {
    etimer_request_poll();
  }
#else
/* doing this locally saves 9 pushes and 9 pops, but these etimer.c and process.c variables have to lose the static qualifier */
  extern struct etimer *timerlist;
  extern volatile unsigned char poll_requested;

#define PROCESS_STATE_NONE        0
#define PROCESS_STATE_RUNNING     1
#define PROCESS_STATE_CALLED      2

  if (timerlist) {
    if(etimer_process.state == PROCESS_STATE_RUNNING || etimer_process.state == PROCESS_STATE_CALLED) {
      etimer_process.needspoll = 1;
      poll_requested = 1;
    }
  }
#endif
}
#endif /* defined(DOXYGEN) */
/*---------------------------------------------------------------------------*/
/* Debugging aids */

#ifdef HANDLE_UNSUPPORTED_INTERRUPTS
/* Ignore unsupported interrupts, optionally hang for debugging */
/* BADISR is a gcc weak symbol that matches any undefined interrupt */
ISR(BADISR_vect) {
//static volatile uint8_t x;while (1) x++;
}
#endif
#ifdef HANG_ON_UNKNOWN_INTERRUPT
/* Hang on any unsupported interrupt */
/* Useful for diagnosing unknown interrupts that reset the mcu.
 * Currently set up for 12mega128rfa1.
 * For other mcus, enable all and then disable the conflicts.
 */
static volatile uint8_t x;
ISR( _VECTOR(0)) {while (1) x++;}
ISR( _VECTOR(1)) {while (1) x++;}
ISR( _VECTOR(2)) {while (1) x++;}
ISR( _VECTOR(3)) {while (1) x++;}
ISR( _VECTOR(4)) {while (1) x++;}
ISR( _VECTOR(5)) {while (1) x++;}
ISR( _VECTOR(6)) {while (1) x++;}
ISR( _VECTOR(7)) {while (1) x++;}
ISR( _VECTOR(8)) {while (1) x++;}
ISR( _VECTOR(9)) {while (1) x++;}
ISR( _VECTOR(10)) {while (1) x++;}
ISR( _VECTOR(11)) {while (1) x++;}
ISR( _VECTOR(12)) {while (1) x++;}
ISR( _VECTOR(13)) {while (1) x++;}
ISR( _VECTOR(14)) {while (1) x++;}
ISR( _VECTOR(15)) {while (1) x++;}
ISR( _VECTOR(16)) {while (1) x++;}
ISR( _VECTOR(17)) {while (1) x++;}
ISR( _VECTOR(18)) {while (1) x++;}
ISR( _VECTOR(19)) {while (1) x++;}
//ISR( _VECTOR(20)) {while (1) x++;}
//ISR( _VECTOR(21)) {while (1) x++;}
ISR( _VECTOR(22)) {while (1) x++;}
ISR( _VECTOR(23)) {while (1) x++;}
ISR( _VECTOR(24)) {while (1) x++;}
//ISR( _VECTOR(25)) {while (1) x++;}
ISR( _VECTOR(26)) {while (1) x++;}
//ISR( _VECTOR(27)) {while (1) x++;}
ISR( _VECTOR(28)) {while (1) x++;}
ISR( _VECTOR(29)) {while (1) x++;}
ISR( _VECTOR(30)) {while (1) x++;}
ISR( _VECTOR(31)) {while (1) x++;}
//ISR( _VECTOR(32)) {while (1) x++;}
ISR( _VECTOR(33)) {while (1) x++;}
ISR( _VECTOR(34)) {while (1) x++;}
ISR( _VECTOR(35)) {while (1) x++;}
//ISR( _VECTOR(36)) {while (1) x++;}
ISR( _VECTOR(37)) {while (1) x++;}
//ISR( _VECTOR(38)) {while (1) x++;}
ISR( _VECTOR(39)) {while (1) x++;}
ISR( _VECTOR(40)) {while (1) x++;}
ISR( _VECTOR(41)) {while (1) x++;}
ISR( _VECTOR(42)) {while (1) x++;}
ISR( _VECTOR(43)) {while (1) x++;}
ISR( _VECTOR(44)) {while (1) x++;}
ISR( _VECTOR(45)) {while (1) x++;}
ISR( _VECTOR(46)) {while (1) x++;}
ISR( _VECTOR(47)) {while (1) x++;}
ISR( _VECTOR(48)) {while (1) x++;}
ISR( _VECTOR(49)) {while (1) x++;}
ISR( _VECTOR(50)) {while (1) x++;}
ISR( _VECTOR(51)) {while (1) x++;}
ISR( _VECTOR(52)) {while (1) x++;}
ISR( _VECTOR(53)) {while (1) x++;}
ISR( _VECTOR(54)) {while (1) x++;}
ISR( _VECTOR(55)) {while (1) x++;}
ISR( _VECTOR(56)) {while (1) x++;}
//ISR( _VECTOR(57)) {while (1) x++;}
//ISR( _VECTOR(58)) {while (1) x++;}
//ISR( _VECTOR(59)) {while (1) x++;}
//ISR( _VECTOR(60)) {while (1) x++;}
ISR( _VECTOR(61)) {while (1) x++;}
ISR( _VECTOR(62)) {while (1) x++;}
ISR( _VECTOR(63)) {while (1) x++;}
ISR( _VECTOR(64)) {while (1) x++;}
ISR( _VECTOR(65)) {while (1) x++;}
ISR( _VECTOR(66)) {while (1) x++;}
ISR( _VECTOR(67)) {while (1) x++;}
ISR( _VECTOR(68)) {while (1) x++;}
ISR( _VECTOR(69)) {while (1) x++;}
ISR( _VECTOR(70)) {while (1) x++;}
ISR( _VECTOR(71)) {while (1) x++;}
ISR( _VECTOR(72)) {while (1) x++;}
ISR( _VECTOR(73)) {while (1) x++;}
ISR( _VECTOR(74)) {while (1) x++;}
ISR( _VECTOR(75)) {while (1) x++;}
ISR( _VECTOR(76)) {while (1) x++;}
ISR( _VECTOR(77)) {while (1) x++;}
ISR( _VECTOR(78)) {while (1) x++;}
ISR( _VECTOR(79)) {while (1) x++;}
#endif
/** @} */
/** @} */
