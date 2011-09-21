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

#if DCOSYNCH_CONF_ENABLED
/* dco_required set to 1 will cause the CPU not to go into
   sleep modes where the DCO clock stopped */
int msp430_dco_required;
#endif /* DCOSYNCH_CONF_ENABLED */

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
#if CONTIKI_TARGET_WISMOTE && defined(__IAR_SYSTEMS_ICC__)
  /* set to 8 MHz 244 * 32768 ? */
  /* set to 8 MHz 244 * 32768  (or 0x107a)? */
  __bis_SR_register(SCG0);
  UCSCTL0 = 0x0000;
  UCSCTL1 = DCORSEL_4;

  UCSCTL2 = 244 * 2; //0x107a;
  UCSCTL4 = 0x33; /* instead of 0x44 that is DCO/2 */
  __bic_SR_register(SCG0);
#elif CONTIKI_TARGET_WISMOTE
  // Stop watchdog
  WDTCTL = WDTPW + WDTHOLD;

  /** Configure XTAL **/
  P7SEL |= BIT0 + BIT1;   // Activate XT1
  UCSCTL6 &= ~XT1OFF;     // Set XT1 On

  UCSCTL6 |= XT1DRIVE_2 | XTS | XT2OFF;     // Max drive strength, adjust
  UCSCTL6 &= ~XT1DRIVE_1;

  do {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
    // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

  UCSCTL2 = FLLD0 + FLLD2;
  UCSCTL5 |= DIVA__2 + DIVS__2+ DIVM__2;//DIVPA__32 + DIVA__32 + DIVS__2+ DIVM__2;
  UCSCTL4 = SELA__DCOCLKDIV + SELS__XT1CLK + SELM__XT1CLK;       // Set MCLCK = XT1/2 , SMCLK = XT1/2 , ACLK = XT1/2

#else
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

  CCTL2 = CCIS0 + CM0 + CAP;            // Define CCR2, CAP, ACLK
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

#endif
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
#if defined(__MSP430__) && defined(__GNUC__)
extern int _end;		/* Not in sys/unistd.h */
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
#if DCOSYNCH_CONF_ENABLED
  if(req <= MSP430_REQUIRE_LPM1) {
    msp430_dco_required++;
  }
#endif /* DCOSYNCH_CONF_ENABLED */
}

void
msp430_remove_lpm_req(int req)
{
#if DCOSYNCH_CONF_ENABLED
  if(req <= MSP430_REQUIRE_LPM1) {
    msp430_dco_required--;
  }
#endif /* DCOSYNCH_CONF_ENABLED */
}

void
msp430_cpu_init(void)
{
  dint();
  watchdog_init();
  init_ports();
  msp430_init_dco();
  eint();
#if defined(__MSP430__) && defined(__GNUC__)
  if((uintptr_t)cur_break & 1) { /* Workaround for msp430-ld bug! */
    cur_break++;
  }
#endif

#if DCOSYNCH_CONF_ENABLED
  msp430_dco_required = 0;
#endif /* DCOSYNCH_CONF_ENABLED */
}
/*---------------------------------------------------------------------------*/

#define STACK_EXTRA 32
#define asmv(arg) __asm__ __volatile__(arg)

/*
 * Allocate memory from the heap. Check that we don't collide with the
 * stack right now (some other routine might later). A watchdog might
 * be used to check if cur_break and the stack pointer meet during
 * runtime.
 */
#if defined(__MSP430__) && defined(__GNUC__)
#define asmv(arg) __asm__ __volatile__(arg)
void *
sbrk(int incr)
{
  char *stack_pointer;

  asmv("mov r1, %0" : "=r" (stack_pointer));
  stack_pointer -= STACK_EXTRA;
  if(incr > (stack_pointer - cur_break))
    return (void *)-1;		/* ENOMEM */

  void *old_break = cur_break;
  cur_break += incr;
  /*
   * If the stack was never here then [old_break .. cur_break] should
   * be filled with zeros.
  */
  return old_break;
}
#endif
/*---------------------------------------------------------------------------*/
/*
 * Mask all interrupts that can be masked.
 */
int
splhigh_(void)
{
  int sr;
  /* Clear the GIE (General Interrupt Enable) flag. */
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
int __low_level_init(void)
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
#if DCOSYNCH_CONF_ENABLED
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

  // wait for next Capture
  TBCCTL6 &= ~CCIFG;
  while(!(TBCCTL6 & CCIFG));
  last = TBCCR6;

  TBCCTL6 &= ~CCIFG;
  // wait for next Capture - and calculate difference
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
#endif /* DCOSYNCH_CONF_ENABLED */
/*---------------------------------------------------------------------------*/
