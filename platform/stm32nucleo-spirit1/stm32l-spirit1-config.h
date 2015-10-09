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
/*----------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L_SPIRIT1_CONFIG_H
#define __STM32L_SPIRIT1_CONFIG_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL
#define USB_DISCONNECT                      GPIOD
#define USB_DISCONNECT_PIN                  GPIO_PIN_9
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD
#define EVAL_COM1_IRQHandler                USART1_IRQHandler

#elif defined(USE_STM3210E_EVAL)
#define USB_DISCONNECT                      GPIOB
#define USB_DISCONNECT_PIN                  GPIO_PIN_14
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB
#define EVAL_COM1_IRQHandler                USART1_IRQHandler

#elif defined(USE_STM3210C_EVAL)
#define USB_DISCONNECT                      0
#define USB_DISCONNECT_PIN                  0
#define RCC_APB2Periph_GPIO_DISCONNECT      0
#define EVAL_COM1_IRQHandler                USART2_IRQHandler

#elif defined(USE_STM32L152_EVAL) || defined(USE_STM32L152D_EVAL)
/*
   For STM32L15xx devices it is possible to use the internal USB pullup
   controlled by register SYSCFG_PMC (refer to RM0038 reference manual for
   more details).
   It is also possible to use external pullup (and disable the internal pullup)
   by setting the define USB_USE_EXTERNAL_PULLUP in file platform_config.h
   and configuring the right pin to be used for the external pull up configuration.
   To have more details on how to use an external pull up, please refer to
   STM3210E-EVAL evaluation board manuals.
 */
/* Uncomment the following define to use an external pull up instead of the
   integrated STM32L15xx internal pull up. In this case make sure to set up
   correctly the external required hardware and the GPIO defines below.*/
/* #define USB_USE_EXTERNAL_PULLUP */

#if !defined(USB_USE_EXTERNAL_PULLUP)
#define STM32L15_USB_CONNECT                SYSCFG_USBPuCmd(ENABLE)
#define STM32L15_USB_DISCONNECT             SYSCFG_USBPuCmd(DISABLE)

#elif defined(USB_USE_EXTERNAL_PULLUP)
/* PA0 is chosen just as illustrating example, you should modify the defines
   below according to your hardware configuration. */
#define USB_DISCONNECT                      GPIOA
#define USB_DISCONNECT_PIN                  GPIO_PIN_0
#define RCC_AHBPeriph_GPIO_DISCONNECT       RCC_AHBPeriph_GPIOA
#define STM32L15_USB_CONNECT                GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
#define STM32L15_USB_DISCONNECT             GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
#endif /* USB_USE_EXTERNAL_PULLUP */

#ifdef USE_STM32L152_EVAL
#define EVAL_COM1_IRQHandler              USART2_IRQHandler
#elif defined(USE_STM32L152D_EVAL)
#define EVAL_COM1_IRQHandler              USART1_IRQHandler
#endif /*USE_STM32L152_EVAL*/

#endif /* USE_STM3210B_EVAL */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __STM32L_SPIRIT1_CONFIG_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
