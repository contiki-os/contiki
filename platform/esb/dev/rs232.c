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

/** \addtogroup esbrs232
 * @{ */

/**
 * \file
 * RS232 communication device driver for the MSP430.
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file contains an RS232 device driver for the MSP430 microcontroller.
 *
 */

#include "contiki.h"
#include <string.h>

#include "contiki-esb.h"
#include "isr_compat.h"

static int (* input_handler)(unsigned char) = NULL;

/*---------------------------------------------------------------------------*/
ISR(UART1RX, rs232_rx_usart1)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* Check status register for receive errors. - before reading RXBUF since
     it clears the error and interrupt flags */
  if(!(URCTL1 & RXERR) && input_handler != NULL) {

    if(input_handler(RXBUF1)) {
      LPM4_EXIT;
    }
  } else {
    /* Else read out the char to clear the I-flags, etc. */
    RXBUF1;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 */
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

  ENERGEST_ON(ENERGEST_TYPE_SERIAL);
  /* Loop until the transmission buffer is available. */
  while((IFG2 & UTXIFG1) == 0) {
  }

  /* Transmit the data. */
  TXBUF1 = c;
  ENERGEST_OFF(ENERGEST_TYPE_SERIAL);
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
rs232_print(char *cptr)
{
  while(*cptr != 0) {
    rs232_send(*cptr);
    ++cptr;
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_set_input(int (*f)(unsigned char))
{
  input_handler = f;
}
/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  rs232_send(c);
}
/*---------------------------------------------------------------------------*/
/** @} */
