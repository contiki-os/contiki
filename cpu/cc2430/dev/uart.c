/**
 * \file
 *
 *   uart write routines
 *
 * \author
 * 
 *   Anthony "Asterisk" Ambuehl
 *
 *   non-interrupt routines which may be called from ISR's and therefore should be in HOME bank.
 *
 */
#include <stdlib.h>
#include <string.h>

#include "cc2430_sfr.h"

#include "dev/leds.h"
#include "dev/uart.h"

/*---------------------------------------------------------------------------*/
/* Write one byte over the UART. */
void
uart0_writeb(uint8_t byte)
{
  IRCON2_UTX0IF = 0;
  U0BUF = byte;
  while(!IRCON2_UTX0IF); /* Wait until byte has been transmitted. */
  IRCON2_UTX0IF = 0;
}
/*---------------------------------------------------------------------------*/
/* Write one byte over the UART. */
void
uart1_writeb(uint8_t byte)
{
  IRCON2_UTX1IF = 0;
  U1BUF = byte;
  while(!IRCON2_UTX1IF); /* Wait until byte has been transmitted. */
  IRCON2_UTX1IF = 0;
}
/*---------------------------------------------------------------------------*/
