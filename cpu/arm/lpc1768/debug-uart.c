/*
 * debug-uart.c
 *
 *  Created on: Nov 22, 2012
 *      Author: cazulu
 */

#include "contiki-conf.h"
#include "debug-uart.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

#if DEBUG_UART==0
#define LPC_UART LPC_UART0
#endif

#if DEBUG_UART==1
#define LPC_UART LPC_UART1
#endif

//Setups the UART defined in
//contiki-conf.h as DEBUG_UART
void
setup_debug_uart()
{
  // UART Configuration structure variable
  UART_CFG_Type UARTConfigStruct;
  // UART FIFO configuration Struct variable
  UART_FIFO_CFG_Type UARTFIFOConfigStruct;
  // Pin configuration for UART0
  PINSEL_CFG_Type PinCfg;

#if (DEBUG_UART == 0)
  /*
   * Initialize UART0 pin connect
   */
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Pinnum = 2;
  PinCfg.Portnum = 0;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 3;
  PINSEL_ConfigPin(&PinCfg);
#endif

#if (DEBUG_UART == 1)
  /*
   * Initialize UART1 pin connect
   */
  PinCfg.Funcnum = 2;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Pinnum = 0;
  PinCfg.Portnum = 2;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);
#endif

  /* Initialize UART Configuration parameter structure to default state:
   * Baudrate = 9600bps
   * 8 data bit
   * 1 Stop bit
   * None parity
   */
  UART_ConfigStructInit(&UARTConfigStruct);

  // Initialize DEBUG_UART(defined in contiki-conf.h to the configuration parameters
  UART_Init((LPC_UART_TypeDef *) LPC_UART, &UARTConfigStruct);

  /* Initialize FIFOConfigStruct to default state:
   *                              - FIFO_DMAMode = DISABLE
   *                              - FIFO_Level = UART_FIFO_TRGLEV0
   *                              - FIFO_ResetRxBuf = ENABLE
   *                              - FIFO_ResetTxBuf = ENABLE
   *                              - FIFO_State = ENABLE
   */
  UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

  // Initialize FIFO for the LPC_UART peripheral
  UART_FIFOConfig(LPC_UART, &UARTFIFOConfigStruct);

  // Enable UART Transmit
  UART_TxCmd(LPC_UART, ENABLE);

}

void
dbg_putchar(char c)
{
  UART_SendByte(LPC_UART, c);
}

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
  return UART_Send(LPC_UART, seq, len, BLOCKING);
}

