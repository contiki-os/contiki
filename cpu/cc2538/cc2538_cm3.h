/*
 * Template:
 * Copyright (c) 2012 ARM LIMITED
 * All rights reserved.
 *
 * CC2538:
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
 * All rights reserved.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-cm3 CC2538 Cortex-M3
 *
 * CC2538 Cortex-M3 CMSIS definitions
 * @{
 *
 * \file
 * CMSIS Cortex-M3 core peripheral access layer header file for CC2538
 */
#ifndef CC2538_CM3_H
#define CC2538_CM3_H

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup CC2538_CMSIS CC2538 CMSIS Definitions
 * Configuration of the Cortex-M3 Processor and Core Peripherals
 * @{
 */

/** \name Interrupt Number Definition
 * @{
 */

typedef enum IRQn
{
  /******  Cortex-M3 Processor Exceptions Numbers  ****************************/
  NonMaskableInt_IRQn   = -14,  /**<  2 Non Maskable Interrupt */
  HardFault_IRQn        = -13,  /**<  3 HardFault Interrupt */
  MemoryManagement_IRQn = -12,  /**<  4 Memory Management Interrupt */
  BusFault_IRQn         = -11,  /**<  5 Bus Fault Interrupt */
  UsageFault_IRQn       = -10,  /**<  6 Usage Fault Interrupt */
  SVCall_IRQn           = -5,   /**< 11 SV Call Interrupt */
  DebugMonitor_IRQn     = -4,   /**< 12 Debug Monitor Interrupt */
  PendSV_IRQn           = -2,   /**< 14 Pend SV Interrupt */
  SysTick_IRQn          = -1,   /**< 15 System Tick Interrupt */

  /******  CC2538-Specific Interrupt Numbers  *********************************/
  GPIO_A_IRQn           = 0,    /**< GPIO port A Interrupt */
  GPIO_B_IRQn           = 1,    /**< GPIO port B Interrupt */
  GPIO_C_IRQn           = 2,    /**< GPIO port C Interrupt */
  GPIO_D_IRQn           = 3,    /**< GPIO port D Interrupt */
  UART0_IRQn            = 5,    /**< UART0 Interrupt */
  UART1_IRQn            = 6,    /**< UART1 Interrupt */
  SSI0_IRQn             = 7,    /**< SSI0 Interrupt */
  I2C_IRQn              = 8,    /**< I²C Interrupt */
  ADC_IRQn              = 14,   /**< ADC Interrupt */
  WDT_IRQn              = 18,   /**< Watchdog Timer Interrupt */
  GPT0A_IRQn            = 19,   /**< GPTimer 0A Interrupt */
  GPT0B_IRQn            = 20,   /**< GPTimer 0B Interrupt */
  GPT1A_IRQn            = 21,   /**< GPTimer 1A Interrupt */
  GPT1B_IRQn            = 22,   /**< GPTimer 1B Interrupt */
  GPT2A_IRQn            = 23,   /**< GPTimer 2A Interrupt */
  GPT2B_IRQn            = 24,   /**< GPTimer 2B Interrupt */
  ADC_CMP_IRQn          = 25,   /**< Analog Comparator Interrupt */
  RF_TX_RX_ALT_IRQn     = 26,   /**< RF Tx/Rx (Alternate) Interrupt */
  RF_ERR_ALT_IRQn       = 27,   /**< RF Error (Alternate) Interrupt */
  SYS_CTRL_IRQn         = 28,   /**< System Control Interrupt */
  FLASH_CTRL_IRQn       = 29,   /**< Flash memory Control Interrupt */
  AES_ALT_IRQn          = 30,   /**< AES (Alternate) Interrupt */
  PKA_ALT_IRQn          = 31,   /**< PKA (Alternate) Interrupt */
  SMT_ALT_IRQn          = 32,   /**< SM Timer (Alternate) Interrupt */
  MACT_ALT_IRQn         = 33,   /**< MAC Timer (Alternate) Interrupt */
  SSI1_IRQn             = 34,   /**< SSI1 Interrupt */
  GPT3A_IRQn            = 35,   /**< GPTimer 3A Interrupt */
  GPT3B_IRQn            = 36,   /**< GPTimer 3B Interrupt */
  UDMA_SW_IRQn          = 46,   /**< µDMA Software Interrupt */
  UDMA_ERR_IRQn         = 47,   /**< µDMA Error Interrupt */
  USB_IRQn              = 140,  /**< USB Interrupt */
  RF_TX_RX_IRQn         = 141,  /**< RF Tx/Rx Interrupt */
  RF_ERR_IRQn           = 142,  /**< RF Error Interrupt */
  AES_IRQn              = 143,  /**< AES Interrupt */
  PKA_IRQn              = 144,  /**< PKA Interrupt */
  SMT_IRQn              = 145,  /**< SM Timer Interrupt */
  MACT_IRQn             = 146   /**< MAC Timer Interrupt */
} IRQn_Type;

/** @} */

/** \name Processor and Core Peripheral Section
 * @{
 */

/* Configuration of the Cortex-M3 Processor and Core Peripherals */
#define __CM3_REV               0x0200  /**< Core Revision r2p0 */
#define __MPU_PRESENT           1       /**< MPU present or not */
#define __NVIC_PRIO_BITS        3       /**< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig  0       /**< Set to 1 if different SysTick Config is used */

/** @} */

/** @} */ /* CC2538_CMSIS */

#include <core_cm3.h> /* Cortex-M3 processor and core peripherals */

#ifdef __cplusplus
}
#endif

#endif /* CC2538_CM3_H */

/**
 * @}
 * @}
 */
