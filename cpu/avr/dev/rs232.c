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
 * @(#)$Id: rs232.c,v 1.7 2010/10/27 14:51:20 dak664 Exp $
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "contiki-conf.h"
#include "contiki.h"

#include "dev/slip.h"
#include "dev/rs232.h"

/*ATmega32 and smaller have UBRRH/UCSRC at the same I/O address.
 *USART_UCSRC_SEL (bit7) selects writing to UBRHH(0) or UCSRC(1).
 *It is OR'd in below so if not defined we can just set it to zero.
 */
#ifndef USART_UCSRC_SEL
#define USART_UCSRC_SEL 0x00
#endif

#ifdef RS232_CONF_PRINTF_BUFFER_LENGTH
#define RS232_PRINTF_BUFFER_LENGTH RS232_CONF_PRINTF_BUFFER_LENGTH
#else
#define RS232_PRINTF_BUFFER_LENGTH 64
#endif

#ifndef ADD_CARRAGE_RETURNS_TO_SERIAL_OUTPUT
#define ADD_CARRAGE_RETURNS_TO_SERIAL_OUTPUT 1
#endif

typedef struct {
  volatile uint8_t * udr;
  volatile uint8_t * ubrrh;
  volatile uint8_t * ubrrl;
  volatile uint8_t * ucsrb;
  volatile uint8_t * ucsrc;
  volatile uint8_t txwait;
  int (* input_handler)(unsigned char);
} rs232_t;

#if defined (__AVR_ATmega128__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega128RFA1__)
static rs232_t rs232_ports[2] = {
  {   // UART0
    &UDR0,
    &UBRR0H,
    &UBRR0L,
    &UCSR0B,
    &UCSR0C,
    0,
    NULL
  },

  {  // UART1
    &UDR1,
    &UBRR1H,
    &UBRR1L,
    &UCSR1B,
    &UCSR1C,
    0,
    NULL
  }
};
#elif defined (__AVR_AT90USB1287__)
/* Has only UART1, map it to port 0 */
static rs232_t rs232_ports[1] = {
  {  // UART1
    &UDR1,
    &UBRR1H,
    &UBRR1L,
    &UCSR1B,
    &UCSR1C,
    0,
    NULL
  }
};
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega8515__) \
  || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
static rs232_t rs232_ports[1] = {
  {   // UART0
    &UDR,
    &UBRRH,
    &UBRRL,
    &UCSRB,
    &UCSRC,
    0,
    NULL
  }
};
#elif defined (__AVR_ATmega644__) || defined (__AVR_ATmega328P__)
static rs232_t rs232_ports[1] = {
  {   // UART0
    &UDR0,
    &UBRR0H,
    &UBRR0L,
    &UCSR0B,
    &UCSR0C,
    0,
    NULL
  }
};
#else
#error Please define the UART registers for your MCU!
#endif

#if defined (__AVR_ATmega128__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega128RFA1__)
/*---------------------------------------------------------------------------*/
ISR(USART0_TX_vect)
{
  rs232_ports[RS232_PORT_0].txwait = 0;
}

/*---------------------------------------------------------------------------*/
ISR(USART0_RX_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_0].udr);

  if(rs232_ports[RS232_PORT_0].input_handler != NULL) {
    rs232_ports[RS232_PORT_0].input_handler(c);
  }
}
/*---------------------------------------------------------------------------*/
ISR(USART1_TX_vect)
{
  rs232_ports[RS232_PORT_1].txwait = 0;
}

/*---------------------------------------------------------------------------*/
ISR(USART1_RX_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_1].udr);

  if(rs232_ports[RS232_PORT_1].input_handler != NULL) {
    rs232_ports[RS232_PORT_1].input_handler(c);
  }
}

#elif defined (__AVR_ATmega644__)
/*---------------------------------------------------------------------------*/
ISR(USART0_TX_vect)
{
  rs232_ports[RS232_PORT_0].txwait = 0;
}
 /*---------------------------------------------------------------------------*/
ISR(USART0_RX_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_0].udr);

  if(rs232_ports[RS232_PORT_0].input_handler != NULL) {
    rs232_ports[RS232_PORT_0].input_handler(c);
   }
}
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
/*---------------------------------------------------------------------------*/
ISR(USART_TXC_vect)
{
  rs232_ports[RS232_PORT_0].txwait = 0;
}

/*---------------------------------------------------------------------------*/
ISR(USART_RXC_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_0].udr);

  if(rs232_ports[RS232_PORT_0].input_handler != NULL) {
    rs232_ports[RS232_PORT_0].input_handler(c);
  }
}

