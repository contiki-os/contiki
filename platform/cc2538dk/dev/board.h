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
/** \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-smartrf SmartRF06EB Peripherals
 *
 * Defines related to the SmartRF06EB
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other SmartRF peripherals
 *
 * Notably, PC0 is used to drive LED1 as well as the USB D+ pullup. Therefore
 * when USB is enabled, LED1 can not be driven by firmware.
 *
 * This file can be used as the basis to configure other platforms using the
 * cc2538 SoC.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the TI
 * SmartRF06EB
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"
/*---------------------------------------------------------------------------*/
/** \name SmartRF LED configuration
 *
 * LEDs on the SmartRF06 (EB and BB) are connected as follows:
 * - LED1 (Red)    -> PC0
 * - LED2 (Yellow) -> PC1
 * - LED3 (Green)  -> PC2
 * - LED4 (Orange) -> PC3
 *
 * LED1 shares the same pin with the USB pullup
 * @{
 */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_RED
#undef LEDS_CONF_ALL

#define LEDS_YELLOW    2 /**< LED2 (Yellow) -> PC1 */
#define LEDS_GREEN     4 /**< LED3 (Green)  -> PC2 */
#define LEDS_ORANGE    8 /**< LED4 (Orange) -> PC3 */

#if USB_SERIAL_CONF_ENABLE
#define LEDS_CONF_ALL 14
#define LEDS_RED LEDS_ORANGE
#else
#define LEDS_CONF_ALL 15
#define LEDS_RED       1 /**< LED1 (Red)    -> PC0 */
#endif

/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS        1
/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup is driven by PC0 and is shared with LED1
 */
#define USB_PULLUP_PORT          GPIO_C_BASE
#define USB_PULLUP_PIN           0
#define USB_PULLUP_PIN_MASK      (1 << USB_PULLUP_PIN)
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART configuration
 *
 * On the SmartRF06EB, the UART (XDS back channel) is connected to the
 * following ports/pins
 * - RX:  PA0
 * - TX:  PA1
 * - CTS: PB0 (Can only be used with UART1)
 * - RTS: PD3 (Can only be used with UART1)
 *
 * We configure the port to use UART0. To use UART1, change UART_CONF_BASE
 * @{
 */
#define UART_CONF_BASE           UART_0_BASE

#define UART_RX_PORT             GPIO_A_NUM
#define UART_RX_PIN              0

#define UART_TX_PORT             GPIO_A_NUM
#define UART_TX_PIN              1

#define UART_CTS_PORT            GPIO_B_NUM
#define UART_CTS_PIN             0

#define UART_RTS_PORT            GPIO_D_NUM
#define UART_RTS_PIN             3
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SmartRF Button configuration
 *
 * Buttons on the SmartRF06 are connected as follows:
 * - BUTTON_SELECT -> PA3
 * - BUTTON_LEFT -> PC4
 * - BUTTON_RIGHT -> PC5
 * - BUTTON_UP -> PC6
 * - BUTTON_DOWN -> PC7
 * @{
 */
/** BUTTON_SELECT -> PA3 */
#define BUTTON_SELECT_PORT_NO    GPIO_A_NUM
#define BUTTON_SELECT_PIN        3
#define BUTTON_SELECT_PORT       GPIO_A_BASE
#define BUTTON_SELECT_PIN_MASK   (1 << BUTTON_SELECT_PIN)
#define BUTTON_SELECT_VECTOR     NVIC_INT_GPIO_PORT_A

/** BUTTON_LEFT -> PC4 */
#define BUTTON_LEFT_PORT_NO      GPIO_C_NUM
#define BUTTON_LEFT_PIN          4
#define BUTTON_LEFT_PORT         GPIO_C_BASE
#define BUTTON_LEFT_PIN_MASK     (1 << BUTTON_LEFT_PIN)
#define BUTTON_LEFT_VECTOR       NVIC_INT_GPIO_PORT_C

/** BUTTON_RIGHT -> PC5 */
#define BUTTON_RIGHT_PORT_NO     GPIO_C_NUM
#define BUTTON_RIGHT_PIN         5
#define BUTTON_RIGHT_PORT        GPIO_C_BASE
#define BUTTON_RIGHT_PIN_MASK    (1 << BUTTON_RIGHT_PIN)
#define BUTTON_RIGHT_VECTOR      NVIC_INT_GPIO_PORT_C

/** BUTTON_UP -> PC6 */
#define BUTTON_UP_PORT_NO        GPIO_C_NUM
#define BUTTON_UP_PIN            6
#define BUTTON_UP_PORT           GPIO_C_BASE
#define BUTTON_UP_PIN_MASK       (1 << BUTTON_UP_PIN)
#define BUTTON_UP_VECTOR         NVIC_INT_GPIO_PORT_C

/** BUTTON_DOWN -> PC7 */
#define BUTTON_DOWN_PORT_NO      GPIO_C_NUM
#define BUTTON_DOWN_PIN          7
#define BUTTON_DOWN_PORT         GPIO_C_BASE
#define BUTTON_DOWN_PIN_MASK     (1 << BUTTON_DOWN_PIN)
#define BUTTON_DOWN_VECTOR       NVIC_INT_GPIO_PORT_C

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON      1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI configuration
 *
 * These values configure which CC2538 pins to use for the SPI lines.
 * @{
 */
#define CC2538_SPI_CLK_PORT_NUM  GPIO_A_NUM
#define CC2538_SPI_CLK_PIN_NUM   2
#define CC2538_SPI_MOSI_PORT_NUM GPIO_A_NUM
#define CC2538_SPI_MOSI_PIN_NUM  4
#define CC2538_SPI_MISO_PORT_NUM GPIO_A_NUM
#define CC2538_SPI_MISO_PIN_NUM  5
#define CC2538_SPI_SEL_PORT_NUM  GPIO_B_NUM
#define CC2538_SPI_SEL_PIN_NUM   5
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "TI SmartRF06 + cc2538EM"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
