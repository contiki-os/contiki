#ifndef UART_H
#define UART_H

#include "contiki-conf.h"
#include "banked.h"

#include "cc2430_sfr.h"

void uart0_init(uint32_t speed) __banked;
void uart0_writeb(uint8_t byte);

void uart0_set_input(int (*input)(unsigned char c));

void uart0_rxISR( void ) __interrupt (URX0_VECTOR);
void uart0_txISR( void ) __interrupt (UTX0_VECTOR);

void uart1_init(uint32_t speed) __banked;
void uart1_writeb(uint8_t byte);

void uart1_set_input(int (*input)(unsigned char c));

void uart1_rxISR( void ) __interrupt (URX1_VECTOR);
void uart1_txISR( void ) __interrupt (UTX1_VECTOR);

#endif /*UART_H*/
