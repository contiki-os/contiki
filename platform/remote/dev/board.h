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
/** \addtogroup remote
 * @{
 *
 * \defgroup remote-peripherals Re-Mote Peripherals
 *
 * Defines related to the Re-Mote
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other Re-Mote peripherals
 *
 * This file can be used as the basis to configure other platforms using the
 * cc2538 SoC.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the Zolertia's
 * Re-Mote platform, cc2538-based
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"
/*---------------------------------------------------------------------------*/
/** \name Re-Mote LED configuration
 *
 * LEDs on the Re-mote are connected as follows:
 * - LED1 (Red)    -> PD2
 * - LED2 (Blue)   -> PC3
 * - LED3 (Green)  -> PD5
 *
 * LED1 routed also to JP5 connector
 * LED2 shares the same pin with Watchdog WDI pulse and routed to JP8 connector
 * LED3 routed also to JP5 connector
 * @{
 */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_BLUE
#undef LEDS_RED
#undef LEDS_CONF_ALL

/* In leds.h the LEDS_BLUE is defined by LED_YELLOW definition */
#define LEDS_GREEN    1  /**< LED1 (Green) -> PD5 */
#define LEDS_BLUE     2  /**< LED2 (Blue)  -> PC3 */
#define LEDS_RED      4  /**< LED3 (Red)   -> PD2 */

#define LEDS_CONF_ALL 7

#define LEDS_LIGHT_BLUE (LEDS_GREEN | LEDS_BLUE) /**< Green + Blue (3) */
#define LEDS_YELLOW     (LEDS_GREEN | LEDS_RED) /**< Green + Red (5) */
#define LEDS_PURPLE     (LEDS_BLUE | LEDS_RED) /**< Blue + Red (6) */
#define LEDS_WHITE      LEDS_ALL /**< Green + Blue + Red (7) */

/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS        1
/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup is enabled by an external resistor, not mapped to a GPIO
 */
#ifdef USB_PULLUP_PORT
#undef USB_PULLUP_PORT
#endif
#ifdef USB_PULLUP_PIN
#undef USB_PULLUP_PIN
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART configuration
 *
 * On the Re-Mote, the UART is connected to the following ports/pins
 * - UART0:
 *   - RX:  PA0
 *   - TX:  PA1
 * - UART1:
 *   - RX:  PC6
 *   - TX:  PC5
 *   - CTS: 
 *   - RTS: 
 * We configure the port to use UART0 and UART1, CTS/RTS only for UART1,
 * both without a HW pull-up resistor
 * @{
 */
#define UART0_RX_PORT            GPIO_A_NUM
#define UART0_RX_PIN             0
#define UART0_TX_PORT            GPIO_A_NUM
#define UART0_TX_PIN             1

#define UART1_RX_PORT            GPIO_C_NUM
#define UART1_RX_PIN             6
#define UART1_TX_PORT            GPIO_C_NUM
#define UART1_TX_PIN             5
#define UART1_CTS_PORT           GPIO_C_NUM
#define UART1_CTS_PIN            1
#define UART1_RTS_PORT           GPIO_C_NUM
#define UART1_RTS_PIN            2
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Re-Mote Button configuration
 *
 * Buttons on the Re-Mote are connected as follows:
 * - BUTTON_USER  -> PA3, S1 user button, shared with bootloader
 * - BUTTON_RESET -> RESET_N line, S2 reset both CC2538 and CoP
 * - BUTTON_VBAT  -> Power switch, not mounted by default
 * @{
 */
/** BUTTON_USER -> PA3 */
#define BUTTON_USER_PORT       GPIO_A_NUM
#define BUTTON_USER_PIN        3
#define BUTTON_USER_VECTOR     NVIC_INT_GPIO_PORT_A

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON      1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs. By default the Re-Mote allows two out-of-the-box ADC ports with a 
 * phidget-like 3-pin connector (GND/3V3/ADC)
 *
 * ADC inputs can only be on port A.
 * @{
 */
#define ADC_PHIDGET_PORT         GPIO_A_NUM /**< Phidget GPIO control port */
#define ADC_PHIDGET_ADC2_PIN     6 /**< ADC2 to PA6, 3V3 */
#define ADC_PHIDGET_ADC3_PIN     7 /**< ADC3 to PA7, 3V3 */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI0) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI0) lines,
 * shared with the CC1120 RF transceiver
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI0_CLK_PORT             GPIO_D_NUM
#define SPI0_CLK_PIN              1
#define SPI0_TX_PORT              GPIO_D_NUM
#define SPI0_TX_PIN               0
#define SPI0_RX_PORT              GPIO_C_NUM
#define SPI0_RX_PIN               4
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI1) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI1) lines,
 * shared with the microSD, not routed anywhere.
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI1_CLK_PORT            GPIO_B_NUM
#define SPI1_CLK_PIN             5
#define SPI1_TX_PORT             GPIO_C_NUM
#define SPI1_TX_PIN              7
#define SPI1_RX_PORT             GPIO_A_NUM
#define SPI1_RX_PIN              4
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name I2C configuration
 *
 * These values configure which CC2538 pins to use for the I2C lines, shared
 * with the TMP102 built-in temperature sensor
 * @{
 */
#define I2C_SCL_PORT             GPIO_B_NUM
#define I2C_SCL_PIN              1
#define I2C_SDA_PORT             GPIO_B_NUM
#define I2C_SDA_PIN              0
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Antenna switch configuration
 *
 * These values configure the required pin to drive the antenna switch, to 
 * use either the built-in ceramic antenna or an external one over the uFL
 * connector
 * - Internal antenna: LOW
 * - External antenna: HIGH
 * @{
 */
#define ANTENNA_2_4GHZ_SW_PORT GPIO_D_NUM
#define ANTENNA_2_4GHZ_SW_PIN  4
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name CC1120/CC1200 configuration
 *
 * These values configure the required pins to drive the CC1120/CC1200
 * @{
 */
#define CC1120_SPI_SCLK_PORT        SPI0_CLK_PORT
#define CC1120_SPI_SCLK_PIN         SPI0_CLK_PIN
#define CC1120_SPI_MOSI_PORT        SPIO0_TX_PORT
#define CC1120_SPI_MOSI_PIN         SPIO0_TX_PIN
#define CC1120_SPI_MISO_PORT        SPIO0_RX_PORT
#define CC1120_SPI_MISO_PIN         SPIO0_RX_PIN
#define CC1120_SPI_CSN_PORT         GPIO_D_NUM
#define CC1120_SPI_CSN_PIN          3
#define CC1120_GDO0_PORT            GPIO_B_NUM
#define CC1120_GDO0_PIN             4
#define CC1120_GDO2_PORT            GPIO_B_NUM
#define CC1120_GDO2_PIN             3
#define CC1120_RESET_PORT           GPIO_B_NUM
#define CC1120_RESET_PIN            2
#define CC1120_GPIO0_VECTOR         NVIC_INT_GPIO_PORT_B
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name microSD configuration
 *
 * These values configure the required pins to drive the built-in microSD 
 * external module, to be used with SSI1
 * @{
 */
#define USD_CLK_PORT             SPI1_CLK_PORT
#define USD_CLK_PIN              SPI1_CLK_PIN
#define USD_MOSI_PORT            SPI1_TX_PORT
#define USD_MOSI_PIN             SPI1_TX_PIN
#define USD_MISO_PORT            SPI1_RX_PORT
#define USD_MISO_PIN             SPI1_RX_PIN
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "Zolertia Re-Mote platform"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
