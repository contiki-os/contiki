/**
 * \file
 *
 * Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef _SAMG53_USART_INSTANCE_
#define _SAMG53_USART_INSTANCE_

/* ========== Register definition for USART peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define REG_USART_CR                    (0x40024000U) /**< \brief (USART) Control Register */
  #define REG_USART_MR                    (0x40024004U) /**< \brief (USART) Mode Register */
  #define REG_USART_IER                   (0x40024008U) /**< \brief (USART) Interrupt Enable Register */
  #define REG_USART_IDR                   (0x4002400CU) /**< \brief (USART) Interrupt Disable Register */
  #define REG_USART_IMR                   (0x40024010U) /**< \brief (USART) Interrupt Mask Register */
  #define REG_USART_CSR                   (0x40024014U) /**< \brief (USART) Channel Status Register */
  #define REG_USART_RHR                   (0x40024018U) /**< \brief (USART) Receiver Holding Register */
  #define REG_USART_THR                   (0x4002401CU) /**< \brief (USART) Transmitter Holding Register */
  #define REG_USART_BRGR                  (0x40024020U) /**< \brief (USART) Baud Rate Generator Register */
  #define REG_USART_RTOR                  (0x40024024U) /**< \brief (USART) Receiver Time-out Register */
  #define REG_USART_TTGR                  (0x40024028U) /**< \brief (USART) Transmitter Timeguard Register */
  #define REG_USART_FIDI                  (0x40024040U) /**< \brief (USART) FI DI Ratio Register */
  #define REG_USART_NER                   (0x40024044U) /**< \brief (USART) Number of Errors Register */
  #define REG_USART_IF                    (0x4002404CU) /**< \brief (USART) IrDA Filter Register */
  #define REG_USART_WPMR                  (0x400240E4U) /**< \brief (USART) Write Protect Mode Register */
  #define REG_USART_WPSR                  (0x400240E8U) /**< \brief (USART) Write Protect Status Register */
  #define REG_USART_RPR                   (0x40024100U) /**< \brief (USART) Receive Pointer Register */
  #define REG_USART_RCR                   (0x40024104U) /**< \brief (USART) Receive Counter Register */
  #define REG_USART_TPR                   (0x40024108U) /**< \brief (USART) Transmit Pointer Register */
  #define REG_USART_TCR                   (0x4002410CU) /**< \brief (USART) Transmit Counter Register */
  #define REG_USART_RNPR                  (0x40024110U) /**< \brief (USART) Receive Next Pointer Register */
  #define REG_USART_RNCR                  (0x40024114U) /**< \brief (USART) Receive Next Counter Register */
  #define REG_USART_TNPR                  (0x40024118U) /**< \brief (USART) Transmit Next Pointer Register */
  #define REG_USART_TNCR                  (0x4002411CU) /**< \brief (USART) Transmit Next Counter Register */
  #define REG_USART_PTCR                  (0x40024120U) /**< \brief (USART) Transfer Control Register */
  #define REG_USART_PTSR                  (0x40024124U) /**< \brief (USART) Transfer Status Register */
#else
  #define REG_USART_CR   (*(__O  uint32_t*)0x40024000U) /**< \brief (USART) Control Register */
  #define REG_USART_MR   (*(__IO uint32_t*)0x40024004U) /**< \brief (USART) Mode Register */
  #define REG_USART_IER  (*(__O  uint32_t*)0x40024008U) /**< \brief (USART) Interrupt Enable Register */
  #define REG_USART_IDR  (*(__O  uint32_t*)0x4002400CU) /**< \brief (USART) Interrupt Disable Register */
  #define REG_USART_IMR  (*(__I  uint32_t*)0x40024010U) /**< \brief (USART) Interrupt Mask Register */
  #define REG_USART_CSR  (*(__I  uint32_t*)0x40024014U) /**< \brief (USART) Channel Status Register */
  #define REG_USART_RHR  (*(__I  uint32_t*)0x40024018U) /**< \brief (USART) Receiver Holding Register */
  #define REG_USART_THR  (*(__O  uint32_t*)0x4002401CU) /**< \brief (USART) Transmitter Holding Register */
  #define REG_USART_BRGR (*(__IO uint32_t*)0x40024020U) /**< \brief (USART) Baud Rate Generator Register */
  #define REG_USART_RTOR (*(__IO uint32_t*)0x40024024U) /**< \brief (USART) Receiver Time-out Register */
  #define REG_USART_TTGR (*(__IO uint32_t*)0x40024028U) /**< \brief (USART) Transmitter Timeguard Register */
  #define REG_USART_FIDI (*(__IO uint32_t*)0x40024040U) /**< \brief (USART) FI DI Ratio Register */
  #define REG_USART_NER  (*(__I  uint32_t*)0x40024044U) /**< \brief (USART) Number of Errors Register */
  #define REG_USART_IF   (*(__IO uint32_t*)0x4002404CU) /**< \brief (USART) IrDA Filter Register */
  #define REG_USART_WPMR (*(__IO uint32_t*)0x400240E4U) /**< \brief (USART) Write Protect Mode Register */
  #define REG_USART_WPSR (*(__I  uint32_t*)0x400240E8U) /**< \brief (USART) Write Protect Status Register */
  #define REG_USART_RPR  (*(__IO uint32_t*)0x40024100U) /**< \brief (USART) Receive Pointer Register */
  #define REG_USART_RCR  (*(__IO uint32_t*)0x40024104U) /**< \brief (USART) Receive Counter Register */
  #define REG_USART_TPR  (*(__IO uint32_t*)0x40024108U) /**< \brief (USART) Transmit Pointer Register */
  #define REG_USART_TCR  (*(__IO uint32_t*)0x4002410CU) /**< \brief (USART) Transmit Counter Register */
  #define REG_USART_RNPR (*(__IO uint32_t*)0x40024110U) /**< \brief (USART) Receive Next Pointer Register */
  #define REG_USART_RNCR (*(__IO uint32_t*)0x40024114U) /**< \brief (USART) Receive Next Counter Register */
  #define REG_USART_TNPR (*(__IO uint32_t*)0x40024118U) /**< \brief (USART) Transmit Next Pointer Register */
  #define REG_USART_TNCR (*(__IO uint32_t*)0x4002411CU) /**< \brief (USART) Transmit Next Counter Register */
  #define REG_USART_PTCR (*(__O  uint32_t*)0x40024120U) /**< \brief (USART) Transfer Control Register */
  #define REG_USART_PTSR (*(__I  uint32_t*)0x40024124U) /**< \brief (USART) Transfer Status Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _SAMG53_USART_INSTANCE_ */
