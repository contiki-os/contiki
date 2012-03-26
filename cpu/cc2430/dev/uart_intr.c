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
#include "dev/uart0.h"
#include "dev/uart1.h"
#include "sys/energest.h"

#if UART_ZERO_ENABLE
static int (*uart0_input_handler)(unsigned char c);
#endif
#if UART_ONE_ENABLE
static int (*uart1_input_handler)(unsigned char c);
#endif

#if UART_ZERO_ENABLE
/*---------------------------------------------------------------------------*/
void
uart0_set_input(int (*input)(unsigned char c))
{
  uart0_input_handler = input;
}

/*---------------------------------------------------------------------------*/
void
uart0_rx_ISR(void) __interrupt (URX0_VECTOR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  TCON_URX0IF = 0;
  if(uart0_input_handler != NULL) {
    uart0_input_handler(U0BUF);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
uart0_tx_ISR( void ) __interrupt (UTX0_VECTOR)
{
}
#endif /* UART_ZERO_ENABLE */
#if UART_ONE_ENABLE
/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input)(unsigned char c))
{
  uart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/
#if UART_ONE_CONF_WITH_INPUT
void
uart1_rx_ISR(void) __interrupt (URX1_VECTOR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  TCON_URX1IF = 0;
  if(uart1_input_handler != NULL) {
    uart1_input_handler(U1BUF);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
uart1_tx_ISR( void ) __interrupt (UTX1_VECTOR)
{
}
/*---------------------------------------------------------------------------*/
#endif /* UART_ONE_CONF_WITH_INPUT */
#endif /* UART_ONE_ENABLE */
