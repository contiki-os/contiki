#ifndef UART_H
#define UART_H

#include "contiki-conf.h"

#include "cc253x.h"
#include "8051def.h"

/*---------------------------------------------------------------------------*/
/* UART BAUD Rates */
/*
 * Macro to set speed of UART N by setting the UnBAUD SFR to M and the
 * UnGCR SRF to E. See the cc2530 datasheet for possible values of M and E
 */
#define UART_SET_SPEED(N, M, E) do{ U##N##BAUD = M; U##N##GCR = E; } while(0)

/*
 * Sample Values for M and E in the macro above to achieve some common BAUD
 * rates. For more values, see the cc2430 datasheet
 */
/* 2000000 - cc2430 theoretical MAX when using the 32MHz clock */
#define UART_2K_M      0
#define UART_2K_E     16
/* 1000000 - cc2430 theoretical MAX when using the 16MHz clock */
#define UART_1K_M      0
#define UART_1K_E     15
/* 921600 */
#define UART_921_M   216
#define UART_921_E    14
/* 460800 Higher values lead to problems when the node needs to RX */
#define UART_460_M   216
#define UART_460_E    13
/* 115200 */
#define UART_115_M   216
#define UART_115_E    11
/* 38400 */
#define UART_38_M     59
#define UART_38_E     10
/* 9600 */
#define UART_9_M      59
#define UART_9_E       8

#endif /* UART_H */
