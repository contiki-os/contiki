/**
 * \file
 *         Provide common UART routines for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#ifndef __UART_H__
#define __UART_H__

void uart_init(void);
void uart_putchar(char ch);
void uart_putstring(char *str);
void uart_enable_rx_interrupt();
void uart_set_rx_callback(int (*callback)(unsigned char));

#endif
