#include "dev/uart0.h"
#include <stdio.h>

int
putchar(int c)
{
  uart0_writeb((char)c);
  return c;
}
