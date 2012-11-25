/*
 * debug-uart.h
 *
 *  Created on: Nov 25, 2012
 *      Author: cazulu
 */

#ifndef DEBUG_UART_H_
#define DEBUG_UART_H_

//Configures the UART defined in
//contiki-conf.h as DEBUG_UART
void setup_debug_uart();
void dbg_putchar(char c);
unsigned int dbg_send_bytes(const unsigned char *seq, unsigned int len);

#endif /* DEBUG_UART_H_ */
