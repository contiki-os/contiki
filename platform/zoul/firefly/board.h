/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia
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
 * \defgroup firefly Firefly platform
 *
 * The Zolertia Firefly is the most down-to-core development platform, exposing
 * the Zoul core functionalities and features, with a slick design to allow a
 * flexible and easier user experience.
 *
 * Defines related to the Firefly platform: a Zoul-based breakout board
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other peripherals
 *
 * This file can be used as the basis to configure other platforms using the
 * cc2538 SoC.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the Zolertia's
 * Firefly platform, Zoul-based
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"
/*---------------------------------------------------------------------------*/
/** \name Connector headers
 *
 * The Firefly features two 2.54 mm header rows over which exposes the following
 * pins (facing up, Zolertia logo above and Micro-USB connector below):
 * -----------------------------+---+---+--------------------------------------
 * PIN_NAME                     |JP3|JP2|   PIN_NAME
 * -----------------------------+---+---+--------------------------------------
 * PB5/CC1200.CS                |-01|01-|   LED1/PD5
 * PB2/SPI0.SCLK/CC1200.SCLK    |-02|02-|   LED2/PD4
 * PB1/SPIO0.MOSI/CC1200.MOSI   |-03|03-|   LED3/PD3
 * PB3/SPIO0.MISO/CC1200.MISO   |-04|04-|   PD2
 * PB3/CC1200.GPIO0             |-05|05-|   PD1
 * PC0/UART1.TX                 |-06|06-|   PD0
 * PC1/UART1.RX                 |-07|07-|   AIN7/PA7
 * PC2/I2C.SDA                  |-08|08-|   AIN6/PA6
 * PC3/I2C.SCL                  |-09|09-|   ADC1/AIN5/PA5
 * PC4/SPI1.SCLK                |-10|10-|   ADC2/AIN4/PA4
 * PC5/SPI1.MOSI                |-11|11-|   BUTTON.USER/PA3
 * PC6/SPI1.MISO                |-12|12-|   ADC3/AIN2/PA2
 * USB.D+                       |-13|13-|   DGND
 * USB.D-                       |-14|14-|   D+3.3
 * ---------------------------+-+---+---+-+------------------------------------
 */
