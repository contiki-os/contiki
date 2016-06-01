#include <stdarg.h>
#include "segger-rtt.h"

int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);

int
putchar(int c)
{
  SEGGER_RTT_Write(0, &c, 1);
  return c;
}

int
printf(const char *fmt, ...)
{
  int res;
  va_list ap;
  va_start(ap, fmt);
  res = SEGGER_RTT_vprintf(0, fmt, &ap);
  va_end(ap);
  return res;
}


