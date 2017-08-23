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
 * \defgroup remote-reva RE-Mote platform revision A
 *
 * The RE-Mote was designed jointly with universities and industry partners in
 * RERUM European project, to ease the development of private and secure
 * applications for IoT and Smart City applications.  The RE-Mote packs several
 * on-board resources, like a RTC, external WDT, Micro-SD, RF switch and a
 * Shutdown mode to reduce its power consumption down to 300nA.
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other RE-Mote revision A peripherals
 *
 * This file can be used as the basis to configure other platforms using the
 * cc2538 SoC.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the Zolertia's
 * RE-Mote platform (revision A), cc2538-based
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
 * The RE-Mote features two 2.54 mm header rows over which exposes the following
 * pins (facing up, Zolertia/RERUM logo above, buttons and micro USB at bottom):
 * ----------------------+---+---+---------------------------------------------
 * PIN_NAME              |JP6|JP5|   PIN_NAME
 * ----------------------+---+---+---------------------------------------------
 * LED1/EXT_WDT/PD5      |-01|18-|   PC6/SPI1.MISO/USD.MISO
 * LED2/UART1.CTS/PD4    |-02|17-|   PC5/SPI1.MOSI/USD.MOSI
 * LED3/UART1.RTS/PD3    |-03|16-|   PC4/SPI1.SCLK/USD.SCLK
 * UART0.RX/PA0          |-04|15-|   PA3/BUTTON.USER
 * UART0.TX/PA1          |-05|14-|   RESET/JTAG.RESET/BUTTON.RESET
 * SHUTDOWN_ENABLE/PD1   |-06|13-|   DGND
 * RTC.SDA/I2C.SDA/PC2   |-07|12-|   D+3.3
 * RTC.SCL/I2C.SCL/PC3   |-08|11-|   PA5/AIN5/ADC1
 * DGND                  |-09|10-|   PA4/RTC_INT1/AIN4/ADC2
 * D+3.3                 |-10|09-|   DGND
 * USD.CS/AIN7/PA7       |-11|08-|   D+5.1
 * SHUTDOWN_DONE/PD0     |-12|07-|   PA2/AIN2/ADC3
 * UART1.RX/PC1          |-13|06-|   JTAG.TMS
 * UART1.TX/PC0          |-14|05-|   JTAG.TCK
 * DGND                  |-15|04-|   PB7/JTAG.TDO
 * D+3.3                 |-16|03-|   PB6/JTAG.TDI
 * DGND                  |-17|02-|   PS+EXT
 * +VBAT                 |-18|01-|   DGND
 * ----------------------+---+---+---------------------------------------------
 */
/*---------------------------------------------------------------------------*/
/** \name RE-Mote LED configuration
 *
 * LEDs on the RE-Mote are connected as follows:
 * - LED1 (Red)    -> PD5
 * - LED2 (Green)  -> PD4
 * - LED3 (Blue)   -> PD3
 *
 * LED1 pin shared with EXT_WDT and exposed in JP6 connector
 * LED2 pin shared with UART1 CTS, pin exposed in JP6 connector
 * LED3 pin shared with UART1 RTS, exposed in JP6 connector
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
 * On the RE-Mote, the UARTs are connected to the following ports/pins:
 *
 * - UART0:
 *   - RX:  PA0, connected to CP2104 serial-to-usb converter TX pin
 *   - TX:  PA1, connected to CP2104 serial-to-usb converter RX pin
 * - UART1:
 *   - RX:  PC1
 *   - TX:  PC0
 *   - CTS: PD4, shared with LED2 (Green), disabled as default
 *   - RTS: PD3, shared with LED3 (Blue), disabled as default
 *
 * We configure the port to use UART0 and UART1, CTS/RTS only for UART1,
 * both without a HW pull-up resistor
 * UART0 and UART1 pins are exposed over the JP6 connector
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
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs. By default the RE-Mote allows two out-of-the-box ADC ports with a
 * phidget-like 3-pin connector (GND/VDD/ADC)
 *
 * The RE-Mote allows both 3.3V and 5V analogue sensors as follow:
 *
 * - ADC1: up to 3.3V.
 * - ADC2: up to 3.3V, shared with RTC_INT
 * - ADC3: up to 5V, by means of a 2/3 voltage divider.
 *
 * Also there are other ADC channels shared by default with Micro SD card and
 * user button implementations: 
 * - ADC4: up to 3.3V.
 * - ADC5: up to 3.3V.
 * - ADC6: up to 3.3V.
 *
 * ADC inputs can only be on port A.
 * All ADCx are exposed in JP5 connector, but only ADC1 and ADC3 have GND and
 * VDD (3/5V) pins next to it, so these can be exposed into a 3-pin phidget-like
 * connector, for ADC2 either solder a wire to connect, or use a 4-pin connector
 * to expose both ADC1 and ADC2 in a single connector, but this will leave no
 * space for a ADC3 connector.
 * The internal ADC reference is 1190mV, use either a voltage divider as input,
 * or a different voltage reference, like AVDD5 or other externally (AIN7), but
 * note the PA7 is shared with the Micro-SD CSn pin, likewise for PA6 (AIN6)
 * shared witht the Micro-SD select pin
 * To use the ADC2 pin, remove the resistor on the Zoul's PA4 pin (JP1, pin 10)
 * and enable below (replace -1 with 4).
 * @{
 */
