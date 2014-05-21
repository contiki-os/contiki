#include <stddef.h>

#include "K60.h"
#include "config-board.h"
#include "config-clocks.h"
#include "uart.h"

static int (*rx_callback)(unsigned char) = NULL;

/**
 * Enable the clock gate to an UART module
 *
 * This is a convenience function mapping UART module number to SIM_SCG register.
 *
 * \param uartch UART module base pointer
 */
void uart_module_enable(UART_MemMapPtr uartch)
{
  if (uartch == UART0_BASE_PTR)
  {
    SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
  }
  else if (uartch == UART1_BASE_PTR)
  {
    SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
  }
  else if (uartch == UART2_BASE_PTR)
  {
    SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  }
  else if (uartch == UART3_BASE_PTR)
  {
    SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
  }
  else if (uartch == UART4_BASE_PTR)
  {
    SIM_SCGC1 |= SIM_SCGC1_UART4_MASK;
  }
  else if (uartch == UART5_BASE_PTR)
  {
    SIM_SCGC1 |= SIM_SCGC1_UART5_MASK;
  }
  else
  {
    /* Unknown UART module!! */
    asm("bkpt #66\n");
    return;
  }
}

/**
 * Initialize UART.
 *
 * This is based on the example found in the CodeWarrior samples provided by
 * Freescale.
 *
 * \param uartch UART channel to initialize (pointer to UART base register)
 * \param module_clk_hz Module clock (in Hz) of the given UART.
 * \param baud Desired target baud rate of the UART.
 */
void uart_init(UART_MemMapPtr uartch, uint32_t module_clk_hz, uint32_t baud)
{
  uint16_t sbr;
  uint16_t brfa;

  SIM_SCGC5  |= SIM_SCGC5_PORTC_MASK;
  /* Choose UART1 RX for the pin mux and disable PORT interrupts on the pin */
  PORTC_PCR3 = PORT_PCR_MUX(3);

  /* Choose UART1 TX for the pin mux and disable PORT interrupts on the pin */
  PORTC_PCR4 = PORT_PCR_MUX(3);

  /* Enable the clock to the selected UART */
  uart_module_enable(uartch);

  /* Compute new SBR value */
  sbr = UART_SBR(module_clk_hz, baud);
  /* Compute new fine-adjust value */
  brfa = UART_BRFA(module_clk_hz, baud);

  /* Make sure that the transmitter and receiver are disabled while we
   * change settings.
   */
  UART_C2_REG(uartch) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );

  /* Configure the UART for 8-bit mode, no parity */
  UART_C1_REG(uartch) = 0;	/* We need all default settings, so entire register is cleared */

  /* Replace SBR bits in BDH, BDL registers */
  /* High bits */
  UART_BDH_REG(uartch) = (UART_BDH_REG(uartch) & ~(UART_BDH_SBR(0xFF))) |
                         UART_BDH_SBR(sbr >> 8);
  /* Low bits */
  UART_BDL_REG(uartch) = (UART_BDL_REG(uartch) & ~(UART_BDL_SBR(0xFF))) |
                         UART_BDL_SBR(sbr);
  /* Fine adjust */
  UART_C4_REG(uartch) = (UART_C4_REG(uartch) & ~(UART_C4_BRFA(0xFF))) |
                        UART_C4_BRFA(brfa);

  /* Enable transmitter and receiver and enable receive interrupt */
  UART_C2_REG(uartch) |= UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK;
}

/*
 * Send char on UART1.
 */
void uart_putchar(UART_MemMapPtr uartch, char ch)
{
  /* Wait until space is available in the FIFO */
  while (!(UART_S1_REG(uartch) & UART_S1_TDRE_MASK));

  /* Send the character */
  UART_D_REG(uartch) = ch;
}

/*
 * Send string to UART1.
 */
void uart_putstring(UART_MemMapPtr uartch, char *str)
{
  char *p = str;
  while(*p)
    uart_putchar(uartch, *p++);
}

void uart_enable_rx_interrupt()
{
  int tmp;
  tmp = UART1_S1; // Clr status 1 register
  (void)tmp; /* Avoid compiler warnings [-Wunused-variable] */
  NVICISER1  |= (1<<15); // Enable Uart1 status interrupt
}

void uart_set_rx_callback(int (*callback)(unsigned char))
{
  rx_callback = callback;
}

void _isr_uart1_status_sources()
{
  int tmp;
  tmp = UART1_S1; // Clr status 1 register
  (void)tmp; /* Avoid compiler warnings [-Wunused-variable] */
  if (rx_callback != NULL)
  {
    rx_callback(UART1_D);
  }
}
