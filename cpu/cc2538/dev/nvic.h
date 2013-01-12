/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-nvic cc2538 Nested Vectored Interrupt Controller
 *
 * Driver for the cc2538 NVIC controller
 * @{
 *
 * \file
 * Header file for the ARM Nested Vectored Interrupt Controller
 */
#ifndef NVIC_H_
#define NVIC_H_

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name NVIC Constants and Configuration
 * @{
 */
#define NVIC_VTABLE_IN_SRAM 0x20000000
#define NVIC_VTABLE_IN_CODE 0x00000000

#define NVIC_INTERRUPT_ENABLED  0x00000001
#define NVIC_INTERRUPT_DISABLED 0x00000000

#ifndef NVIC_CONF_VTABLE_BASE
#define NVIC_CONF_VTABLE_BASE NVIC_VTABLE_IN_CODE
#endif

#ifndef NVIC_CONF_VTABLE_OFFSET
#define NVIC_CONF_VTABLE_OFFSET 0x200000
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name NVIC Interrupt assignments
 * @{
 */
#define NVIC_INT_GPIO_PORT_A         0 /**<  GPIO port A */
#define NVIC_INT_GPIO_PORT_B         1 /**<  GPIO port B */
#define NVIC_INT_GPIO_PORT_C         2 /**<  GPIO port C */
#define NVIC_INT_GPIO_PORT_D         3 /**<  GPIO port D */
#define NVIC_INT_UART0               5 /**<  UART0 */
#define NVIC_INT_UART1               6 /**<  UART1 */
#define NVIC_INT_SSI0                7 /**<  SSI0 */
#define NVIC_INT_I2C                 8 /**<  I2C */
#define NVIC_INT_ADC                14 /**<  ADC */
#define NVIC_INT_WDT                18 /**<  Watchdog Timer */
#define NVIC_INT_GPTIMER_0A         19 /**<  GPTimer 0A */
#define NVIC_INT_GPTIMER_0B         20 /**<  GPTimer 0B */
#define NVIC_INT_GPTIMER_1A         21 /**<  GPTimer 1A */
#define NVIC_INT_GPTIMER_1B         22 /**<  GPTimer 1B */
#define NVIC_INT_GPTIMER_2A         23 /**<  GPTimer 2A */
#define NVIC_INT_GPTIMER_2B         24 /**<  GPTimer 2B */
#define NVIC_INT_ADC_CMP            25 /**<  Analog Comparator */
#define NVIC_INT_RF_RXTX_ALT        26 /**<  RF TX/RX (Alternate) */
#define NVIC_INT_RF_ERR_ALT         27 /**<  RF Error (Alternate) */
#define NVIC_INT_SYS_CTRL           28 /**<  System Control */
#define NVIC_INT_FLASH_CTRL         29 /**<  Flash memory control */
#define NVIC_INT_AES_ALT            30 /**<  AES (Alternate) */
#define NVIC_INT_PKA_ALT            31 /**<  PKA (Alternate) */
#define NVIC_INT_SM_TIMER_ALT       32 /**<  SM Timer (Alternate) */
#define NVIC_INT_MAC_TIMER_ALT      33 /**<  MAC Timer (Alternate) */
#define NVIC_INT_SSI1               34 /**<  SSI1 */
#define NVIC_INT_GPTIMER_3A         35 /**<  GPTimer 3A */
#define NVIC_INT_GPTIMER_3B         36 /**<  GPTimer 3B */
#define NVIC_INT_UDMA               46 /**<  uDMA software */
#define NVIC_INT_UDMA_ERR           47 /**<  uDMA error */
#define NVIC_INT_USB               140 /**<  USB */
#define NVIC_INT_RF_RXTX           141 /**<  RF Core Rx/Tx */
#define NVIC_INT_RF_ERR            142 /**<  RF Core Error */
#define NVIC_INT_AES               143 /**<  AES */
#define NVIC_INT_PKA               144 /**<  PKA */
#define NVIC_INT_SM_TIMER          145 /**<  SM Timer */
#define NVIC_INT_MACTIMER          146 /**<  MAC Timer */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name NVIC Register Declarations
 * @{
 */
#define NVIC_EN0                0xE000E100 /**< Interrupt 0-31 Set Enable */
#define NVIC_EN1                0xE000E104 /**< Interrupt 32-54 Set Enable */
#define NVIC_EN2                0xE000E108 /**< Interrupt 64-95 Set Enable */
#define NVIC_EN3                0xE000E10C /**< Interrupt 96-127 Set Enable */
#define NVIC_EN4                0xE000E110 /**< Interrupt 128-131 Set Enable */
#define NVIC_DIS0               0xE000E180 /**< Interrupt 0-31 Clear Enable */
#define NVIC_DIS1               0xE000E184 /**< Interrupt 32-54 Clear Enable */
#define NVIC_DIS2               0xE000E188 /**< Interrupt 64-95 Clear Enable */
#define NVIC_DIS3               0xE000E18C /**< Interrupt 96-127 Clear Enable */
#define NVIC_DIS4               0xE000E190 /**< Interrupt 128-131 Clear Enable */
#define NVIC_PEND0              0xE000E200 /**< Interrupt 0-31 Set Pending */
#define NVIC_PEND1              0xE000E204 /**< Interrupt 32-54 Set Pending */
#define NVIC_PEND2              0xE000E208 /**< Interrupt 64-95 Set Pending */
#define NVIC_PEND3              0xE000E20C /**< Interrupt 96-127 Set Pending */
#define NVIC_PEND4              0xE000E210 /**< Interrupt 128-131 Set Pending */
#define NVIC_UNPEND0            0xE000E280 /**< Interrupt 0-31 Clear Pending */
#define NVIC_UNPEND1            0xE000E284 /**< Interrupt 32-54 Clear Pending */
#define NVIC_UNPEND2            0xE000E288 /**< Interrupt 64-95 Clear Pending */
#define NVIC_UNPEND3            0xE000E28C /**< Interrupt 96-127 Clear Pending */
#define NVIC_UNPEND4            0xE000E290 /**< Interrupt 128-131 Clear Pending */
#define NVIC_ACTIVE0            0xE000E300 /**< Interrupt 0-31 Active Bit */
#define NVIC_ACTIVE1            0xE000E304 /**< Interrupt 32-54 Active Bit */
#define NVIC_ACTIVE2            0xE000E308 /**< Interrupt 64-95 Active Bit */
#define NVIC_ACTIVE3            0xE000E30C /**< Interrupt 96-127 Active Bit */
#define NVIC_ACTIVE4            0xE000E310 /**< Interrupt 128-131 Active Bit */
#define NVIC_PRI0               0xE000E400 /**< Interrupt 0-3 Priority */
#define NVIC_PRI1               0xE000E404 /**< Interrupt 4-7 Priority */
#define NVIC_PRI2               0xE000E408 /**< Interrupt 8-11 Priority */
#define NVIC_PRI3               0xE000E40C /**< Interrupt 12-15 Priority */
#define NVIC_PRI4               0xE000E410 /**< Interrupt 16-19 Priority */
#define NVIC_PRI5               0xE000E414 /**< Interrupt 20-23 Priority */
#define NVIC_PRI6               0xE000E418 /**< Interrupt 24-27 Priority */
#define NVIC_PRI7               0xE000E41C /**< Interrupt 28-31 Priority */
#define NVIC_PRI8               0xE000E420 /**< Interrupt 32-35 Priority */
#define NVIC_PRI9               0xE000E424 /**< Interrupt 36-39 Priority */
#define NVIC_PRI10              0xE000E428 /**< Interrupt 40-43 Priority */
#define NVIC_PRI11              0xE000E42C /**< Interrupt 44-47 Priority */
#define NVIC_PRI12              0xE000E430 /**< Interrupt 48-51 Priority */
#define NVIC_PRI13              0xE000E434 /**< Interrupt 52-53 Priority */
#define NVIC_PRI14              0xE000E438 /**< Interrupt 56-59 Priority */
#define NVIC_PRI15              0xE000E43C /**< Interrupt 60-63 Priority */
#define NVIC_PRI16              0xE000E440 /**< Interrupt 64-67 Priority */
#define NVIC_PRI17              0xE000E444 /**< Interrupt 68-71 Priority */
#define NVIC_PRI18              0xE000E448 /**< Interrupt 72-75 Priority */
#define NVIC_PRI19              0xE000E44C /**< Interrupt 76-79 Priority */
#define NVIC_PRI20              0xE000E450 /**< Interrupt 80-83 Priority */
#define NVIC_PRI21              0xE000E454 /**< Interrupt 84-87 Priority */
#define NVIC_PRI22              0xE000E458 /**< Interrupt 88-91 Priority */
#define NVIC_PRI23              0xE000E45C /**< Interrupt 92-95 Priority */
#define NVIC_PRI24              0xE000E460 /**< Interrupt 96-99 Priority */
#define NVIC_PRI25              0xE000E464 /**< Interrupt 100-103 Priority */
#define NVIC_PRI26              0xE000E468 /**< Interrupt 104-107 Priority */
#define NVIC_PRI27              0xE000E46C /**< Interrupt 108-111 Priority */
#define NVIC_PRI28              0xE000E470 /**< Interrupt 112-115 Priority */
#define NVIC_PRI29              0xE000E474 /**< Interrupt 116-119 Priority */
#define NVIC_PRI30              0xE000E478 /**< Interrupt 120-123 Priority */
#define NVIC_PRI31              0xE000E47C /**< Interrupt 124-127 Priority */
#define NVIC_PRI32              0xE000E480 /**< Interrupt 128-131 Priority */
#define NVIC_PRI33              0xE000E480 /**< Interrupt 132-135 Priority */
#define NVIC_PRI34              0xE000E484 /**< Interrupt 136-139 Priority */
#define NVIC_PRI35              0xE000E488 /**< Interrupt 140-143 Priority */
#define NVIC_PRI36              0xE000E48c /**< Interrupt 144-147 Priority */
/** @} */
/*---------------------------------------------------------------------------*/
/** \brief Initialises the NVIC driver */
void nvic_init();

/**
 * \brief Enables interrupt intr
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 *
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to enable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
void nvic_interrupt_enable(uint32_t intr);

/**
 * \brief Disables interrupt intr
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 *
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to disable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
void nvic_interrupt_disable(uint32_t intr);

/**
 * \brief Enables interrupt intr if v > 0
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 * \param v 0: No effect, 1: Enables the interrupt
 *
 * This function is useful to restore an interrupt to a state previously
 * saved by nvic_interrupt_en_save. Thus, if when nvic_interrupt_en_save was
 * called the interrupt was enabled, this function will re-enabled it.
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to disable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
void nvic_interrupt_en_restore(uint32_t intr, uint8_t v);

/**
 * \brief Checks the interrupt enabled status for intr
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 * \return 1: Enabled, 0: Disabled
 *
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to disable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
uint8_t nvic_interrupt_en_save(uint32_t intr);

/**
 * \brief Sets intr to pending
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 *
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to enable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
void nvic_interrupt_pend(uint32_t intr);

/**
 * \brief Sets intr to no longer pending
 * \param intr The interrupt number (NOT the vector number). For example,
 * GPIO Port A interrupt is 0, not 16.
 *
 * Possible values for the \e intr param are defined as NVIC_INT_xyz. For
 * instance, to disable the GPIO Port A interrupt, pass NVIC_INT_GPIO_PORT_A
 */
void nvic_interrupt_unpend(uint32_t intr);

#endif /* NVIC_H_ */

/**
 * @}
 * @}
 */
