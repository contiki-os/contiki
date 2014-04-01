#include <stddef.h>

#include "K60.h"
#include "config-board.h"
#include "config-clocks.h"
#include "uart.h"

static int (*rx_callback)(unsigned char) = NULL;
/*
 * Initialize UART1 to baud 115200
 */
void uart_init(void)
{
  SIM_SCGC5  |= SIM_SCGC5_PORTC_MASK;
  /* Choose UART1 RX for the pin mux and disable PORT interrupts on the pin */
  PORTC_PCR3 = PORT_PCR_MUX(3);

  /* Choose UART1 TX for the pin mux and disable PORT interrupts on the pin */
  PORTC_PCR4 = PORT_PCR_MUX(3);

  /* SIM_SCGC4 */
  SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;

  /*
   * Baud rate generator is driven by the System clock (UART0, UART1) or bus
   * clock (UART2-4) divided by SBR in UARTx_BDL, UARTx_BDH (13 bits)
   * The receiver needs to sample the input at 16 times the line baud rate.
   *
   * From the reference manual: UART baud rate = UART module clock / (16 * (SBR[12:0] + BRFD))
   *
   * So if we want 115200 baud, we need to have a UART clock of (after dividing)
   * 115200*16=1843200
   * If we are running a system clock of 96 MHz we will need to divide the clock
   * by 96000000/1843200=52.083333
   * We set the clock divisor SBR to 52 and the BRFA fine adjust to 3 (BRFD = 0.09375)
   * This yields a baud rate of 115176.9646.
   * Alternatively, we can run at BRFA = 2 yielding a baud rate of 115246.0984
   */
  UART1_BDH    = UART_BDH_SBR(((F_SYS)/(K60_DEBUG_BAUD * 16)) / 256);
  UART1_BDL    = UART_BDL_SBR(((F_SYS)/(K60_DEBUG_BAUD * 16)) % 256);
  /**
   * \todo Verify the UART1 fine-adjust calculations if F_SYS*32 > 2^32 <=> F_SYS > 2^27 (== 134217728)
   */
  /*
   * The below calculation will yield a fine adjust value rounded to the nearest
   * configurable fraction.
   */
  /** \todo Verify proper rounding on UART1 fine adjust calculation */
  /*
   * The number will be computed compile time by most (all?) compilers.
   * The suffix ull on 64ull is in order to avoid overflows in the variable in
   * the compiler when computing the number. Without ull suffix the number will
   * be truncated to a 32 bit integer before the division yielding the wrong
   * fine adjust value. */
  UART1_C4     = UART_C4_BRFA((((64ull * (F_SYS))/((K60_DEBUG_BAUD) * 16)+1)/2) % 32);
  /* Enable transmitter and receiver and enable receive interrupt */
  UART1_C2     = UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK;
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
