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
 */

#include "contiki.h"
#include "dev/watchdog.h"

/* dco_required set to 1 will cause the CPU not to go into
 *    sleep modes where the DCO clock stopped */
int msp430_dco_required;

#if defined(__MSP430__) && defined(__GNUC__)
#define asmv(arg) __asm__ __volatile__ (arg)
#endif

/*---------------------------------------------------------------------------*/
#if defined(__MSP430__) && defined(__GNUC__) && MSP430_MEMCPY_WORKAROUND
void *
w_memcpy(void *out, const void *in, size_t n)
{
  uint8_t *src, *dest;
  src = (uint8_t *)in;
  dest = (uint8_t *)out;
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
  dest = (uint8_t *)out;
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

#ifdef P7DIR
  P7DIR = 0;
  P7OUT = 0;
#endif

#ifdef P8DIR
  P8DIR = 0;
  P8OUT = 0;
#endif

  P1IE = 0;
  P2IE = 0;
}
/*---------------------------------------------------------------------------*/
/* msp430-ld may align _end incorrectly. Workaround in cpu_init. */
#if defined(__MSP430__) && defined(__GNUC__)
extern int _end;    /* Not in sys/unistd.h */
static char *cur_break = (char *)&_end;
#endif

/*---------------------------------------------------------------------------*/
/* add/remove_lpm_req - for requiring a specific LPM mode. currently Contiki */
/* jumps to LPM3 to save power, but DMA will not work if DCO is not clocked  */
/* so some modules might need to enter their LPM requirements                */
/* NOTE: currently only works with LPM1 (e.g. DCO) requirements.             */
/*---------------------------------------------------------------------------*/
void
msp430_add_lpm_req(int req)
{
  if(req <= MSP430_REQUIRE_LPM1) {
    msp430_dco_required++;
  }
}
void
msp430_remove_lpm_req(int req)
{
  if(req <= MSP430_REQUIRE_LPM1) {
    msp430_dco_required--;
  }
}
void
msp430_cpu_init(void)
{
  dint();
  watchdog_init();
  init_ports();
  /* set DCO to a reasonable default value (8MHz) */
  msp430_init_dco();
  /* calibrate the DCO step-by-step */
  msp430_sync_dco();
  eint();
#if defined(__MSP430__) && defined(__GNUC__)
  if((uintptr_t)cur_break & 1) { /* Workaround for msp430-ld bug! */
    cur_break++;
  }
#endif
  msp430_dco_required = 0;
}
/*---------------------------------------------------------------------------*/

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
  return sr & GIE;    /* Ignore other sr bits. */
}
/*---------------------------------------------------------------------------*/
/*
 * Restore previous interrupt mask.
 */
/* void */
/* splx_(int sr) */
/* { */
/* #ifdef __IAR_SYSTEMS_ICC__ */
/*   __bis_SR_register(sr); */
/* #else */
/*   /\* If GIE was set, restore it. *\/ */
/*   asmv("bis %0, r2" : : "r" (sr)); */
/* #endif */
/* } */
/*---------------------------------------------------------------------------*/
#ifdef __IAR_SYSTEMS_ICC__
int
__low_level_init(void)
{
  /* turn off watchdog so that C-init will run */
  WDTCTL = WDTPW + WDTHOLD;
  /*
   * Return value:
   *
   *  1 - Perform data segment initialization.
   *  0 - Skip data segment initialization.
   */
  return 1;
}
#endif
/*---------------------------------------------------------------------------*/
void
msp430_sync_dco(void)
{
  uint16_t oldcapture;
  int16_t diff;
  /* DELTA_2 assumes an ACLK of 32768 Hz */
#define DELTA_2    ((MSP430_CPU_SPEED) / 32768)

  /* Select SMCLK clock, and capture on ACLK for TBCCR6 */
  TBCTL = TBSSEL1 | TBCLR;
  TBCCTL6 = CCIS0 + CM0 + CAP;
  /* start the timer */
  TBCTL |= MC1;

  while(1) {
    /* wait for the next capture */
    TBCCTL6 &= ~CCIFG;
    while(!(TBCCTL6 & CCIFG));
    oldcapture = TBCCR6;

    /* wait for the next capture - and calculate difference */
    TBCCTL6 &= ~CCIFG;
    while(!(TBCCTL6 & CCIFG));
    diff = TBCCR6 - oldcapture;

    /* resynchronize the DCO speed if not at target */
    if(DELTA_2 == diff) {
      break;                            /* if equal, leave "while(1)" */
    } else if(DELTA_2 < diff) {         /* DCO is too fast, slow it down */
      DCOCTL--;
      if(DCOCTL == 0xFF) {              /* Did DCO roll under? */
        BCSCTL1--;
      }
    } else {                            /* -> Select next lower RSEL */
      DCOCTL++;
      if(DCOCTL == 0x00) {              /* Did DCO roll over? */
        BCSCTL1++;
      }
      /* -> Select next higher RSEL  */
    }
  }

  /* Stop the timer - conserves energy according to user guide */
  TBCTL = 0;
}
/*---------------------------------------------------------------------------*/
