
#include "dev/slip.h"

#include "uart.h"
#include "llwu.h"

/*---------------------------------------------------------------------------*/
void
slip_arch_init(unsigned long ubr)
{
  uart_set_rx_callback(slip_input_byte);
  uart_enable_rx_interrupt();
  LLWU_INHIBIT_LLS();
}
/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  uart_putchar(BOARD_DEBUG_UART, c);
}
