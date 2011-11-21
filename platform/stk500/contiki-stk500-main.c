
/* Copyright (c) 2008, Daniel Willmann <daniel@totalueberwachung.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 * $Id$
 *
 */

#include "contiki.h"
#include "dev/rs232.h"

#include <avr/io.h>
#include <stdio.h>
#include <dev/watchdog.h>
#include <avr/pgmspace.h>

#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#define DEBUG 0
#if DEBUG
#define PRINTD PRINTA
#else
#define PRINTD(...)
#endif

/* Test rtimers, also stack monitor and time stamps */
#define TESTRTIMER 1
#if TESTRTIMER
#define STAMPS 30
#define STACKMONITOR 128

uint8_t rtimerflag=1;
uint16_t rtime;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif /* TESTRTIMER */

#if defined (__AVR_ATmega8__)
FUSES =
  {
    .low = 0xe0,
    .high = 0xd9,
  };
#elif defined (__AVR_ATmega16__)
FUSES =
  {
    .low = 0xe0,
    .high = 0x99,
  };
#elif defined (__AVR_ATmega644__)
FUSES =
  {
    .low = 0xe0,
    .high = 0x99,
    .extended = 0xff,
  };

//MCU=atmega8515
//MCU=atmega328p
//MCU=atmega32
#endif


PROCESS(led_process, "LED process");
PROCESS_THREAD(led_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_BEGIN();
  while (1) {
    PRINTD("LED1\r\n");
    PORTB |= (1<<PB1);
    PORTD |= (1<<PD3);
    etimer_set(&etimer, CLOCK_SECOND*0.5);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    PORTB &= ~(1<<PB1);
    PORTD &= ~(1<<PD3);
    etimer_set(&etimer, CLOCK_SECOND*0.5);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  }

  PROCESS_END();
}

PROCESS(led2_process, "LED process");
PROCESS_THREAD(led2_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_BEGIN();
  while (1) {
    PRINTD("LED2\r\n");
    PORTB |= (1<<PB0);
    PORTD |= (1<<PD2);
    etimer_set(&etimer, CLOCK_SECOND*0.3);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    PORTB &= ~(1<<PB0);
    PORTD &= ~(1<<PD2);
    etimer_set(&etimer, CLOCK_SECOND*0.3);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  }

  PROCESS_END();
}

void led_init()
{
  DDRB |= (1<<PB1)|(1<<PB0);
  PORTB &= ~((1<<PB1)|(1<<PB0));
  DDRD |= (1<<PD2)|(1<<PD3);
  PORTD &= ~((1<<PD2)|(1<<PD3));
}

/* These can also be explicitly started below */
PROCINIT(&etimer_process, &led_process, &led2_process);

void
initialize(void)
{
  watchdog_init();
  watchdog_start();
  
#if STACKMONITOR
  /* Simple stack pointer highwater monitor. Checks for magic numbers in the main
   * loop. In conjuction with TESTRTIMER, never-used stack will be printed
   * every STACKMONITOR seconds.
   */
{
extern uint16_t __bss_end;
uint16_t p=(uint16_t)&__bss_end;
    do {
      *(uint16_t *)p = 0x4242;
      p+=4;
    } while (p<SP-4); //don't overwrite our own stack
}
#endif

  /* rtimers needed for radio cycling */
  rtimer_init();

  rs232_init(RS232_PORT_0, BAUD_RATE(38400), USART_DATA_BITS_8 | USART_PARITY_NONE | USART_STOP_BITS_1);
  rs232_redirect_stdout(RS232_PORT_0);

  clock_init();
  sei();

  /* Initialize drivers and event kernel */
  process_init();
 
  led_init();

#if 0
  procinit_init();
#else
  process_start(&etimer_process, NULL);
  process_start(&led_process, NULL);
  process_start(&led2_process, NULL);
#endif

  PRINTA(CONTIKI_VERSION_STRING " started\r\n");

  /* Comment this out if autostart_processes not defined at link */
  /* Note AUTOSTART_PROCESSES(...) is only effective in the .co module */
  autostart_start(autostart_processes);
  
}
int
main(void)
{
  initialize();
  
  while(1) {
    process_run();
	
#if TESTRTIMER
/* Timeout can be increased up to 8 seconds maximum.
 * A one second cycle is convenient for triggering the various debug printouts.
 * The triggers are staggered to avoid printing everything at once.
 */
    if (rtimerflag) {
      rtimer_set(&rt, RTIMER_NOW()+ RTIMER_ARCH_SECOND*1UL, 1,(void *) rtimercycle, NULL);
      rtimerflag=0;

#if STAMPS
if ((rtime%STAMPS)==0) {
  PRINTA("%us ",rtime);
}
#endif
      rtime+=1;

#if STACKMONITOR
if ((rtime%STACKMONITOR)==3) {
  extern uint16_t __bss_end;
  uint16_t p=(uint16_t)&__bss_end;
  do {
    if (*(uint16_t *)p != 0x4242) {
      PRINTA("Never-used stack > %d bytes\n",p-(uint16_t)&__bss_end);
      break;
    }
    p+=4;
  } while (p<RAMEND-4);
}
#endif

    }
#endif /* TESTRTIMER */

  }

  return 0;
}
