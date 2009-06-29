#include "dev/msb430-uart1.h"
#include "dev/rs232.h"
#include "sys/clock.h"
#include "dev/slip.h"


void
slip_arch_writeb(unsigned char c)
{
  rs232_send(c);
}
/*---------------------------------------------------------------------------*/

/*
 * The serial line is used to transfer IP packets using slip. To make
 * it possible to send debug output over the same line we send debug
 * output as slip frames (i.e delimeted by SLIP_END).
 *
 */
int
putchar(int c)
{
#define SLIP_END 0300
  static char debug_frame = 0;

  if(!debug_frame) {		/* Start of debug output */
    slip_arch_writeb(SLIP_END);
    slip_arch_writeb('\r');	/* Type debug line == '\r' */
    debug_frame = 1;
  }

  slip_arch_writeb((char)c);
  
  /*
   * Line buffered output, a newline marks the end of debug output and
   * implicitly flushes debug output.
   */
  if(c == '\n') {
    slip_arch_writeb(SLIP_END);
    debug_frame = 0;
  }

  clock_delay(100);

  return c;
}

/**
 * Initalize the RS232 port and the SLIP driver.
 *
 */
void
slip_arch_init(unsigned long ubr)
{
  rs232_set_input(slip_input_byte);
}