#elif defined (__AVR_ATmega8515__) || defined (__AVR_ATmega328P__)
/*---------------------------------------------------------------------------*/
ISR(USART_TX_vect)
{
  rs232_ports[RS232_PORT_0].txwait = 0;
}

/*---------------------------------------------------------------------------*/
ISR(USART_RX_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_0].udr);

  if(rs232_ports[RS232_PORT_0].input_handler != NULL) {
    rs232_ports[RS232_PORT_0].input_handler(c);
  }
}

#elif defined (__AVR_AT90USB1287__)
/*---------------------------------------------------------------------*/
ISR(USART1_TX_vect)
{
  rs232_ports[RS232_PORT_0].txwait = 0;
}

/*---------------------------------------------------------------------------*/
ISR(USART1_RX_vect)
{
  unsigned char c;

  c = *(rs232_ports[RS232_PORT_0].udr);

  if(rs232_ports[RS232_PORT_0].input_handler != NULL) {
    rs232_ports[RS232_PORT_0].input_handler(c);
  }
}
#else
#error Please define the interrupt vectors for your MCU!
#endif

/*---------------------------------------------------------------------------*/
void
rs232_init (uint8_t port, uint8_t bd, uint8_t ffmt)
{
  *(rs232_ports[port].ubrrh) = (uint8_t)(bd>>8);
  *(rs232_ports[port].ubrrl) = (uint8_t)bd;

  /*
   * - Enable receiver and transmitter,
   * - Enable interrupts for receiver and transmitter
   */
  *(rs232_ports[port].ucsrb) = USART_INTERRUPT_RX_COMPLETE | USART_INTERRUPT_TX_COMPLETE | \
    USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;

  /*
   * - mode (sync. / async)
   * - Parity
   * - Stop bits
   * - charater size (9 bits are currently not supported)
   * - clock polarity
   */
  *(rs232_ports[port].ucsrc) = USART_UCSRC_SEL | ffmt;

  rs232_ports[port].txwait = 0;

  rs232_ports[port].input_handler = NULL;
}

void
rs232_print_p(uint8_t port, prog_char *buf)
{
  while(pgm_read_byte(buf)) {
    rs232_send(port, pgm_read_byte(buf));
    ++buf;
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_print(uint8_t port, char *buf)
{
  while(*buf) {
#if ADD_CARRAGE_RETURNS_TO_SERIAL_OUTPUT
    if(*buf=='\n') rs232_send(port, '\r');
	if(*buf=='\r') buf++; else rs232_send(port, *buf++);
#else
    rs232_send(port, *buf++);
#endif
  }
}
/*---------------------------------------------------------------------------*/
void
rs232_printf(uint8_t port, const char *fmt, ...)
{
  va_list ap;
  static char buf[RS232_PRINTF_BUFFER_LENGTH];

  va_start (ap, fmt);
  vsnprintf (buf, RS232_PRINTF_BUFFER_LENGTH, fmt, ap);
  va_end(ap);

  rs232_print (port, buf);
}
/*---------------------------------------------------------------------------*/
void
rs232_send(uint8_t port, unsigned char c)
{
  rs232_ports[port].txwait = 1;
  *(rs232_ports[port].udr) = c;
  while(rs232_ports[port].txwait);
}
/*---------------------------------------------------------------------------*/
void
rs232_set_input(uint8_t port, int (*f)(unsigned char))
{
  rs232_ports[port].input_handler = f;
}
/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  rs232_send(SLIP_PORT, c);
}
/*---------------------------------------------------------------------------*/
int rs232_stdout_putchar(char c, FILE *stream);
static uint8_t stdout_rs232_port=RS232_PORT_0;
static FILE rs232_stdout = FDEV_SETUP_STREAM(rs232_stdout_putchar,
					     NULL,
					     _FDEV_SETUP_WRITE);

int rs232_stdout_putchar(char c, FILE *stream)
{
#if ADD_CARRAGE_RETURNS_TO_SERIAL_OUTPUT
  if(c=='\n') rs232_send(stdout_rs232_port, '\r');
  if(c!='\r') rs232_send (stdout_rs232_port, c);
#else
  rs232_send (stdout_rs232_port, c);
#endif
  return 0;
}
/*---------------------------------------------------------------------------*/
void rs232_redirect_stdout (uint8_t port) {
  stdout_rs232_port = port;
  stdout = &rs232_stdout;
}
