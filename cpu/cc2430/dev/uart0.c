/**
 * \file
 *
 *   uart0 write routines
 *
 * \author
 *
 *   Anthony "Asterisk" Ambuehl
 *
 */
#include <stdlib.h>
#include <string.h>

#include "cc2430_sfr.h"
#include "dev/uart0.h"

#if UART_ZERO_ENABLE
/*---------------------------------------------------------------------------*/
void
uart0_init()
{
  UART_SET_SPEED(0, UART_115_M, UART_115_E);

#ifdef UART0_ALTERNATIVE_2
  PERCFG |= U0CFG;  /*alternative port 2 = P1.5-2*/
#ifdef UART0_RTSCTS
  P1SEL |= 0x3C;    /*peripheral select for TX and RX, RTS, CTS*/
#else
  P1SEL |= 0x30;    /*peripheral select for TX and RX*/
  P1 &= ~0x08;    /*RTS down*/
#endif
  P1DIR |= 0x28;    /*RTS, TX out*/
  P1DIR &= ~0x14;   /*CTS & RX in*/
#else
  PERCFG &= ~U0CFG; /*alternative port 1 = P0.5-2*/
#ifdef UART0_RTSCTS
  P0SEL |= 0x3C;    /*peripheral select for TX and RX, RTS, CTS*/
#else
  P0SEL |= 0x0C;    /*peripheral select for TX and RX*/
  P0 &= ~0x20;    /*RTS down*/
#endif
  P0DIR |= 0x28;    /*RTS & TX out*/
  P0DIR &= ~0x14;   /*CTS & RX in*/
#endif


#ifdef UART0_RTSCTS
  U0UCR = 0x42;   /*defaults: 8N1, RTS/CTS, high stop bit*/
#else
  U0UCR = 0x02;   /*defaults: 8N1, no flow control, high stop bit*/
#endif

  U0CSR = U_MODE | U_RE | U_TXB;  /*UART mode, receiver enable, TX done*/

  /*set priority group of group 3 to highest, so the UART won't miss bytes*/
  IP1 |= IP1_3;
  IP0 |= IP0_3;
}
/*---------------------------------------------------------------------------*/
/* Write one byte over the UART. */
void
uart0_writeb(uint8_t byte)
{
  IRCON2_UTX0IF = 0;
  U0BUF = byte;
  while(!IRCON2_UTX0IF); /* Wait until byte has been transmitted. */
  IRCON2_UTX0IF = 0;
}
#endif
