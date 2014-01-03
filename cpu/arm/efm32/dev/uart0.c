/*
 * Copyright (c) 2013, Kerlink
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
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup efm32-devices
 * @{
 */

/**
 * \file
 *         EFM32 UART0 driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdlib.h>
#include "sys/energest.h"

#include <efm32.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "uart0.h"
#include "dev/watchdog.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


static int (*uart0_input_handler)(unsigned char c);

/*---------------------------------------------------------------------------*/
//UART0_TX_IRQHandler
void UART0_RX_IRQHandler(void)
{
  unsigned char c;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(UART0->STATUS & UART_STATUS_RXDATAV)
  {
    c = USART_Rx(UART0);
    if(uart0_input_handler != NULL) uart0_input_handler(c);
  }
  /*
  else
  {
	  PRINTF("ERROR: control reg = 0x%lX", (SI32_UART_0->CONTROL.U32 & 0x7));
	  // Disable all errors
	  SI32_UART_0->CONTROL_CLR = 0x07;
  }
*/
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
uint8_t
uart0_active(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void
uart0_set_input(int (*input)(unsigned char c))
{
  uart0_input_handler = input;
}
/*---------------------------------------------------------------------------*/
int *uart0_get_input(void)
{
  return (int*) uart0_input_handler;
}
/*---------------------------------------------------------------------------*/
void
uart0_writeb(unsigned char c)
{
  watchdog_periodic();
#ifdef UART0_LF_TO_CRLF
  if(c == '\n')
  {
    USART_Tx(UART0, '\r');
  }
#endif

  USART_Tx(UART0, c);

#ifdef UART0_LF_TO_CRLF
  if(c == '\r')
  {
    USART_Tx(UART0, '\n');
  }
#endif
}
/*---------------------------------------------------------------------------*/
unsigned int
uart0_send_bytes(const unsigned char *seq, unsigned int len)
{
  /* TODO : Use DMA Here ... */

  int i=0;
  for(i=0;i<len;i++)
  {
    uart0_writeb(seq[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
void
uart0_drain(void)
{
  while (!(UART0->STATUS & UART_STATUS_TXBL));
}
/*---------------------------------------------------------------------------*/
/**
 * Initialize the UART.
 *
 */
void
uart0_init(unsigned long baudrate)
{
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
  /* Configure controller */

  // Enable clocks
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_UART0, true);

  init.enable = usartDisable;
  init.baudrate = baudrate;
  USART_InitAsync(UART0, &init);

  /* Enable pins at UART0 location */
  UART0->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN |
                  (UART0_LOCATION << _UART_ROUTE_LOCATION_SHIFT);

  /* Clear previous RX interrupts */
  USART_IntClear(UART0, UART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(UART0_RX_IRQn);

  /* Enable RX interrupts */
  USART_IntEnable(UART0, UART_IF_RXDATAV);
  NVIC_EnableIRQ(UART0_RX_IRQn);

  /* Finally enable it */
  USART_Enable(UART0, usartEnable);
}
/*---------------------------------------------------------------------------*/

/** @} */
