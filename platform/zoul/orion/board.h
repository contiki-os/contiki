/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zoul-platforms
 * @{
 *
 * \defgroup zolertia-orion-ethernet-router Zolertia IoT Orion Ethernet Router
 *
 * The Zolertia Orion Router includes an Ethernet ENC28J60 controller with
 * active POE (power over ethernet), operating over IPv4/IP64.  It features a
 * dual RF interface (2.4GHz and 863-950MHz) with external antenna connectors,
 * a power on/off switch and programable user button.
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other peripherals
 *
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the Zolertia's
 * Orion Ethernet Router, Zoul-based
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"
/*---------------------------------------------------------------------------*/
/** \name Orion Ethernet Router LED configuration
 *
 * LEDs on the eth-gw are connected as follows:
 * - LED1 (Red)    -> PD5
 * - LED2 (Green)  -> PD4
 * - LED3 (Blue)   -> PD3
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
#define LEDS_GREEN    (1 << 4) /**< LED1 (Green) -> PD4 */
#define LEDS_BLUE     (1 << 3) /**< LED2 (Blue)  -> PD3 */
#define LEDS_RED      (1 << 5) /**< LED3 (Red)   -> PD5 */

#define LEDS_CONF_ALL (LEDS_GREEN | LEDS_BLUE | LEDS_RED)

#define LEDS_LIGHT_BLUE (LEDS_GREEN | LEDS_BLUE) /**< Green + Blue (24)       */
#define LEDS_YELLOW     (LEDS_GREEN | LEDS_RED)  /**< Green + Red  (48)       */
#define LEDS_PURPLE     (LEDS_BLUE  | LEDS_RED)  /**< Blue + Red   (40)       */
#define LEDS_WHITE      LEDS_ALL                 /**< Green + Blue + Red (56) */

/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS        1
/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup for D+ is not included in this platform
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
 * On the eth-gw, the UARTs are connected to the following ports/pins:
 *
 * - UART0:
 *   - RX:  PA0, connected to CP2104 serial-to-usb converter TX pin
 *   - TX:  PA1, connected to CP2104 serial-to-usb converter RX pin
 * - UART1:
 *   - RX:  PC1
 *   - TX:  PC0
 *   - CTS: not used
 *   - RTS: not used
 *
 * We configure the port to use UART0 and UART1, CTS/RTS only for UART1,
 * both without a HW pull-up resistor.
 * UART0 is not exposed anywhere, UART1 pins are exposed over the JP9 connector.
 * @{
 */
#define UART0_RX_PORT            GPIO_A_NUM
#define UART0_RX_PIN             0
#define UART0_TX_PORT            GPIO_A_NUM
#define UART0_TX_PIN             1

#define UART1_RX_PORT            GPIO_C_NUM
#define UART1_RX_PIN             1
#define UART1_TX_PORT            GPIO_C_NUM
#define UART1_TX_PIN             0
#define UART1_CTS_PORT           (-1)
#define UART1_CTS_PIN            (-1)
#define UART1_RTS_PORT           (-1)
#define UART1_RTS_PIN            (-1)
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Zolertia Orion Router button configuration
 *
 * Buttons on the eth-gw are connected as follows:
 * - BUTTON_USER  -> PA3, S1 user button, shared with bootloader
 * - BUTTON_RESET -> RESET_N line
 * @{
 */
/** BUTTON_USER -> PA3 */
#define BUTTON_USER_PORT       GPIO_A_NUM
#define BUTTON_USER_PIN        3
#define BUTTON_USER_VECTOR     GPIO_A_IRQn

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON    1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs. There pins are suggested as they can be changed, but note that only
 * pins from PA can be configured as ADC.
 *
 * The Zolertia eth-gw, as it is, only allows 3.3VDC sensors.
 *
 * The internal ADC reference is 1190mV, use either a voltage divider as input, 
 * or a different voltage reference, like AVDD5 or other externally (AIN7 or 
 * AIN6).
 *
 * The ADC1 is exposed over the JP9 connector
 * @{
 */
#define ADC_SENSORS_PORT         GPIO_A_NUM /**< ADC GPIO control port */
#define ADC_SENSORS_ADC1_PIN     2          /**< ADC1 to PA2, 3V3      */
#define ADC_SENSORS_ADC2_PIN     4          /**< ADC2 to PA4, 3V3      */
#define ADC_SENSORS_ADC3_PIN     5          /**< ADC3 to PA5, 3V3      */
#define ADC_SENSORS_ADC4_PIN     6          /**< ADC4 to PA6, 3V3      */
#define ADC_SENSORS_ADC5_PIN     (-1)       /**< Not used              */
#define ADC_SENSORS_ADC6_PIN     (-1)       /**< Not used              */
#define ADC_SENSORS_MAX          4          /**< PA2, PA4, PA5, PA6    */


/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI0) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI0) lines,
 * reserved exclusively for the CC1200 RF transceiver.
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI0_CLK_PORT             GPIO_B_NUM
#define SPI0_CLK_PIN              2
#define SPI0_TX_PORT              GPIO_B_NUM
#define SPI0_TX_PIN               1
#define SPI0_RX_PORT              GPIO_B_NUM
#define SPI0_RX_PIN               3
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI1) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI1) lines,
 * reserved exclusively for the ENC28J60 ethernet module.  These pins should not
 * be used for other use, unless you really know what you are doing
 * TX -> MOSI, RX -> MISO
 * @{
 */
#define SPI1_CLK_PORT            GPIO_C_NUM
#define SPI1_CLK_PIN             4
#define SPI1_TX_PORT             GPIO_C_NUM
#define SPI1_TX_PIN              5
#define SPI1_RX_PORT             GPIO_C_NUM
#define SPI1_RX_PIN              6
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name I2C configuration
 *
 * As default there is not a default pin assignment for I2C, change this values 
 * accordingly if mapping either pin to the I2C controller.
 * @{
 */
#define I2C_SCL_PORT             GPIO_C_NUM
#define I2C_SCL_PIN              3
#define I2C_SDA_PORT             GPIO_C_NUM
#define I2C_SDA_PIN              2
#define I2C_INT_PORT             GPIO_D_NUM
#define I2C_INT_PIN              2
#define I2C_INT_VECTOR           GPIO_D_IRQn
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Dual RF interface support
 *
 * Enables support for dual band operation (both CC1200 and 2.4GHz enabled).
 * @{
 */
#define REMOTE_DUAL_RF_ENABLED 1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name CC1200 configuration
 *
 * These values configure the required pins to drive the CC1200
 * None of the following pins are exposed to any connector, kept for internal
 * use only
 * @{
 */
#define CC1200_SPI_INSTANCE         0
#define CC1200_SPI_SCLK_PORT        SPI0_CLK_PORT
#define CC1200_SPI_SCLK_PIN         SPI0_CLK_PIN
#define CC1200_SPI_MOSI_PORT        SPI0_TX_PORT
#define CC1200_SPI_MOSI_PIN         SPI0_TX_PIN
#define CC1200_SPI_MISO_PORT        SPI0_RX_PORT
#define CC1200_SPI_MISO_PIN         SPI0_RX_PIN
#define CC1200_SPI_CSN_PORT         GPIO_B_NUM
#define CC1200_SPI_CSN_PIN          5
#define CC1200_GDO0_PORT            GPIO_B_NUM
#define CC1200_GDO0_PIN             4
#define CC1200_GDO2_PORT            GPIO_B_NUM
#define CC1200_GDO2_PIN             0
#define CC1200_RESET_PORT           GPIO_C_NUM
#define CC1200_RESET_PIN            7
#define CC1200_GPIOx_VECTOR         GPIO_B_IRQn
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Ethernet ENC28J60 configuration
 *
 * These values configure the required pins to drive an external Ethernet
 * module.  The implementation can be SPI or GPIO-based, for the first the SPI1
 * controller should be used
 * @{
 */
#define ETH_SPI_INSTANCE           1
#define ETH_SPI_CLK_PORT           SPI1_CLK_PORT
#define ETH_SPI_CLK_PIN            SPI1_CLK_PIN
#define ETH_SPI_MOSI_PORT          SPI1_TX_PORT
#define ETH_SPI_MOSI_PIN           SPI1_TX_PIN
#define ETH_SPI_MISO_PORT          SPI1_RX_PORT
#define ETH_SPI_MISO_PIN           SPI1_RX_PIN
#define ETH_SPI_CSN_PORT           GPIO_A_NUM
#define ETH_SPI_CSN_PIN            7
#define ETH_INT_PORT               GPIO_D_NUM
#define ETH_INT_PIN                0
#define ETH_RESET_PORT             GPIO_D_NUM
#define ETH_RESET_PIN              1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name On-board external WDT
 * The Orion Ethernet Router has an external WDT and battery monitor, which
 * adds more robustness and prevents the mote to run wild if any unexpected
 * problem shows-up.
 * The external WDT requires a short pulse (<1ms) to be sent before a 2-second
 * period.  The battery monitor keeps the device in Reset if the voltage input
 * is lower than 2.5V.
 * @{
 */
#define EXT_WDT_PORT                GPIO_D_NUM
#define EXT_WDT_PIN                 5
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "Zolertia Orion Ethernet Router"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */

