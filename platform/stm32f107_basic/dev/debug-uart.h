/**
 * USART driver for STM32F1xx w/ libopencm3 peripherl lib
 * (c) 2012 Blessed Contraptions
 * Jeff Ciesielski <jeff.ciesielski@gmail.com>
 */

#ifndef _UART_H_
#define _UART_H_

int uart_putchar(char c);
int uart_getchar(char *c);
void uart_init(int baud);

#endif
