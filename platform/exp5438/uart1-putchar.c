#include <stdio.h>
#include "dev/uart1.h"
#include "lcd.h"

int
putchar(int c)
{
  uart1_writeb((char)c);

  lcd_write_char((char)c);
  return c;
}
