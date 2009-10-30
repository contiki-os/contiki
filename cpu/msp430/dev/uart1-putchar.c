#include <stdio.h>
#include "dev/uart1.h"

int
putchar(int c)
{
  uart1_writeb((char)c);
  return c;
}
