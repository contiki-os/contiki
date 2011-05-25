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
 * @(#)$Id: msp430.c,v 1.1 2010/08/24 16:26:38 joxe Exp $
 */

#include "contiki.h"

#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#include <sys/unistd.h>
#define asmv(arg) __asm__ __volatile__(arg)
#endif

#include "msp430def.h"
#include "dev/watchdog.h"
#include "net/uip.h"

/*---------------------------------------------------------------------------*/
#if defined(__MSP430__) && defined(__GNUC__) && MSP430_MEMCPY_WORKAROUND
void *
w_memcpy(void *out, const void *in, size_t n)
{
  uint8_t *src, *dest;
  src = (uint8_t *) in;
  dest = (uint8_t *) out;
  while(n-- > 0) {
    *dest++ = *src++;
  }
  return out;
}
#endif /* __GNUC__ &&  __MSP430__ && MSP430_MEMCPY_WORKAROUND */
/*---------------------------------------------------------------------------*/
#if defined(__MSP430__) && defined(__GNUC__) && MSP430_MEMCPY_WORKAROUND
void *
w_memset(void *out, int value, size_t n)
{
  uint8_t *dest;
  dest = (uint8_t *) out;
  while(n-- > 0) {
    *dest++ = value & 0xff;
  }
  return out;
}
#endif /* __GNUC__ &&  __MSP430__ && MSP430_MEMCPY_WORKAROUND */
/*---------------------------------------------------------------------------*/
void
msp430_init_dco(void)
{
  if(CALBC1_8MHZ != 0xFF) {
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_8MHZ;                    /*Set DCO to 8MHz */
    DCOCTL = CALDCO_8MHZ;
  } else { /*start using reasonable values at 8 Mhz */
    DCOCTL = 0x00;
    BCSCTL1 = 0x8D;
    DCOCTL = 0x88;
  }

  /*BCSCTL1 |= XT2OFF; // Make sure XT2 is off */
  /* BCSCTL2 = 0x00;   //  MCLK  = DCOCLK/1 */ 
  /* SMCLK = DCOCLK/1 */
  /* DCO Internal Resistor  */
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Start CPU with full speed (? good or bad?) and go downwards               */
/*---------------------------------------------------------------------------*/
void
msp430_quick_synch_dco(void) {
  uint16_t last;
  uint16_t diff;
  uint16_t dco_reg = 0x0fff;
  uint8_t current_bit = 12;
  uint16_t i;
  /*  DELTA_2 assumes an ACLK of 32768 Hz */
#define DELTA_2    ((MSP430_CPU_SPEED) / 32768)

  /* Select SMCLK clock, and capture on ACLK for TBCCR6 */
  TBCTL = TBSSEL1 | TBCLR;
  TBCCTL6 = CCIS0 + CM0 + CAP;
  /* start the timer */
  TBCTL |= MC1;

  BCSCTL1 = 0x8D | 7;
  DCOCTL = 0xff; /* MAX SPEED ?? */

  /* IDEA: do binary search - check MSB first, etc...   */
  /* 1 set current bit to zero - if to slow, put back to 1 */
  while(current_bit--) {
    /* first set the current bit to zero and check - we know that it is
       set from start so ^ works (first bit = bit 11) */
    dco_reg = dco_reg ^ (1 << current_bit); /* clear bit 11..10..9.. */

    /* set dco registers */
    DCOCTL = dco_reg & 0xff;
    BCSCTL1 = (BCSCTL1 & 0xf8) | (dco_reg >> 8);

    /* some delay to make clock stable - could possibly be made using
       captures too ... */
    for(i=0; i < 1000; i++) {
      i = i | 1;
    }


    /* do capture... */
    while(!(TBCCTL6 & CCIFG));
    last = TBCCR6;

    TBCCTL6 &= ~CCIFG;
    /* wait for next Capture - and calculate difference */
    while(!(TBCCTL6 & CCIFG));
    diff = TBCCR6 - last;

/*     /\* store what was run during the specific test *\/ */
/*     dcos[current_bit] = dco_reg; */
/*     vals[current_bit] = diff; */

    /* should we keep the bit cleared or not ? */
    if (diff < DELTA_2) { /* DCO is too slow - fewer ticks than desired */
      /* toggle bit again to get it back to one */
      dco_reg = dco_reg ^ (1 << current_bit);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
msp430_init_dco_old(void) /*Enric NOT IN USE, RIGHT NOW */
{
    /* This code taken from the FU Berlin sources and reformatted. */
#define DELTA    ((MSP430_CPU_SPEED) / (32768 / 8))

  unsigned int compare, oldcapture = 0;
  unsigned int i;


  BCSCTL1 = 0xa4; /* ACLK is devided by 4. RSEL=6 no division for MCLK
		     and SSMCLK. XT2 is off. */

  BCSCTL2 = 0x00; /* Init FLL to desired frequency using the 32762Hz
		     crystal DCO frquenzy = 2,4576 MHz  */

  BCSCTL1 |= DIVA1 + DIVA0;             /* ACLK = LFXT1CLK/8 */
  for(i = 0xffff; i > 0; i--) {         /* Delay for XTAL to settle */
    asm("nop");
  }

  CCTL2 = CCIS0 + CM0 + CAP;            /* Define CCR2, CAP, ACLK */
  TACTL = TASSEL1 + TACLR + MC1;        // SMCLK, continous mode


  while(1) {

    while((CCTL2 & CCIFG) != CCIFG);    /* Wait until capture occured! */
    CCTL2 &= ~CCIFG;                    /* Capture occured, clear flag */
    compare = CCR2;                     /* Get current captured SMCLK */
    compare = compare - oldcapture;     /* SMCLK difference */
    oldcapture = CCR2;                  /* Save current captured SMCLK */

    if(DELTA == compare) {
      break;                            /* if equal, leave "while(1)" */
    } else if(DELTA < compare) {        /* DCO is too fast, slow it down */
      DCOCTL--;
      if(DCOCTL == 0xFF) {              /* Did DCO role under? */
	BCSCTL1--;
      }
    } else {                            /* -> Select next lower RSEL */
      DCOCTL++;
      if(DCOCTL == 0x00) {              /* Did DCO role over? */
	BCSCTL1++;
      }
                                        /* -> Select next higher RSEL  */
    }
  }

  CCTL2 = 0;                            /* Stop CCR2 function */
  TACTL = 0;                            /* Stop Timer_A */

  BCSCTL1 &= ~(DIVA1 + DIVA0);          /* remove /8 divisor from ACLK again */
}
/*---------------------------------------------------------------------------*/

static void
init_ports(void)
{
  /* Turn everything off, device drivers enable what is needed. */

  /* All configured for digital I/O */
#ifdef P1SEL
  P1SEL = 0;
#endif
#ifdef P2SEL
  P2SEL = 0;
#endif
#ifdef P3SEL
  P3SEL = 0;
#endif
#ifdef P4SEL
  P4SEL = 0;
#endif
#ifdef P5SEL
  P5SEL = 0;
#endif
#ifdef P6SEL
  P6SEL = 0;
#endif

  /* All available inputs */
#ifdef P1DIR
  P1DIR = 0;
  P1OUT = 0;
#endif
#ifdef P2DIR
  P2DIR = 0;
  P2OUT = 0;
#endif
#ifdef P3DIR
  P3DIR = 0;
  P3OUT = 0;
#endif
#ifdef P4DIR
  P4DIR = 0;
  P4OUT = 0;
#endif

#ifdef P5DIR
  P5DIR = 0;
  P5OUT = 0;
#endif

#ifdef P6DIR
  P6DIR = 0;
  P6OUT = 0;
#endif

  P1IE = 0;
  P2IE = 0;
}
/*---------------------------------------------------------------------------*/
/* msp430-ld may align _end incorrectly. Workaround in cpu_init. */
#ifdef __GNUC__
extern int _end;		/* Not in sys/unistd.h */
static char *cur_break = (char *)&_end;
#endif

void
msp430_cpu_init(void)
{
  dint();
  watchdog_init();
  init_ports();
  msp430_quick_synch_dco();
  eint();
#ifdef __GNUC__
  if((uintptr_t)cur_break & 1) { /* Workaround for msp430-ld bug! */
    cur_break++;
  }
#endif
}
/*---------------------------------------------------------------------------*/

#define STACK_EXTRA 32

/*---------------------------------------------------------------------------*/
/*
 * Mask all interrupts that can be masked.
 */
int
splhigh_(void)
{
  /* Clear the GIE (General Interrupt Enable) flag. */
  int sr;
#ifdef __IAR_SYSTEMS_ICC__
  sr = __get_SR_register();
  __bic_SR_register(GIE);
#else
  asmv("mov r2, %0" : "=r" (sr));
  asmv("bic %0, r2" : : "i" (GIE));
#endif
  return sr & GIE;		/* Ignore other sr bits. */
}
/*---------------------------------------------------------------------------*/
/*
 * Restore previous interrupt mask.
 */
void
splx_(int sr)
{
#ifdef __IAR_SYSTEMS_ICC__
  __bis_SR_register(sr);
#else
  /* If GIE was set, restore it. */
  asmv("bis %0, r2" : : "r" (sr));
#endif
}
/*---------------------------------------------------------------------------*/
/* this code will always start the TimerB if not already started */
void
msp430_sync_dco(void) {
  uint16_t last;
  uint16_t diff;
/*   uint32_t speed; */
  /* DELTA_2 assumes an ACLK of 32768 Hz */
#define DELTA_2    ((MSP430_CPU_SPEED) / 32768)

  /* Select SMCLK clock, and capture on ACLK for TBCCR6 */
  TBCTL = TBSSEL1 | TBCLR;
  TBCCTL6 = CCIS0 + CM0 + CAP;
  /* start the timer */
  TBCTL |= MC1;

  /* wait for next Capture */
  TBCCTL6 &= ~CCIFG;
  while(!(TBCCTL6 & CCIFG));
  last = TBCCR6;

  TBCCTL6 &= ~CCIFG;
  /* wait for next Capture - and calculate difference */
  while(!(TBCCTL6 & CCIFG));
  diff = TBCCR6 - last;

  /* Stop timer - conserves energy according to user guide */
  TBCTL = 0;

  /*   speed = diff; */
  /*   speed = speed * 32768; */
  /*   printf("Last TAR diff:%d target: %ld ", diff, DELTA_2); */
  /*   printf("CPU Speed: %lu DCOCTL: %d\n", speed, DCOCTL); */

  /* resynchronize the DCO speed if not at target */
  if(DELTA_2 < diff) {        /* DCO is too fast, slow it down */
    DCOCTL--;
    if(DCOCTL == 0xFF) {              /* Did DCO role under? */
      BCSCTL1--;
    }
  } else if (DELTA_2 > diff) {
    DCOCTL++;
    if(DCOCTL == 0x00) {              /* Did DCO role over? */
      BCSCTL1++;
    }
  }
}
/*---------------------------------------------------------------------------*/
