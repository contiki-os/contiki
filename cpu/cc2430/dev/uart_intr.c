/**
 * \file
 *
 *   uart write routines
 *
 * \author
 * 
 *   Anthony "Asterisk" Ambuehl
 *
 *   interrupt routines which must be in HOME bank.  handles received data from UART.
 *
 */
#include <stdlib.h>
#include <string.h>

#include "cc2430_sfr.h"

#include "dev/leds.h"
#include "dev/uart.h"

static int (*uart0_input_handler)(unsigned char c);
static int (*uart1_input_handler)(unsigned char c);
/*---------------------------------------------------------------------------*/
void
uart0_set_input(int (*input)(unsigned char c))
{
  uart0_input_handler = input;
}

/*---------------------------------------------------------------------------*/
void
uart0_rxISR(void) __interrupt (URX0_VECTOR)
{
  TCON_URX0IF = 0;
  if(uart0_input_handler != NULL) {
    uart0_input_handler(U0BUF);
  }
}
/*---------------------------------------------------------------------------*/
void
uart0_txISR( void ) __interrupt (UTX0_VECTOR)
{
}
/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input)(unsigned char c))
{
  uart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/
void
uart1_rxISR(void) __interrupt (URX1_VECTOR)
{
  TCON_URX1IF = 0;
  if(uart1_input_handler != NULL) {
    uart1_input_handler(U1BUF);
  }
}
/*---------------------------------------------------------------------------*/
void
uart1_txISR( void ) __interrupt (UTX1_VECTOR)
{
}
/*---------------------------------------------------------------------------*/
