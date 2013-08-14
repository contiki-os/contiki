#ifndef UART_0_H
#define UART_0_H

#include "contiki-conf.h"

#include "cc2430_sfr.h"
#include "8051def.h"
#include "uart.h"

/*---------------------------------------------------------------------------*/
/* UART0 Enable - Disable */
#ifdef UART_ZERO_CONF_ENABLE
#define UART_ZERO_ENABLE UART_ZERO_CONF_ENABLE
#else
#define UART_ZERO_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
/* UART0 Function Declarations */
#if UART_ZERO_ENABLE
void uart0_init();
void uart0_writeb(uint8_t byte);

void uart0_set_input(int (* input)(unsigned char c));

void uart0_rx_ISR(void) __interrupt(URX0_VECTOR);
void uart0_tx_ISR(void) __interrupt(UTX0_VECTOR);
/* Macro to turn on / off UART RX Interrupt */
#define UART0_RX_INT(v) do { IEN0_URX0IE = v; } while(0)
#endif

#endif /* UART_0_H */
