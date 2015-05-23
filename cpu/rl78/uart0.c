/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Maxim Salov <max.salov@gmail.com>, Ian Martin <martini@redwirellc.com>
 */

#include "rl78.h"     /* for f_CLK */
#include "sfrs.h"
#include "sfrs-ext.h"

#include "uart0.h"

#define DESIRED_BAUDRATE 38400

/* Note that only 9600, 38400, and 115200 bps were tested. */
#define PRESCALE_THRESH  ((38400 + 115200) / 2)
#define PRS_VALUE        ((DESIRED_BAUDRATE < PRESCALE_THRESH) ? 4 : 0)
#define f_MCK            (f_CLK / (1 << PRS_VALUE))
#define SDR_VALUE        (f_MCK / DESIRED_BAUDRATE / 2 - 1)

void
uart0_init(void)
{
  /* Reference R01AN0459EJ0100 or hardware manual for details */
  PIOR = 0U;                                           /* Disable IO redirection */
  PM1 |= 0x06U;                                         /* Set P11 and P12 as inputs */
  SAU0EN = 1;                                               /* Supply clock to serial array unit 0 */
  SPS0 = (PRS_VALUE << 4) | PRS_VALUE;                  /* Set input clock (CK00 and CK01) to fclk/16 = 2MHz */
  ST0 = 0x03U;                                          /* Stop operation of channel 0 and 1 */
  /* Setup interrupts (disable) */
  STMK0 = 1;                                                /* Disable INTST0 interrupt */
  STIF0 = 0;                                                /* Clear INTST0 interrupt request flag */
  STPR10 = 1;                                               /* Set INTST0 priority: lowest  */
  STPR00 = 1;
  SRMK0 = 1;                                                /* Disable INTSR0 interrupt */
  SRIF0 = 0;                                                /* Clear INTSR0 interrupt request flag */
  SRPR10 = 1;                                               /* Set INTSR0 priority: lowest */
  SRPR00 = 1;
  SREMK0 = 1;                                               /* Disable INTSRE0 interrupt */
  SREIF0 = 0;                                               /* Clear INTSRE0 interrupt request flag */
  SREPR10 = 1;                                              /* Set INTSRE0 priority: lowest */
  SREPR00 = 1;
  /* Setup operation mode for transmitter (channel 0) */
  SMR00 = 0x0023U;                                    /* Operation clock : CK00,
                                                               Transfer clock : division of CK00
                                                               Start trigger : software
                                                               Detect falling edge as start bit
                                                               Operation mode : UART
                                                               Interrupt source : buffer empty
                                                       */
  SCR00 = 0x8097U;                                    /* Transmission only
                                                               Reception error interrupt masked
                                                               Phase clock : type 1
                                                               No parity
                                                               LSB first
                                                               1 stop bit
                                                               8-bit data length
                                                       */
  SDR00 = SDR_VALUE << 9;
  /* Setup operation mode for receiver (channel 1) */
  NFEN0 |= 1;                                         /* Enable noise filter on RxD0 pin */
  SIR01 = 0x0007U;                                    /* Clear error flags */
  SMR01 = 0x0122U;                                    /* Operation clock : CK00
                                                               Transfer clock : division of CK00
                                                               Start trigger : valid edge on RxD pin
                                                               Detect falling edge as start bit
                                                               Operation mode : UART
                                                               Interrupt source : transfer end
                                                       */
  SCR01 = 0x4097U;                                    /* Reception only
                                                               Reception error interrupt masked
                                                               Phase clock : type 1
                                                               No parity
                                                               LSB first
                                                               1 stop bit
                                                               8-bit data length
                                                       */
  SDR01 = SDR_VALUE << 9;
  SO0 |= 1;                                             /* Prepare for use of channel 0 */
  SOE0 |= 1;
  P1 |= (1 << 2);                                        /* Set TxD0 high */
  PM1 &= ~(1 << 2);                                      /* Set output mode for TxD0 */
  PM1 |= (1 << 1);                                       /* Set input mode for RxD0 */
  SS0 |= 0x03U;                                         /* Enable UART0 operation (both channels) */
  STIF0 = 1;                                                /* Set buffer empty interrupt request flag */
}
void
uart0_putchar(int c)
{
  while(0 == STIF0) ;
  STIF0 = 0;
  SDR00 = c;
}
char
uart0_getchar(void)
{
  char c;
  while(!uart0_can_getchar()) ;
  c = SDR01;
  SRIF0 = 0;
  return c;
}
int
uart0_puts(const char *s)
{
  int len = 0;
  SMR00 |= 0x0001U;                                   /* Set buffer empty interrupt */
  while('\0' != *s) {
    uart0_putchar(*s);
    s++;
    ++len;
  }
#if 0
  while(0 == STIF0) ;
  STIF0 = 0;
  SDR00.sdr00 = '\r';
#endif
  SMR00 &= ~0x0001U;
  uart0_putchar('\n');
#if 0
  while(0 != SSR00.BIT.bit6) ;                              /* Wait until TSF00 == 0 */
#endif
  return len;
}
