/**
 * \file
 *
 *   Definition of some debugging functions.
 *
 *   putstring() and puthex() are from msp430/watchdog.c
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "8051def.h"
#include "debug.h"

static const char hexconv[] = "0123456789abcdef";
static const char binconv[] = "01";
/*---------------------------------------------------------------------------*/
void
putstring(char *s)
{
  while(*s) {
    putchar(*s++);
  }
}
/*---------------------------------------------------------------------------*/
void
puthex(uint8_t c)
{
  putchar(hexconv[c >> 4]);
  putchar(hexconv[c & 0x0f]);
}
/*---------------------------------------------------------------------------*/
void
putbin(uint8_t c)
{
  unsigned char i = 0x80;
  while(i) {
    putchar(binconv[(c & i) != 0]);
    i >>= 1;
  }
}
/*---------------------------------------------------------------------------*/
