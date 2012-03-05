#ifndef UART_1_H
#define UART_1_H

#include "contiki-conf.h"

#include "cc2430_sfr.h"
#include "8051def.h"
#include "uart.h"

/*---------------------------------------------------------------------------*/
/* UART1 Enable - Disable */
#ifdef UART_ONE_CONF_ENABLE
#define UART_ONE_ENABLE UART_ONE_CONF_ENABLE
#else
#define UART_ONE_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
/* UART1 Function Declarations */
#if UART_ONE_ENABLE
void uart1_init();
void uart1_writeb(uint8_t byte);

void uart1_set_input(int (*input)(unsigned char c));
#if UART_ONE_CONF_WITH_INPUT
void uart1_rx_ISR( void ) __interrupt (URX1_VECTOR);
void uart1_tx_ISR( void ) __interrupt (UTX1_VECTOR);
/* Macro to turn on / off UART RX Interrupt */
#define UART1_RX_INT(v) IEN0_URX1IE = v
#else
#define UART1_RX_INT(v)
#endif /* UART_ONE_CONF_WITH_INPUT */
#else
#define uart1_init(...)
#define uart1_writeb(...)
#define uart1_set_input(...)
#define UART1_RX_INT(v)
#endif /* UART_ONE_ENABLE */

#endif /* UART_1_H */
