#include <stddef.h>

#include "MK60N512VMD100.h"
#include "uart.h"

static int (*rx_callback)(unsigned char) = NULL;
/*
 * Initialize UART1 to baud 115200
 */
void uart_init(void)
{
  SIM_SCGC5  |= SIM_SCGC5_PORTC_MASK;
  /* PORTE_PCR25: ISF=0,MUX=3 */
  PORTC_PCR3 = ((PORTC_PCR25 & ~0x01000400) | 0x00000300);

  /* PORTE_PCR24: ISF=0,MUX=3 */
  PORTC_PCR4 = ((PORTC_PCR24 & ~0x01000400) | 0x00000300);

  /* SIM_SCGC4 */
  SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;

  UART1_BDH    = 0;
  UART1_BDL    = 11;
  UART1_C4     = 24;
  UART1_C2     = 0x2C;
}

/*
 * Send char on UART1.
 */
void uart_putchar(char ch)
{
  /* Wait until space is available in the FIFO */
  while (!(UART1_S1 & UART_S1_TDRE_MASK));

  /* Send the character */
  UART1_D = ch;
}

/*
 * Send string to UART1.
 */
void uart_putstring(char *str)
{
  char *p = str;
  while(*p)
    uart_putchar(*p++);
}

void uart_enable_rx_interrupt()
{
  int tmp = UART1_S1; // Clr status 1 register
  NVICISER1  |= (1<<15); // Enable Uart1 status interrupt
}

void uart_set_rx_callback(int (*callback)(unsigned char))
{
  rx_callback = callback;
}

void _isr_uart1_status_sources()
{
  int tmp = UART1_S1; // Clr S1
  if (rx_callback != NULL)
  {
    rx_callback(UART1_D);
  }
}
