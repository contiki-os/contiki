#include <stdio.h>
#include "dev/uart1.h"


#include PLATFORM_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
//#include "uart.h"

#ifdef __GNUC__
# define _LLIO_STDIN ((int) stdin)
# define _LLIO_STDOUT ((int) stdout)
# define _LLIO_STDERR ((int) stderr)
# define _LLIO_ERROR  (-1)
#else
# ifdef __ICCARM__
# include <yfuns.h>
# endif
#endif

#undef putchar

int __attribute__(( weak )) putchar(int c)
{
  uart1_writeb(c);
  return c;
}

void __io_putchar(char c)
{
  putchar(c);
}

size_t _write(int handle, const unsigned char * buffer, size_t size)
{
  size_t nChars = 0;

  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
    return _LLIO_ERROR;
  }

  if (buffer == 0) {
    // This means that we should flush internal buffers.  
    //spin until TX complete (TX is idle)
    while ((SC1_UARTSTAT&SC_UARTTXIDLE)!=SC_UARTTXIDLE) {}
    return 0;
  }
  
  // ensure port is configured for UART
  if(SC1_MODE != SC1_MODE_UART) {
    return _LLIO_ERROR;
  }

  while(size--) {
    __io_putchar(*buffer++);
    ++nChars;
  }

  return nChars;
}


size_t _read(int handle, unsigned char * buffer, size_t size)
{
	return 0;
}

