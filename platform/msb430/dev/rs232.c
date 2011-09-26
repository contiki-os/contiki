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
 * @(#)$Id: rs232.c,v 1.9 2009/06/29 12:46:50 nvt-se Exp $
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
#include "dev/msb430-uart1.h"
#include "rs232.h"

#ifndef U1IFG
#define U1IFG	IFG2
#endif

/*---------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 */
void
rs232_init(void)
{
  rs232_set_speed(RS232_115200);
}
/*---------------------------------------------------------------------------*/
int
putchar(int c)
{
  if(uart_get_mode() == UART_MODE_RS232) {
    /* Loop until the transmission buffer is available. */
    UART_WAIT_TX();
    /* Transmit the data. */
    UART_TX = c;
    return c;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_send(char c)
{
  /* Check if the UART is in RS232 mode before sending.
     This check can be ommitted if every access to rs232 locks the uart
     before using it.
  */

  putchar(c);
}
/*---------------------------------------------------------------------------*/
void
rs232_set_speed(enum rs232_speed speed)
{
  // baud
  const unsigned char br_table[5][3] = {
        {0x00, 0x01, 0x00},             // 9600
	{0x80, 0x00, 0x00},		// 19200
	{0x40, 0x00, 0x00},		// 38400
	{0x2a, 0x00, 0x5b},		// 57600
	{0x15, 0x00, 0x4a}		// 115200
  };

  uart_set_speed(UART_MODE_RS232, br_table[speed][0], 
	br_table[speed][1], br_table[speed][2]);
}
/*---------------------------------------------------------------------------*/
void
rs232_print(char *cptr)
{
  /* lock UART for the print operation */
  if(uart_lock(UART_MODE_RS232)) {
    while(*cptr != 0) {
      rs232_send(*cptr);
      ++cptr;
    }
    uart_unlock(UART_MODE_RS232);
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_set_input(uart_handler_t f)
{
  uart_set_handler(UART_MODE_RS232, f);
}
/** @} */
