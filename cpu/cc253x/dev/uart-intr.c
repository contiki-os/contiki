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
#include "cc253x.h"

#include "dev/uart0.h"
#include "dev/uart1.h"
#include "sys/energest.h"
#include  "dev/leds.h"

#if UART0_ENABLE
static int (*uart0_input_handler)(unsigned char c);
#endif
#if UART1_ENABLE
static int (*uart1_input_handler)(unsigned char c);
#endif

#if UART0_ENABLE
/*---------------------------------------------------------------------------*/
void
uart0_set_input(int (*input)(unsigned char c))
{
  uart0_input_handler = input;
}
/*---------------------------------------------------------------------------*/
#if UART0_CONF_WITH_INPUT
void
uart0_rx_isr(void) __interrupt (URX0_VECTOR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  leds_toggle(LEDS_YELLOW);
  URX0IF = 0;
  if(uart0_input_handler != NULL) {
    uart0_input_handler(U0DBUF);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif
#endif /* UART0_ENABLE */
#if UART1_ENABLE
/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input)(unsigned char c))
{
  uart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/
#if UART_ONE_CONF_WITH_INPUT
void
uart1_rx_isr(void) __interrupt (URX1_VECTOR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  URX1IF = 0;
  if(uart1_input_handler != NULL) {
    uart1_input_handler(U1DBUF);
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
#endif /* UART_ONE_CONF_WITH_INPUT */
#endif /* UART1_ENABLE */
