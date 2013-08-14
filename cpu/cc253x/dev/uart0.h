#ifndef UART_0_H
#define UART_0_H

#include "contiki-conf.h"

#include "cc253x.h"
#include "8051def.h"
#include "uart.h"

/*---------------------------------------------------------------------------*/
/* UART0 Enable - Disable */
#ifdef UART0_CONF_ENABLE
#define UART0_ENABLE UART0_CONF_ENABLE
#else
#define UART0_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
/* UART0 Function Declarations */
#if UART0_ENABLE
void uart0_init();
void uart0_writeb(uint8_t byte);

void uart0_set_input(int (*input)(unsigned char c));

#if UART0_CONF_WITH_INPUT
void uart0_rx_isr( void ) __interrupt (URX0_VECTOR);
/* Macro to turn on / off UART RX Interrupt */
#define UART0_RX_INT(v) do { URX0IE = v; } while(0)
#define UART0_RX_EN()   do {   U0CSR |= UCSR_RE; } while(0)
#else
#define UART0_RX_INT(v)
#define UART0_RX_EN()
#endif /* UART0_CONF_WITH_INPUT */
#else
#define uart0_init(...)
#define uart0_writeb(...)
#define uart0_set_input(...)
#define UART0_RX_INT(v)
#define UART0_RX_EN()
#endif /* UART0_ENABLE */
#endif /* UART_0_H */
