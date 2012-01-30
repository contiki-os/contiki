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

/* Currently only the STK500 platform uses a static RAM buffer for printfs.
 * Others print a character at a time using the gcc string pointer.
 * gcc may not strip the unused buffer, even though it is statically
 * allocated in an unused routine.
 */
#ifdef RS232_CONF_PRINTF_BUFFER_LENGTH
#define RS232_PRINTF_BUFFER_LENGTH RS232_CONF_PRINTF_BUFFER_LENGTH
#else
#if CONTIKI_TARGET_STK500
#define RS232_PRINTF_BUFFER_LENGTH 64
#endif
#endif

/* TX interrupts would allow non-blocking output up to the size of some RAM buffer.
 * Since a RAM buffer is not implemented tx interrupts are superfluous and unwanted
 * because they block debug prints from within interrupt routines
 */
#ifdef RS232_CONF_TX_INTERRUPTS
#define RS232_TX_INTERRUPTS RS232_CONF_TX_INTERRUPTS
#else
#define RS232_TX_INTERRUPTS 0
#endif

/* Insert a carriage return after a line feed. This is the default. */
#ifndef ADD_CARRIAGE_RETURN_AFTER_NEWLINE
#define ADD_CARRIAGE_RETURN_AFTER_NEWLINE 1
#endif

/* Reducing NUMPORTS from the default will harmlessly disable usage of those ports */
/* Two ports take 400 bytes flash and 4 bytes RAM. */
#ifdef RS232_CONF_NUMPORTS
#define NUMPORTS RS232_CONF_NUMPORTS
#endif

#if defined (__AVR_ATmega128__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega128RFA1__)
#ifndef NUMPORTS
#define NUMPORTS 2
#elif NUMPORTS > 2
#error Only two serial ports are defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR0
#define D_UDRE0M (1 << UDRE0)
#define D_UBRR0H UBRR0H
#define D_UBRR0L UBRR0L
#define D_UCSR0A UCSR0A
#define D_UCSR0B UCSR0B
#define D_UCSR0C UCSR0C
#define D_USART0_RX_vect USART0_RX_vect
#define D_USART0_TX_vect USART0_TX_vect

#if NUMPORTS > 1
#define D_UDR1   UDR1
#define D_UDRE1M (1 << UDRE1)
#define D_UBRR1H UBRR1H
#define D_UBRR1L UBRR1L
#define D_UCSR1A UCSR1A
#define D_UCSR1B UCSR1B
#define D_UCSR1C UCSR1C
#define D_USART1_RX_vect USART1_RX_vect
#define D_USART1_TX_vect USART1_TX_vect
#endif

#endif

#elif defined (__AVR_AT90USB1287__)
/* Has only UART1, map it to port 0 */
#ifndef NUMPORTS
#define NUMPORTS 1
#elif NUMPORTS > 1
#error Only one serial port is defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR1
#define D_UDRE0M (1 << UDRE1)
#define D_UBRR0H UBRR1H
#define D_UBRR0L UBRR1L
#define D_UCSR0A UCSR1A
#define D_UCSR0B UCSR1B
#define D_UCSR0C UCSR1C
#define D_USART0_RX_vect USART1_RX_vect
#define D_USART0_TX_vect USART1_TX_vect
#endif

#elif defined (__AVR_ATmega8515__)
#ifndef NUMPORTS
#define NUMPORTS 1
#elif NUMPORTS > 1
#error Only one serial port is defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR
#define D_UDRE0M (1 << UDRE)
#define D_UBRR0H UBRRH
#define D_UBRR0L UBRRL
#define D_UCSR0A UCSRA
#define D_UCSR0B UCSRB
#define D_UCSR0C UCSRC
#define D_USART0_RX_vect USART_RX_vect
#define D_USART0_TX_vect USART_TX_vect
#endif

#elif defined (__AVR_ATmega328P__)
#ifndef NUMPORTS
#define NUMPORTS 1
#elif NUMPORTS > 1
#error Only one serial port is defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR0
#define D_UDRE0M (1 << UDRE0)
#define D_UBRR0H UBRR0H
#define D_UBRR0L UBRR0L
#define D_UCSR0A UCSR0A
#define D_UCSR0B UCSR0B
#define D_UCSR0C UCSR0C
#define D_USART0_RX_vect USART_RX_vect
#define D_USART0_TX_vect USART_TX_vect
#endif

#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
#ifndef NUMPORTS
#define NUMPORTS 1
#elif NUMPORTS > 1
#error Only one serial port is defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR
#define D_UDRE0M (1 << UDRE)
#define D_UBRR0H UBRRH
#define D_UBRR0L UBRRL
#define D_UCSR0A UCSRA
#define D_UCSR0B UCSRB
#define D_UCSR0C UCSRC
#define D_USART0_RX_vect USART_RXC_vect
#define D_USART0_TX_vect USART_TXC_vect
#endif

#elif defined (__AVR_ATmega644__)
#ifndef NUMPORTS
#define NUMPORTS 1
#elif NUMPORTS > 1
#error Only one serial port is defined for this processor!
#endif

#if NUMPORTS > 0
#define D_UDR0   UDR0
#define D_UDRE0M (1 << UDRE0)
#define D_UBRR0H UBRR0H
#define D_UBRR0L UBRR0L
#define D_UCSR0A UCSR0A
#define D_UCSR0B UCSR0B
#define D_UCSR0C UCSR0C
#define D_USART0_RX_vect USART0_RX_vect
#define D_USART0_TX_vect USART0_TX_vect
#endif

#else
#error Please define the UART registers for your MCU!
#endif

