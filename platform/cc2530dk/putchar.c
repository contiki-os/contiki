/**
 * \file
 *         hardware-specific putchar() routine for TI SmartRF05EB
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki-conf.h"
#include "dev/io-arch.h"
/*---------------------------------------------------------------------------*/
void
putchar(char c)
{
#if SLIP_ARCH_CONF_ENABLE
#define SLIP_END     0300
  static char debug_frame = 0;

  if(!debug_frame) {            /* Start of debug output */
    io_arch_writeb(SLIP_END);
    io_arch_writeb('\r');     /* Type debug line == '\r' */
    debug_frame = 1;
  }
#endif

  io_arch_writeb((char)c);

#if SLIP_ARCH_CONF_ENABLE
  /*
   * Line buffered output, a newline marks the end of debug output and
   * implicitly flushes debug output.
   */
  if(c == '\n') {
    io_arch_writeb(SLIP_END);
    debug_frame = 0;
  }
#endif
}