#define ADC_SENSORS_PORT         GPIO_A_NUM /**< ADC GPIO control port */

#ifndef ADC_SENSORS_CONF_ADC1_PIN
#define ADC_SENSORS_ADC1_PIN     5             /**< ADC1 to PA5, 3V3    */
#else
#if ((ADC_SENSORS_CONF_ADC1_PIN != -1) && (ADC_SENSORS_CONF_ADC1_PIN != 5))
#error "ADC1 channel should be mapped to PA5 or disabled with -1"
#else
#define ADC_SENSORS_ADC1_PIN ADC_SENSORS_CONF_ADC1_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC3_PIN
#define ADC_SENSORS_ADC3_PIN     2             /**< ADC3 to PA2, 5V     */
#else
#if ((ADC_SENSORS_CONF_ADC3_PIN != -1) && (ADC_SENSORS_CONF_ADC3_PIN != 2))
#error "ADC3 channel should be mapped to PA2 or disabled with -1"
#else
#define ADC_SENSORS_ADC3_PIN ADC_SENSORS_CONF_ADC3_PIN
#endif
#endif

#ifndef ADC_SENSORS_CONF_ADC2_PIN
#define ADC_SENSORS_ADC2_PIN     (-1)          /**< ADC2 no declared    */
#else
#define ADC_SENSORS_ADC2_PIN     4             /**< Hard-coded to PA4    */
#endif

#ifndef ADC_SENSORS_CONF_ADC4_PIN
#define ADC_SENSORS_ADC4_PIN     (-1)          /**< ADC4 not declared    */
#else
#define ADC_SENSORS_ADC4_PIN     6             /**< Hard-coded to PA6    */
#endif

#ifndef ADC_SENSORS_CONF_ADC5_PIN
#define ADC_SENSORS_ADC5_PIN     (-1)          /**< ADC5 not declared    */
#else
#define ADC_SENSORS_ADC5_PIN     7             /**< Hard-coded to PA7    */
#endif

#ifndef ADC_SENSORS_CONF_ADC6_PIN
#define ADC_SENSORS_ADC6_PIN     (-1)             /**< ADC6 not declared    */
#else
#define ADC_SENSORS_ADC6_PIN     3             /**< Hard-coded to PA3    */
#endif

