/*
 * Copyright (c) 2012, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "console.h"
#include "stm32l1xx.h"
#include "stm32l1xx_hal_dma.h"
#include "stm32l1xx_hal_uart.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
extern st_lib_uart_handle_typedef st_lib_uart_handle;
/*---------------------------------------------------------------------------*/
/**
 * @brief Initialises Nucleo UART port for user IO
 * @retval 0
 */
int
console_init(void)
{
  st_lib_uart_handle.Instance = USART2;

  st_lib_uart_handle.Init.BaudRate = 115200;
  st_lib_uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
  st_lib_uart_handle.Init.StopBits = UART_STOPBITS_1;
  st_lib_uart_handle.Init.Parity = UART_PARITY_NONE;
  st_lib_uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  st_lib_uart_handle.Init.Mode = UART_MODE_TX_RX;

  st_lib_hal_uart_init(&st_lib_uart_handle);

  return 0;
}
/*---------------------------------------------------------------------------*/
/** @brief Sends a character to serial port
 * @param ch Character to send
 * @retval Character sent
 */
int
uart_send_char(int ch)
{
  st_lib_hal_uart_transmit(&st_lib_uart_handle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/*---------------------------------------------------------------------------*/
/** @brief Receives a character from serial port
 * @retval Character received
 */
int
uart_receive_char(void)
{
  uint8_t ch;
  st_lib_hal_uart_receive(&st_lib_uart_handle, &ch, 1, HAL_MAX_DELAY);

  /* Echo character back to console */
  st_lib_hal_uart_transmit(&st_lib_uart_handle, &ch, 1, HAL_MAX_DELAY);

  /* And cope with Windows */
  if(ch == '\r') {
    uint8_t ret = '\n';
    st_lib_hal_uart_transmit(&st_lib_uart_handle, &ret, 1, HAL_MAX_DELAY);
  }

  return ch;
}
/*---------------------------------------------------------------------------*/
#if defined(__IAR_SYSTEMS_ICC__)

size_t __write(int Handle, const unsigned char *Buf, size_t Bufsize);
size_t __read(int Handle, unsigned char *Buf, size_t Bufsize);

/** @brief IAR specific low level standard input
 * @param handle IAR internal handle
 * @param buf Buffer where to store characters read from stdin
 * @param bufsize Number of characters to read
 * @retval Number of characters read
 */
size_t
__read(int handle, unsigned char *buf, size_t bufsize)
{
  int i;

  if(handle != 0) {
    return -1;
  }

  for(i = 0; i < bufsize; i++) {
    buf[i] = uart_receive_char();
  }

  return bufsize;
}
/** @brief IAR specific low level standard output
 * @param handle IAR internal handle
 * @param buf Buffer containing characters to be written to stdout
 * @param bufsize Number of characters to write
 * @retval Number of characters read
 */
size_t
__write(int handle, const unsigned char *buf, size_t bufsize)
{
  int i;

  if(handle != 1 && handle != 2) {
    return -1;
  }

  for(i = 0; i < bufsize; i++) {
    uart_send_char(buf[i]);
  }

  return bufsize;
}
/*---------------------------------------------------------------------------*/
#elif defined(__CC_ARM)
/**
 * @brief fputc call for standard output implementation
 * @param ch Character to print
 * @param f File pointer
 * @retval Character printed
 */
int
fputc(int ch, FILE *f)
{
  return uart_send_char(ch);
}
/** @brief fgetc call for standard input implementation
 * @param f File pointer
 * @retval Character acquired from standard input
 */
int
fgetc(FILE *f)
{
  return uart_receive_char();
}
/*---------------------------------------------------------------------------*/
#elif defined(__GNUC__)

/** @brief putchar call for standard output implementation
 * @param ch Character to print
 * @retval Character printed
 */
int
__io_putchar(int ch)
{
  return uart_send_char(ch);
}
/** @brief getchar call for standard input implementation
 * @param None
 * @retval Character acquired from standard input
 */
int
__io_getchar(void)
{
  return uart_receive_char();
}
/*---------------------------------------------------------------------------*/
#else
#error "Toolchain not supported"
#endif
/*---------------------------------------------------------------------------*/