/*---------------------------------------------------------------------------*/
/** \name Firefly LED configuration
 *
 * LEDs on the Firefly are connected as follows:
 * - LED1 (Red)    -> PD5
 * - LED2 (Green)  -> PD4
 * - LED3 (Blue)   -> PD3
 *
 * LED1 pin exposed in JP2 connector
 * LED2 pin exposed in JP2 connector
 * LED3 pin exposed in JP2 connector
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
 * The USB pullup is to be enabled by an external resistor, as it is not mapped
 * to a GPIO.
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
 * On the Firefly, the UARTs are connected to the following ports/pins:
 *
 * - UART0:
 *   - RX:  PA0, connected to CP2104 serial-to-usb converter TX pin
 *   - TX:  PA1, connected to CP2104 serial-to-usb converter RX pin
 * - UART1:
 *   - RX:  PC1
 *   - TX:  PC0
 *   - CTS: not used, one suggestion however is to use PD1
 *   - RTS: not used, one suggestion however is to use PD0
 *
 * We configure the port to use UART0 and UART1, CTS/RTS only for UART1,
 * both without a HW pull-up resistor.
 * UART0 is not exposed anywhere, UART1 pins are exposed over the JP3 connector.
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
#define UART1_CTS_PORT           (-1) /**< GPIO_D_NUM */
#define UART1_CTS_PIN            (-1) /**< 1 */
#define UART1_RTS_PORT           (-1) /**< GPIO_D_NUM */
#define UART1_RTS_PIN            (-1) /**< 0 */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs. There pins are suggested as they can be changed, but note that only
 * pins from PA can be configured as ADC.
 *
 * - ADC1: up to 3.3V.
 * - ADC2: up to 3.3V.
 * - ADC3: up to 3.3V.
 * - ADC4: up to 3.3V.
 * - ADC5: up to 3.3V.
 * - ADC6: up to 3.3V, shared with user button.
 * 
 * Only ADC1 and ADC3 are enabled as default.
 *
 * The internal ADC reference is 1190mV, use either a voltage divider as input,
 * or a different voltage reference, like AVDD5 or other externally (AIN7 or
 * AIN6).
 * @{
 */
#define ADC_SENSORS_PORT         GPIO_A_NUM /**< ADC GPIO control port */

#ifndef ADC_SENSORS_CONF_ADC1_PIN
#define ADC_SENSORS_ADC1_PIN     5             /**< ADC1 to PA5  */
#else
#if ((ADC_SENSORS_CONF_ADC1_PIN != -1) && (ADC_SENSORS_CONF_ADC1_PIN != 5))
#error "ADC1 channel should be mapped to PA5 or disabled with -1"
#else
#define ADC_SENSORS_ADC1_PIN ADC_SENSORS_CONF_ADC1_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC2_PIN
#define ADC_SENSORS_ADC2_PIN     4             /**< ADC2 to PA4  */
#else
#if ((ADC_SENSORS_CONF_ADC2_PIN != -1) && (ADC_SENSORS_CONF_ADC2_PIN != 4))
#error "ADC2 channel should be mapped to PA4 or disabled with -1"
#else
#define ADC_SENSORS_ADC2_PIN ADC_SENSORS_CONF_ADC2_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC3_PIN
#define ADC_SENSORS_ADC3_PIN     2             /**< ADC3 to PA2  */
#else
#if ((ADC_SENSORS_CONF_ADC3_PIN != -1) && (ADC_SENSORS_CONF_ADC3_PIN != 2))
#error "ADC3 channel should be mapped to PA2 or disabled with -1"
#else
#define ADC_SENSORS_ADC3_PIN ADC_SENSORS_CONF_ADC3_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC4_PIN
#define ADC_SENSORS_ADC4_PIN     6             /**< ADC4 to PA6   */
#else
#if ((ADC_SENSORS_CONF_ADC4_PIN != -1) && (ADC_SENSORS_CONF_ADC4_PIN != 6))
#error "ADC4 channel should be mapped to PA6 or disabled with -1"
#else
#define ADC_SENSORS_ADC4_PIN ADC_SENSORS_CONF_ADC4_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC5_PIN
#define ADC_SENSORS_ADC5_PIN     7             /**< ADC5 to PA7   */
#else
#if ((ADC_SENSORS_CONF_ADC5_PIN != -1) && (ADC_SENSORS_CONF_ADC5_PIN != 7))
#error "ADC5 channel should be mapped to PA7 or disabled with -1"
#else
#define ADC_SENSORS_ADC5_PIN ADC_SENSORS_CONF_ADC5_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC6_PIN
#define ADC_SENSORS_ADC6_PIN     (-1)          /**< ADC6 not declared    */
#else
#define ADC_SENSORS_ADC6_PIN     3             /**< Hard-coded to PA3    */
#endif

#ifndef ADC_SENSORS_CONF_MAX
#define ADC_SENSORS_MAX          5            /**< Maximum sensors       */
#else
#define ADC_SENSORS_MAX          ADC_SENSORS_CONF_MAX
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Firefly Button configuration
 *
 * Buttons on the Firefly are connected as follows:
 * - BUTTON_USER  -> PA3, S1 user button, shared with bootloader
 * - BUTTON_RESET -> RESET_N line
 * @{
 */
/** BUTTON_USER -> PA3 */
#define BUTTON_USER_PORT       GPIO_A_NUM
#define BUTTON_USER_PIN        3
#define BUTTON_USER_VECTOR     GPIO_A_IRQn

/* Notify various examples that we have an user button.
 * If ADC6 channel is used, then disable the user button
 */
#ifdef PLATFORM_CONF_WITH_BUTTON
#if (PLATFORM_CONF_WITH_BUTTON && (ADC_SENSORS_ADC6_PIN == 3))
#error "The ADC6 (PA3) and user button cannot be enabled at the same time" 
#else
#define PLATFORM_HAS_BUTTON  (PLATFORM_CONF_WITH_BUTTON && \
                              !(ADC_SENSORS_ADC6_PIN == 3))
#endif /* (PLATFORM_CONF_WITH_BUTTON && (ADC_SENSORS_ADC6_PIN == 3)) */
#else
#define PLATFORM_HAS_BUTTON  !(ADC_SENSORS_ADC6_PIN == 3)
#endif /* PLATFORM_CONF_WITH_BUTTON */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI (SSI0) configuration
 *
 * These values configure which CC2538 pins to use for the SPI (SSI0) lines,
 * reserved exclusively for the CC1200 RF transceiver.  These pins are exposed
 * to the JP3 connector.  To disable the CC1200 and use these pins, just
 * remove the R10 resistor (0 ohm), which powers both the CC2538 and CC1200 to
 * only power the SoC.
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
 * exposed over JP3 connector.
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
 * These values configure which CC2538 pins to use for the I2C lines, exposed
 * over JP3 connector.
 * @{
 */
#define I2C_SCL_PORT             GPIO_C_NUM
#define I2C_SCL_PIN              3
#define I2C_SDA_PORT             GPIO_C_NUM
#define I2C_SDA_PIN              2
#define I2C_INT_PORT             GPIO_D_NUM
#define I2C_INT_PIN              1
#define I2C_INT_VECTOR           GPIO_D_IRQn
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Dual RF interface support
 *
 * Enables support for dual band operation (both CC1200 and 2.4GHz enabled).
 * Unlike the RE-Mote, the Firefly doesn't have a RF switch, so both interfaces
 * should be always enabled if the R10 resistor is mounted.  If only using the
 * 2.4GHz RF interface, the resistor can be removed to power-off the CC1200.
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
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "Zolertia Firefly platform"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
