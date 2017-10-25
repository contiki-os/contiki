/**
 * \file
 *
 *   uart1 write routines
 *
 * \author
 *
 *   Anthony "Asterisk" Ambuehl
 *
 */
#include <stdlib.h>
#include <string.h>

#include "cc253x.h"
#include "sfr-bits.h"
#include "dev/uart1.h"

#if UART1_ENABLE
/*---------------------------------------------------------------------------*/
/* UART1 initialization */
void
uart1_init()
{
#ifdef UART1_ALTERNATIVE_1
  PERCFG &= ~PERCFG_U1CFG; /*alternative port 1 = P0.5-2*/
#ifdef UART1_RTSCTS
  P0SEL |= 0x3C;    /*peripheral select for TX and RX, RTS, CTS*/
#else
  P0SEL |= 0x30;    /*peripheral select for TX and RX*/
  P0 &= ~0x08;    /*RTS down*/
#endif
  P0DIR |= 0x18;    /*RTS, TX out*/
  P0DIR &= ~0x24;   /*CTS, RX in*/
#else
  PERCFG |= PERCFG_U1CFG;  /*alternative port 2 = P1.7-4*/
#ifdef UART1_RTSCTS
  P1SEL |= 0xF0;    /*peripheral select for TX and RX*/
#else
  P1SEL |= 0xC0;    /*peripheral select for TX and RX*/
  P1 &= ~0x20;    /*RTS down*/
#endif
  P1DIR |= 0x60;    /*RTS, TX out*/
  P1DIR &= ~0x90;   /*CTS, RX in*/
#endif

#if UART_ONE_CONF_HIGH_SPEED
  UART_SET_SPEED(1, UART_460_M, UART_460_E);
#else
  UART_SET_SPEED(1, UART_115_M, UART_115_E);
#endif

#ifdef UART1_RTSCTS
  U1UCR = 0x42; /*defaults: 8N1, RTS/CTS, high stop bit*/
#else
  U1UCR = 0x02; /*defaults: 8N1, no flow control, high stop bit*/
#endif

  U1CSR = UCSR_MODE; /* UART mode */
  U1UCR |= 0x80; /* Flush */

  UART1_RX_INT(1);
  U0DBUF = 0;
}
/*---------------------------------------------------------------------------*/
/* Write one byte over the UART. */
void
uart1_writeb(uint8_t byte)
{
  UTX1IF = 0;
  U1DBUF = byte;
  while(!UTX1IF); /* Wait until byte has been transmitted. */
  UTX1IF = 0;
}
/*---------------------------------------------------------------------------*/
#endif
