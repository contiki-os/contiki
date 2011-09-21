/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

#include "contiki.h"

/*
 * This is SPI initialization code for the MSP430X architecture.
 *
 */

unsigned char spi_busy = 0;

/*
 * Initialize SPI bus.
 */
void
spi_init(void)
{
  UCB0CTL1 |= UCSWRST;                          // Put state machine in reset
  UCB0CTL1 = UCSSEL0 | UCSSEL1;                 // Select ACLK
  UCB0CTL0 |=  UCCKPH | UCSYNC | UCMSB | UCMST; // 3-pin, 8-bit SPI master, rising edge capture

  // 16 bit baud rate register
  UCB0BR0 = 0x00;                         // MSB => 0
  UCB0BR1 = 0x08;                         // LSB => SMCLK / (UCxxBR0 + UCxxBR1 × 256)

  // Set MOSI and SCLK as OUT and MISO as IN ports
  P3SEL |= ( BIT1 + BIT2 + BIT3 );        // Port3 = SPI peripheral
  P3DIR |= ( BIT1 + BIT3 );               //  MOSI and SCLK as Output
  P3DIR &= ~BIT2;                         //  Don't forget to configure MISO as Input
  P3DIR |= BIT0;

  UCB0CTL1 &= ~UCSWRST;                   // Initialize USCI state machine
}