#if NUMPORTS > 0
int (* input_handler_0)(unsigned char);
ISR(D_USART0_RX_vect)
{
  unsigned char c;
  c = D_UDR0;
  if (input_handler_0 != NULL) input_handler_0(c);
}
#if RS232_TX_INTERRUPTS
volatile uint8_t txwait_0;
ISR(D_USART0_TX_vect)
{
  txwait_0 = 0;
}
#endif

#if NUMPORTS > 1
int (* input_handler_1)(unsigned char);
ISR(D_USART1_RX_vect)
{
  unsigned char c;
  c = D_UDR1;
  if (input_handler_1 != NULL) input_handler_1(c);
}
#if RS232_TX_INTERRUPTS
volatile uint8_t txwait_1;
ISR(USART1_TX_vect)
{
  txwait_1 = 0;
}
#endif

#if NUMPORTS > 2
int (* input_handler_2)(unsigned char);
ISR(D_USART2_RX_vect)
{
  unsigned char c;
  c = D_UDR2;
  if (input_handler_2 != NULL) input_handler_2(c);
}
#if RS232_TX_INTERRUPTS
volatile uint8_t txwait_2;
ISR(USART2_TX_vect)
{
  txwait_2= 0;
}
#endif
#endif

#endif
#endif
/*---------------------------------------------------------------------------*/
void
rs232_init (uint8_t port, uint8_t bd, uint8_t ffmt)
{
#if NUMPORTS > 0
 if (port == 0) {
   D_UBRR0H = (uint8_t)(bd>>8);
   D_UBRR0L = (uint8_t)bd;
#if RS232_TX_INTERRUPTS
   txwait_0 = 0;
   D_UCSR0B =  USART_INTERRUPT_RX_COMPLETE | USART_INTERRUPT_TX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#else
   D_UCSR0B =  USART_INTERRUPT_RX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#endif
   D_UCSR0C = USART_UCSRC_SEL | ffmt;
   input_handler_0 = NULL;

#if NUMPORTS > 1
 } else if (port == 1) {
   D_UBRR1H = (uint8_t)(bd>>8);
   D_UBRR1L = (uint8_t)bd;
#if RS232_TX_INTERRUPTS
   txwait_1 = 0;
   D_UCSR1B =  USART_INTERRUPT_RX_COMPLETE | USART_INTERRUPT_TX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#else
   D_UCSR1B =  USART_INTERRUPT_RX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#endif
   D_UCSR1C = USART_UCSRC_SEL | ffmt;
   input_handler_1 = NULL;

#if NUMPORTS > 2
 } else if (port == 2) {
   D_UBRR2H = (uint8_t)(bd>>8);
   D_UBRR2L = (uint8_t)bd;
#if RS232_TX_INTERRUPTS
   txwait_2 = 0;
   D_UCSR2B =  USART_INTERRUPT_RX_COMPLETE | USART_INTERRUPT_TX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#else
   D_UCSR2B =  USART_INTERRUPT_RX_COMPLETE | \
               USART_RECEIVER_ENABLE | USART_TRANSMITTER_ENABLE;
#endif
   D_UCSR2C = USART_UCSRC_SEL | ffmt;
   input_handler_2 = NULL;
#endif
#endif
 }
#endif /* NUMPORTS > 0 */
}

/*---------------------------------------------------------------------------*/
void 
rs232_send(uint8_t port, unsigned char c)
{
#if RS232_TX_INTERRUPTS
  /* Output character and block until it is transmitted */
#if NUMPORTS > 0
  if (port == 0 ) {
    txwait_0 = 1;
    D_UDR0 = c;
    while (txwait_0);
#if NUMPORTS > 1
  } else if (port == 1) {
    txwait_1 = 1;
    D_UDR1 = c;
    while (txwait_1);
#if NUMPORTS > 2
  } else if (port == 2) {
    txwait_2 = 1;
    D_UDR2 = c;
    while (txwait_2);
#endif
#endif
  }
#endif
#else /* RS232_TX_INTERRUPTS */
  /* Block until tx ready and output character */
#if NUMPORTS > 0
  if (port == 0 ) {
    while (!(D_UCSR0A & D_UDRE0M));
    D_UDR0 = c;
#if NUMPORTS > 1
  } else if (port == 1) {
    while (!(D_UCSR1A & D_UDRE1M));
    D_UDR1 = c;
#if NUMPORTS > 2
  } else if (port == 2) {
    while (!(D_UCSR2A & D_UDRE2M));
    D_UDR2 = c;
#endif
#endif
  }
#endif
#endif /* RS232_TX_INTERRUPTS */
}
/*---------------------------------------------------------------------------*/
void
rs232_set_input(uint8_t port, int (*f)(unsigned char))
{
#if NUMPORTS > 0
  if (port == 0) {
    input_handler_0 = f;
#if NUMPORTS > 1
  } else if (port == 1) {
    input_handler_1 = f;
#if NUMPORTS > 2
  } else if (port == 2) {
    input_handler_2 = f;
#endif
#endif
  }
#endif
}

/*---------------------------------------------------------------------------*/
void
rs232_print(uint8_t port, char *buf)
{
  while(*buf) {
#if ADD_CARRIAGE_RETURN_AFTER_NEWLINE
    if(*buf=='\n') rs232_send(port, '\r');
	if(*buf=='\r') buf++; else rs232_send(port, *buf++);
#else
    rs232_send(port, *buf++);
#endif
  }
}

#if RS232_PRINTF_BUFFER_LENGTH
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
#endif
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
#if ADD_CARRIAGE_RETURN_AFTER_NEWLINE
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
