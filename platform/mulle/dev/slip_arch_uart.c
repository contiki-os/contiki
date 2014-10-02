
#include "dev/slip.h"

#include "uart.h"
#include "llwu.h"

/*---------------------------------------------------------------------------*/
void
slip_arch_init(unsigned long ubr)
{
  /* ubr is the desired baud rate, but the name comes from the msp430 platform
   * where baud is converted to some platform specific "UBR" parameter. */
  /** \todo (MULLE) Handle baud rate requests in slip_arch_init */
  uart_set_rx_callback(slip_input_byte);
  uart_enable_rx_interrupt();
  /* Don't allow LLS since it will disable the UART module clock, which prevents
   * any incoming bytes from being detected. */
  LLWU_INHIBIT_LLS();
}
/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  uart_putchar(BOARD_DEBUG_UART, c);
}
