/**
 * \file
 *         Provide common UART routines for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#ifndef CPU_ARM_K60_UART_H_
#define CPU_ARM_K60_UART_H_

#include <stdint.h>
#include "K60.h"

void uart_module_enable(const unsigned int uart_num);
void uart_init(const unsigned int uart_num, uint32_t module_clk_hz, const uint32_t baud);
void uart_putchar(const unsigned int uart_num, const char ch);
void uart_putstring(const unsigned int uart_num, const char *str);
void uart_enable_rx_interrupt(const unsigned int uart_num);
void uart_set_rx_callback(const unsigned int uart_num, int (*callback)(unsigned char));

/*
 * Baud rate generator is driven by the System clock (UART0, UART1) or bus
 * clock (UART2-4) divided by SBR in UARTx_BDL, UARTx_BDH (13 bits)
 * The receiver needs to sample the input at 16 times the line baud rate.
 *
 * From the reference manual:
 * UART baud rate = UART module clock / (16 * (SBR[12:0] + BRFD))
 *
 * So if we want 115200 baud, we need to have a UART clock of (after dividing)
 * 115200*16=1843200
 * If we are running a system clock of 96 MHz we will need to divide the clock
 * by 96000000/1843200=52.083333
 * We set the clock divisor SBR to 52 and the BRFA fine adjust to 3 (BRFD = 0.09375)
 * This yields a baud rate of 115176.9646.
 * Alternatively, we can run at BRFA = 2 yielding a baud rate of 115246.0984
 */
/**
 * UART module SBR parameter based on module frequency f and desired baud rate b.
 */
#define UART_SBR(f, b) ((f) / (b * 16))

/*
 * The constant numbers will be computed compile time by most (all?) compilers.
 * The suffix ull on 64ull is in order to avoid overflows in the variable in
 * the compiler when computing the number. Without ull suffix the number will
 * be truncated to a 32 bit integer before the division yielding the wrong
 * fine adjust value.
 */
/*
 * The below calculation will yield a fine adjust value rounded to the nearest
 * configurable fraction.
 */
/**
 * UART module fine adjust parameter based on module frequency f and desired baud rate b.
 */
/**
 * \todo Verify proper rounding on UART1 fine adjust calculation
 *
 * \todo Verify the UART1 fine-adjust calculations if F_SYS*32 > 2^32 <=> F_SYS > 2^27 (== 134217728)
 */
#define UART_BRFA(f, b) ((((4 * (f)) / (b) + 1) / 2) % 32)

#endif /* CPU_ARM_K60_UART_H_ */
