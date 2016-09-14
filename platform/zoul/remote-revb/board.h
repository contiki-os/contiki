/*
 * Copyright (c) 2016, Zolertia
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
 * \defgroup remote-revb RE-Mote platform revision B
 *
 * The RE-Mote was designed jointly with universities and industry partners in
 * RERUM European project, to ease the development of private and secure
 * applications for IoT and Smart City applications.  The RE-Mote packs several
 * on-board resources, like a RTC, external WDT, Micro-SD, RF switch and a
 * Shutdown mode to reduce its power consumption down to 150nA.
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
 * RE-Mote platform (revision B), cc2538-based
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
 * LED1.R/PD4            |-01|17-|   PB2/SPIO0.SCLK/CC1200.SCLK
 * LED2.G/JTAG.TDO/PB7   |-02|16-|   PB1/SPIO0.MOSI/CC1200.MOSI
 * LED3.B/JTAG.TDI/PB6   |-03|15-|   PB3/SPIO0.MISO/CC1200.MISO
 * UART0.RX/PA0          |-04|14-|   PA7/AIN7/USD.CS|ADC5
 * UART0.TX/PA1          |-05|13-|   DGND
 * PD0                   |-06|12-|   D+3.3
 * I2C.SDA/PC2           |-07|11-|   PA5/AIN5/ADC1
 * I2C.SCL/PC3           |-08|10-|   PA4/AIN4/ADC2
 * DGND                  |-09|09-|   DGND
 * D+3.3                 |-10|08-|   D+5.0
 * CC1200.GPIO0/PB4      |-11|07-|   PA2/AIN2/ADC3
 * CC1200.GPIO2/PB0      |-12|06-|   PA6/AIN6/USD.SEL|ADC4
 * UART1.RX/PC1          |-13|05-|   PC6/SPI1.MISO
 * UART1.TX/PC0          |-14|04-|   PC5/SPI1.MOSI
 * DGND                  |-15|03-|   PC4/SPI1.SCLK
 * D+3.3                 |-16|02-|   PS+EXT/VIN
 * CC1200.CS/PB5         |-17|01-|   DGND
 * ----------------------+---+---+---------------------------------------------
 *
 * Two auxiliary connectors allow to connect an external LiPo battery and
 * access to the RESET/user buttons:
 *
 * - JP4 (placed below JP6 connector): |1-| DGND, |2-| VBAT
 * - JP9 (placed above JP5 connector): |1-| BUTTON.RESET, |2-| BUTTON.USER|ADC6
 */
/*---------------------------------------------------------------------------*/
/** \name RE-Mote LED configuration
 *
 * LEDs on the RE-Mote are exposed in the JP6 port as follows:
 * - LED1 (Red)    -> PD4
 * - LED2 (Green)  -> PB7 (shared with JTAG.TDO)
 * - LED3 (Blue)   -> PB6 (shared with JTAG.TDI)
 *
 * The LEDs are connected to a MOSFET to minimize current draw.  The LEDs can
 * be disabled by removing resistors R12, R13 and R14.
 * @{
 */
/*---------------------------------------------------------------------------*/
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_BLUE
#undef LEDS_RED
#undef LEDS_CONF_ALL

#define LEDS_RED              1           /**< LED1 (Red)   -> PD4 */
#define LEDS_RED_PIN_MASK     (1 << 4)
#define LEDS_RED_PORT_BASE    GPIO_D_BASE

#define LEDS_GREEN            2           /**< LED2 (Green) -> PB7 */
#define LEDS_GREEN_PIN_MASK   (1 << 7)
#define LEDS_GREEN_PORT_BASE  GPIO_B_BASE

#define LEDS_BLUE             4           /**< LED3 (Blue)  -> PB6 */
#define LEDS_BLUE_PIN_MASK    (1 << 6)
#define LEDS_BLUE_PORT_BASE   GPIO_B_BASE

