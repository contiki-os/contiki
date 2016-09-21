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
 *
 * $Id: cputest.c,v 1.19 2007/10/24 22:17:46 nfi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2006-03-07
 * Updated : $Date: 2007/10/24 22:17:46 $
 *           $Revision: 1.19 $
 */

#include "msp430setup.h"
#if __MSPGCC__
#include <msp430.h>
#include <legacymsp430.h>
#else /* __MSPGCC__ */
#include <signal.h>
#include <io.h>
#endif /* __MSPGCC__ */
#include <string.h>
#include <stdio.h>

/* From Adams test-suite */
#define TEST(...) if(__VA_ARGS__) {					 \
                    printf("OK: " #__VA_ARGS__ " passed at %s:%d\n", __FILE__,__LINE__); \
                  } else {						 \
                    printf("FAIL: " #__VA_ARGS__ " failed at %s:%d\n", __FILE__,__LINE__); \
                  }

#define TEST2(text,...) if(__VA_ARGS__) {					 \
                    printf("OK: " #text " passed at %s:%d\n", __FILE__,__LINE__); \
                  } else {						 \
                    printf("FAIL: " #text " failed at %s:%d\n", __FILE__,__LINE__); \
                  }

#define assertTrue(...) TEST(__VA_ARGS__)
#define assertFalse(...) TEST(!(__VA_ARGS__))

#define assertTrue2(text,...) TEST2(text,__VA_ARGS__)
#define assertFalse2(text,...) TEST2(text,!(__VA_ARGS__))

static int testzero(int hm)
{
  return hm > 0;
}

static int caseID = 0;

#define RTIMER_ARCH_SECOND 4096
#define CLOCK_SECOND 128
#define CLOCK_CONF_SECOND CLOCK_SECOND
#define INTERVAL (RTIMER_ARCH_SECOND / CLOCK_SECOND)

/*---------------------------------------------------------------------------*/
static int pos = 0;
static unsigned int ticka0 = 0;
static unsigned int count = 0;
static unsigned int seconds = 0;
static unsigned int last_tar = 0;

interrupt(TIMERA1_VECTOR) timera1 (void) {
  if(TAIV == 2) {
    eint();
    do {
      TACCR1 += INTERVAL;
      ++count;

      if (count == 5) {
	for (pos = 0; pos < 2000; pos++) {
	  last_tar = TAR;
	}
      }

      if(count % CLOCK_CONF_SECOND == 0) {
	++seconds;
      }
    } while((TACCR1 - TAR) > INTERVAL);
    last_tar = TAR;
  }
}


/*---------------------------------------------------------------------------*/
interrupt(TIMERA0_VECTOR) timera0 (void) {
  ticka0++;
  TACCR0 += 4;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
}

/*---------------------------------------------------------------------------*/

static void initTest() {
  caseID = 0;
}

static void testCase(char *description) {
  caseID++;
  printf("-------------\n");
  printf("TEST %d: %s\n", caseID, description);
}

static void testTimers() {
  dint();
  /* Timer A1 */
  /* Select ACLK 32768Hz clock, divide by 8 */
  TACTL = TASSEL0 | TACLR | ID_3;
  /* Initialize ccr1 to create the X ms interval. */
  /* CCR1 interrupt enabled, interrupt occurs when timer equals CCR1. */
  TACCTL1 = CCIE;
  /* Interrupt after X ms. */
  TACCR1 = INTERVAL;
  /* Start Timer_A in continuous mode. */
  TACTL |= MC1;
  count = 0;

  /* Timer A0 */
  /* CCR0 interrupt enabled, interrupt occurs when timer equals CCR0. */
  TACCTL0 = CCIE;
  TACCR0 = 40;

  /* Enable interrupts. */
  eint();


  while (ticka0 < 200) {
    printf("Timer a0:%d \n", ticka0);
  }
  
}

int
main(void)
{
  msp430_setup();

  initTest();

  testTimers();

  printf("EXIT\n");
  return 0;
}
