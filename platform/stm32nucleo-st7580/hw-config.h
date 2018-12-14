/*
 * Copyright (c) 2017, STMicroelectronics.
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
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H
/*---------------------------------------------------------------------------*/
#include "stm32l-st7580-config.h"
/*---------------------------------------------------------------------------*/
#define UART_RxBufferSize    512
/*---------------------------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __DMA1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA

#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART2_IRQn
#define USARTx_IRQHandler                USART2_IRQHandler

#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* USART1 */

#define UARTplm_RxBufferSize 512

#define PLM_USART                           USART1
#define PLM_USART_BAUDRATE                  57600
#define PLM_USART_CLK_ENABLE()              __USART1_CLK_ENABLE()
#define PLM_USART_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define PLM_USART_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define PLM_USART_FORCE_RESET()             __USART1_FORCE_RESET()
#define PLM_USART_RELEASE_RESET()           __USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define PLM_USART_TX_PIN                    GPIO_PIN_9
#define PLM_USART_TX_GPIO_PORT              GPIOA

#define PLM_USART_RX_PIN                    GPIO_PIN_10
#define PLM_USART_RX_GPIO_PORT              GPIOA

/* Definition for USARTx's NVIC */
#define PLM_USART_IRQn                      USART1_IRQn
#define PLM_USART_IRQHandler                USART1_IRQHandler

#define PLM_USART_TX_AF                     GPIO_AF7_USART1
#define PLM_USART_RX_AF                     GPIO_AF7_USART1

/*---------------------------------------------------------------------------*/
#endif /*__HW_CONFIG_H*/
/*---------------------------------------------------------------------------*/
