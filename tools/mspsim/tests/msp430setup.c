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
 * $Id: msp430setup.c,v 1.2 2007/01/22 15:45:04 nfi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2007-01-22
 * Updated : $Date: 2007/01/22 15:45:04 $
 *           $Revision: 1.2 $
 */

#include "msp430setup.h"
#if __MSPGCC__
#include <msp430.h>
#include <msp430libc.h>
#include <legacymsp430.h>
#include <sys/crtld.h>
#else /* __MSPGCC__ */
#include <io.h>
#include <signal.h>
#include <sys/unistd.h>
#endif /* __MSPGCC__ */
#include <stdio.h>


/*--------------------------------------------------------------------------*/
/* RS232 Interface */
/*--------------------------------------------------------------------------*/

static int (* input_handler)(unsigned char) = NULL;

/*---------------------------------------------------------------------------*/
interrupt(UART1RX_VECTOR)
     rs232_rx_usart1(void)
{
  /* Check status register for receive errors. - before reading RXBUF since
     it clears the error and interrupt flags */
  if(!(URCTL1 & RXERR) && input_handler != NULL) {
    input_handler(RXBUF1);

  } else {
    /* Else read out the char to clear the I-flags, etc. */
    RXBUF1;
  }
}
/*--------------------------------------------------------------------------*/
void
rs232_init(void)
{

  /* RS232 */
  UCTL1 = CHAR;                         /* 8-bit character */
  UTCTL1 = SSEL1;                       /* UCLK = MCLK */

  rs232_set_speed(RS232_57600);

  input_handler = NULL;

  ME2 |= (UTXE1 | URXE1);                 /* Enable USART1 TXD/RXD */
  IE2 |= URXIE1;                        /* Enable USART1 RX interrupt  */
}
/*---------------------------------------------------------------------------*/
void
rs232_send(char c)
{
  /* Loop until the transmission buffer is available. */
  while ((IFG2 & UTXIFG1) == 0);

  /* Transmit the data. */
  TXBUF1 = c;
}
/*---------------------------------------------------------------------------*/
void
rs232_set_speed(unsigned char speed)
{
  if(speed == RS232_19200) {
    /* Set RS232 to 19200 */
    UBR01 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x00;                        /* no modulation  */
  } else if(speed == RS232_38400) {
    /* Set RS232 to 38400 */
    UBR01 = 0x40;                         /* 2,457MHz/38400 = 64 -> 0x40 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x00;                        /* no modulation  */
  } else if(speed == RS232_57600) {
    UBR01 = 0x2a;                         /* 2,457MHz/57600 = 42.7 -> 0x2A */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x5b;                        /* */
  } else if(speed == RS232_115200) {
    UBR01 = 0x15;                         /* 2,457MHz/115200 = 21.4 -> 0x15 */
    UBR11 = 0x00;                         /* */
    UMCTL1 = 0x4a;                        /* */
  } else {
    rs232_set_speed(RS232_57600);
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_set_input(int (*f)(unsigned char))
{
  input_handler = f;
}
/*--------------------------------------------------------------------------*/
int
putchar(int c)
{
  rs232_send(c);
  return c;
}
/*--------------------------------------------------------------------------*/




/*---------------------------------------------------------------------------*/
void
esb_sensors_init(void)
{
  P5SEL &= ~(1 << 5);
  P5DIR |= (1 << 5);
}
/*---------------------------------------------------------------------------*/
void
esb_sensors_on(void)
{
  P5OUT &= ~(1 << 5);
}
/*---------------------------------------------------------------------------*/
void
esb_sensors_off(void)
{
  P5OUT |= (1 << 5);
}
/*---------------------------------------------------------------------------*/




/*---------------------------------------------------------------------------*/
/* CPU INIT */
/*--------------------------------------------------------------------------*/

static void
msp430_init_dco(void)
{
    /* This code taken from the FU Berlin sources and reformatted. */

#define DELTA    600

  unsigned int compare, oldcapture = 0;
  unsigned int i;


  BCSCTL1 = 0xa4; /* ACLK is devided by 4. RSEL=6 no division for MCLK
		     and SSMCLK. XT2 is off. */

  BCSCTL2 = 0x00; /* Init FLL to desired frequency using the 32762Hz
		     crystal DCO frquenzy = 2,4576 MHz  */

  WDTCTL = WDTPW + WDTHOLD;             /* Stop WDT */
  BCSCTL1 |= DIVA1 + DIVA0;             /* ACLK = LFXT1CLK/8 */
  for(i = 0xffff; i > 0; i--);          /* Delay for XTAL to settle */

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
}
/*---------------------------------------------------------------------------*/
static void
init_ports(void)
{
  /* Turn everything off, device drivers are supposed to enable what is
   * really needed!
   */

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
static void init_ports_toberemoved() {
    ////////// Port 1 ////
  P1SEL = 0x00;
  P1DIR = 0x81;       // Outputs: P10=IRSend, P17=RS232RTS
                      // Inputs: P11=Light, P12=IRRec, P13=PIR, P14=Vibration,
                      //         P15=Clockalarm, P16=RS232CTS
  P1OUT = 0x00;

  ////////// Port 2 ////
  P2SEL = 0x00;       // No Sels
  P2DIR = 0x7F;       // Outpus: P20..P23=Leds+Beeper, P24..P26=Poti
                      // Inputs: P27=Taster
  P2OUT = 0x77;

  ////////// Port 3 ////
  P3SEL = 0xE0;       // Sels for P34..P37 to activate UART,
  P3DIR = 0x5F;       // Inputs: P30..P33=CON4, P35/P37=RXD Transceiver/RS232
                      // OutPuts: P36/P38=TXD Transceiver/RS232
  P3OUT = 0xE0;       // Output a Zero on P34(TXD Transceiver) and turn SELECT off when receiving!!!

  ////////// Port 4 ////
  P4SEL = 0x00;       // CON5 Stecker
  P4DIR = 0xFF;
  P4OUT = 0x00;

  ////////// Port 5 ////
  P5SEL = 0x00;       // P50/P51= Clock SDA/SCL, P52/P53/P54=EEPROM SDA/SCL/WP
  P5DIR = 0xDA;       // P56/P57=Transceiver CNTRL0/1
  P5OUT = 0x0F;

  ////////// Port 6 ////
  P6SEL = 0x00;       // P60=Microphone, P61=PIR digital (same as P13), P62=PIR analog
  P6DIR = 0x00;       // P63=extern voltage, P64=battery voltage, P65=Receive power
  P6OUT = 0x00;
}
/*--------------------------------------------------------------------------*/
void
msp430_setup(void)
{
  dint();
  init_ports();
  msp430_init_dco();
  eint();

  init_ports_toberemoved();

  esb_sensors_init();
  esb_sensors_on();

  rs232_init();
}

#define asmv(arg) __asm__ __volatile__(arg)

#define STACK_EXTRA 32

static char *cur_break = (char *)(
#if defined(__MSP430_LIBC__) && 20110612 <= __MSP430_LIBC__
				  __bss_end
#else
				  &__bss_end + 1
#endif
);

/*
 * Allocate memory from the heap. Check that we don't collide with the
 * stack right now (some other routine might later). A watchdog might
 * be used to check if cur_break and the stack pointer meet during
 * runtime.
 */
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

/*
 * Mask all interrupts that can be masked.
 */
int
splhigh_(void)
{
  /* Clear the GIE (General Interrupt Enable) flag. */
  int sr;
  asmv("mov r2, %0" : "=r" (sr));
  asmv("bic %0, r2" : : "i" (GIE));
  return sr & GIE;		/* Ignore other sr bits. */
}

/*
 * Restore previous interrupt mask.
 */
void
splx_(int sr)
{
  /* If GIE was set, restore it. */
  asmv("bis %0, r2" : : "r" (sr));
}