#define LEDS_CONF_ALL         (LEDS_GREEN | LEDS_BLUE | LEDS_RED) /* 7 */
#define LEDS_LIGHT_BLUE       (LEDS_GREEN | LEDS_BLUE)            /* 6 */
#define LEDS_YELLOW           (LEDS_GREEN | LEDS_RED)             /* 3 */
#define LEDS_PURPLE           (LEDS_BLUE  | LEDS_RED)             /* 5 */
#define LEDS_WHITE            LEDS_ALL                            /* 7 */

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
 *   - CTS: disabled as default, PD0 may be assigned if not using I2C interrupts
 *   - RTS: disabled as default
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
 * - ADC1 (PA5): up to 3.3V.
 * - ADC2 (PA4): up to 3.3V
 * - ADC3 (PA2): up to 5V, by means of a 2/3 voltage divider.
 *
 * Also there are other ADC channels shared by default with Micro SD card and
 * user button implementations:
 *
 * - ADC4 (PA6): up to 3.3V.
 * - ADC5 (PA7): up to 3.3V.
 * - ADC6 (PA3): up to 3.3V.
 *
 * ADC inputs can only be on port A.
 * All ADCx are exposed in JP5 connector, but only ADC1 and ADC3 have GND and
 * VDD (3/5V) pins next to it, so these can be exposed into a 3-pin phidget-like
 * connector, for ADC2 either solder a wire to connect, or use a 4-pin connector
 * to expose both ADC1 and ADC2 in a single connector, but this will leave no
 * space for a ADC3 connector.
 *
 * The internal ADC reference is 1190mV, use either a voltage divider as input,
 * or a different voltage reference, like AVDD5, or externally using PA7/AIN7
 * and PA6/AIN6 configurable as differential reference, by removing the R26 and
 * R33 0Ohm resistors to disconnect off the Micro-SD, and those will be
 * accessible from JP5 connector.
 *
 * To enable the ADC[2,4-6], remove any 0Ohm resistors if required (see above),
 * and define in your application `ADC_SENSORS_CONF_ADCx_PIN` and set its
 * value with the corresponding pin number (i.e ADC2 to 4 as mapped to PA4).
 * To disable any ADC[1-6] just define as above, but set to (-1) instead.

 * Warning: if using ADC6 (PA3), you will need to disable the bootloader by
 * making FLASH_CCA_CONF_BOOTLDR_BACKDOOR equal to zero
 * 
 * @{
 */
#define ADC_SENSORS_PORT         GPIO_A_NUM    /**< ADC GPIO control port */

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
#define ADC_SENSORS_ADC6_PIN     (-1)          /**< ADC6 not declared    */
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
 * - BUTTON_USER  -> PA3, S1 user button, shared with bootloader
 * - BUTTON_RESET -> RESET_N line, S2 reset the CC2538
 * - BUTTON_PWR   -> Depending on the enabled resistor, it can be used to reset
 *                   the onboard Low-power PIC, provoking a master reset on all
 *                   the RE-Mote's onboards components.  Note the BUTTON_RESET
 *                   only resets the CC2538.  This is disabled by default, as
 *                   the R45 0Ohm resistor is not soldered on that position
 *                   The other R45 position enables a test-button to drive the
 *                   SYSOFF pin of the power management block, disconnecting the
 *                   battery when used, leaving only powered the RTCC and
 *                   Low-Power PIC.  Useful if developing applications using the
 *                   shutdown mode if required to snap out of it.
 * @{
 */
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
 * over JP6 connector.
 * The I2C bus is shared with the on-board RTC and the Low-Power PIC
 * The I2C is exposed over the JP6 header, using a 5-pin connector with 2.54 mm
 * spacing, providing also D+3.3V, GND and PD0 pin that can be used as an
 * interrupt pin if required
 * @{
 */
#define I2C_SCL_PORT             GPIO_C_NUM
#define I2C_SCL_PIN              3
#define I2C_SDA_PORT             GPIO_C_NUM
#define I2C_SDA_PIN              2
#define I2C_INT_PORT             GPIO_D_NUM
#define I2C_INT_PIN              0
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
 * external module, to be used with SSI1.  USD_CSN and USD_SEL are shared with
 * ADC4/ADC5, but it is disabled by default as there are 0Ohm resistors
 * connecting the PA6/PA7 pins to the microSD (see ADC block above for comments)
 * The USD_SEL pin can be used both as output and input, to detect if there is
 * a microSD in the slot, or when connected to disable the microSD to save power
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
 * only keeps running a the on-board RTC and an ultra-low power consumption MCU
 * The Shutdown mode allows:
 *
 * - Put the board in an ultra-low power sleep (shutdown) drawing <150nA avg.
 * - Awake the system by scheduling the RTCC to awake the Low-Power PIC after
 *   it disconnects the battery and goes to sleep mode.
 * - Awake the system by using the Low-Power PIC's timer
 *
 * As commented above, S3 can be used to restart the entire board (power
 * management block included), or to kick the board out of shutdown mode by
 * reconnecting the battery.
 * @{
 */
#define PM_ENABLE_PORT                 GPIO_D_NUM
#define PM_ENABLE_PIN                  1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name On-board RTCC
 *
 * The on-board RTCC (real time clock-calendar) is powered over USB/battery,
 * and it will remain powered in shutdown mode with the Low-Power PIC.  The
 * RTC_INT1 is connected to the CC2538, so it is possible to receive interrupts
 * from a pre-configured alarm, even waking up the CC2538 from PM3.
 * A second interruption pin (RTC_INT2) is connected to the Low-Power PIC, after
 * configuring the RTCC the Low-Power PIC can drive the board to shutdown mode,
 * and enter into low-power mode (sleep), being the RTCC interrupt the waking up
 * source to resume operation.
 * 
 * @{
 */
#define RTC_SDA_PORT                I2C_SDA_PORT
#define RTC_SDA_PIN                 I2C_SDA_PIN
#define RTC_SCL_PORT                I2C_SCL_PORT
#define RTC_SCL_PIN                 I2C_SCL_PIN
#define RTC_INT1_PORT               GPIO_D_NUM
#define RTC_INT1_PIN                3
#define RTC_INT1_VECTOR             NVIC_INT_GPIO_PORT_D
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
 * The external WDT can be disabled by removing the R34 0Ohm resistor.
 * As default the Texas Instrument's TPS3823 WDT is not mounted.
 * Alternatively the testpoint or unused WDT's pad can be used to re-use as GPIO
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
#define BOARD_STRING "Zolertia RE-Mote revision B platform"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