#ifndef ADC_SENSORS_CONF_MAX
#define ADC_SENSORS_MAX          2             /**< Maximum sensors    */
#else
#define ADC_SENSORS_MAX          ADC_SENSORS_CONF_MAX
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RE-Mote Button configuration
 *
 * Buttons on the RE-Mote are connected as follows:
 * - BUTTON_USER  -> PA3, S1 user button, shared with bootloader and RTC_INT1
 * - BUTTON_RESET -> RESET_N line, S2 reset both CC2538 and CoP
 * - BUTTON_PIC1W -> shared with SHUTDOWN_ENABLE, not mounted.
 * @{
 */
/** BUTTON_USER -> PA3 */
#define BUTTON_USER_PORT       GPIO_A_NUM
#define BUTTON_USER_PIN        3
#define BUTTON_USER_VECTOR     NVIC_INT_GPIO_PORT_A

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
 * reserved exclusively for the CC1200 RF transceiver.  These pins are not
 * exposed to any connector, and should be avoid to use it.
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
 * shared with the microSD and exposed over JP5 connector.
 * It is advisable to use a CSn pin other than the Micro-SD's.
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
 * over JP6 connector, also available as testpoints T2 (PC2) and T3 (PC3).
 * The I2C bus is shared with the on-board RTC.
 * The I2C is exposed over the JP6 header, using a 5-pin connector with 2.54 mm
 * spacing, providing also D+3.3V, GND and a generic pin that can be used as an
 * interrupt pin
 * @{
 */
#define I2C_SCL_PORT             GPIO_C_NUM
#define I2C_SCL_PIN              3
#define I2C_SDA_PORT             GPIO_C_NUM
#define I2C_SDA_PIN              2
#define I2C_INT_PORT             GPIO_D_NUM
#define I2C_INT_PIN              1
#define I2C_INT_VECTOR           NVIC_INT_GPIO_PORT_D
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Antenna switch configuration
 *
 * These values configure the required pin to drive the RF antenna switch, to
 * either enable the sub-1Ghz RF interface (power-up the CC1200) or the 2.4GHz
 * RF interface of the CC2538, both alternatively routed to a RP-SMA connector
 * to allow using an external antenna for both cases.
 *
 * Note it is also possible to enable both RF interfaces at the same time, by
 * switching On the sub-1GHz RF interface, and placing an 0Ohm resistor (R19),
 * to select between using a ceramic chip antenna (not mounted), or to connect
 * and external antenna over a pigtail to the U.Fl connector (not mounted).
 *
 * RF switch state:
 * - LOW: 2.4GHz RF interface on RP-SMA connector, CC1200 powered-off.
 * - HIGH: Sub-1GHz RF interface on RP-SMA connector.
 * @{
 */
#define ANTENNA_RF_SW_PORT GPIO_D_NUM
#define ANTENNA_RF_SW_PIN  2
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Dual RF interface support
 *
 * Enables support for dual band operation (both CC1200 and 2.4GHz enabled).
 * The driver checks the selected Radio stack, and forces the antenna switch to
 * either position.  Enabling the definition below forces to skip this check.
 * @{
 */
#define REMOTE_DUAL_RF_ENABLED 0
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
#define CC1200_GPIOx_VECTOR         NVIC_INT_GPIO_PORT_B
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
#define USD_CSN_PORT             GPIO_A_NUM
#define USD_CSN_PIN              7
#define USD_SEL_PORT             GPIO_A_NUM
#define USD_SEL_PIN              6
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Power management and shutdown mode
 *
 * The shutdown mode is an ultra-low power operation mode that effectively
 * powers-down the entire RE-Mote (CC2538, CC1200, attached sensors, etc) and
 * only keeps running a power gating timer (NanoTimer), the on-board RTC and
 * an ultra-low power consumption MCU (PIC12F635).  The Shutdown mode allows:
 *
 * - Put the RE-Mote in an ultra-low power sleep (shutdown) drawing <200nA avg.
 * - Periodically awake and execute tasks, being the shutdown period selectable
 *   via R47 resistor value (22KOhm as default for 1 minute shutdown period).
 * - Enter shutdown mode before the shutdown period expiration, by invoking the
 *   PM_SHUTDOWN_NOW macrp
 *
 * The shutdown mode can be disabled by hardware by short-circuiting or placing
 * an 0Ohm resistor across W1 pad.
 * @{
 */
#define PM_DONE_PORT                GPIO_D_NUM
#define PM_DONE_PIN                 0
#define PM_CMD_PORT                 GPIO_D_NUM
#define PM_CMD_PIN                  1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name On-board RTC
 *
 * The shutdown mode can be disabled by hardware by short-circuiting or placing
 * an 0Ohm resistor across W1 pad.  As the RTC_INT1 pin is also shared with the
 * BUTTON_USER, so either disable or not use the user button, or upon receiving
 * an interrupt, poll the RTC.
 *
 * The RTC_INT1 can be used to exit the CC2538's LPM3 mode.
 * A second interruption pin is connected to the PIC12F635, for applications
 * requiring to put the PIC into deep-sleep and waking up at a certain time.
 * @{
 */
#define RTC_SDA_PORT                I2C_SDA_PORT
#define RTC_SDA_PIN                 I2C_SDA_PIN
#define RTC_SCL_PORT                I2C_SCL_PORT
#define RTC_SCL_PIN                 I2C_SCL_PIN
#define RTC_INT1_PORT               GPIO_A_NUM
#define RTC_INT1_PIN                4
#define RTC_INT1_VECTOR             NVIC_INT_GPIO_PORT_A
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name On-board external WDT
 * The RE-Mote features an on-board external WDT and battery monitor, which
 * adds more robustness and prevents the mote to run wild if any unexpected
 * problem shows-up.
 * The external WDT requires a short pulse (<1ms) to be sent before a 2-second
 * period.  The battery monitor keeps the device in Reset if the voltage input
 * is lower than 2.5V.
 * The external WDT can be disabled by removing the R40 0Ohm resistor.
 * The EXT_WDT pin is shared with LED1 (Red).  For long-time operation, it is
 * advised to remove R14 resistor to disable LED1.
 * As default the Texas Instrument's TPS3823 WDT is not mounted.
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
#define BOARD_STRING "Zolertia RE-Mote revision A platform"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
