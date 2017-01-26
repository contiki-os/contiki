/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup saker
 *
 * @{
 *
 * \defgroup saker-board Board Definitions
 *
 * This file provides connectivity information on LEDs, Buttons, UARTs, Buses
 * etc.
 *
 * @{
 *
 * \file
 *      Header file with definitions related to the I/O connections
 *
 * \note Do not include this file directly. It gets included by contiki-conf
 *       after all relevant directives have been set.
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"

/*---------------------------------------------------------------------------*/
/** \name LED configuration
 *
 * LEDs on are connected as follows:
 * - LED1 (Red)    -> PC1
 * - LED2 (Yellow) -> PC2
 * - LED3 (Green)  -> PC3
 * @{
 */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_RED
#undef LEDS_CONF_ALL

/* PC1 */
#define LEDS_RED                        2
/* PC2 */
#define LEDS_YELLOW                     4
/* PC3 */
#define LEDS_GREEN                      8

#define LEDS_CONF_ALL                   (LEDS_RED + LEDS_YELLOW + LEDS_GREEN)
/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS               1
/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup is driven by PC0
 */
#define USB_PULLUP_PORT                 GPIO_C_NUM
#define USB_PULLUP_PIN                  0
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UART configuration
 *
 * UART0 is connected to a FTD234XD USB driver chip via the RX and TX pins PA0
 * and PA1. The other pins of the chip's serial interface are connected to PCB
 * routes that lead to the CC2538's pins given below, although those routes each
 * feature an air gap where another resistor would have to be fitted to close
 * them. These pins are listed here just for documentation purposes.
 *
 * @{
 */
/* PA0 */
#define UART0_RX_PORT                   GPIO_A_NUM
#define UART0_RX_PIN                    0
/* PA1 */
#define UART0_TX_PORT                   GPIO_A_NUM
#define UART0_TX_PIN                    1
/* PA2 */
#define FTDI_RTS_N_PORT                 GPIO_A_NUM
#define FTDI_RTS_N_PIN                  2
/* PA3 */
#define FTDI_CTS_N_PORT                 GPIO_A_NUM
#define FTDI_CTS_N_PIN                  3
/* PA4 */
#define FTDI_CBUS0_PORT                 GPIO_A_NUM
#define FTDI_CBUS0_PIN                  4

/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name Button configuration
 *
 * The platform offers two buttons, one of which is hardwired to the CPUs Reset
 * pin and therefore not available for programming.
 *
 * @{
 */
/* PA5 */
#define BUTTON_BTN1_PORT                GPIO_A_NUM
#define BUTTON_BTN1_PIN                 5
#define BUTTON_BTN1_VECTOR              GPIO_A_IRQn
/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON             1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name CC1200 configuration
 *
 * These macros configure the interface to the CC1200 over SPI module 0.
 * @{
 */
/* Which SSI instance to use? Valid values are 0 and 1 */
#define CC1200_SPI_INSTANCE             0
/* PB0 */
#define CC1200_SCLK_PORT                GPIO_B_NUM
#define CC1200_SCLK_PIN                 0
/* PC7 */
#define CC1200_MOSI_PORT                GPIO_C_NUM
#define CC1200_MOSI_PIN                 7
/* PB2 */
#define CC1200_MISO_PORT                GPIO_B_NUM
#define CC1200_MISO_PIN                 2
/* PB1 */
#define CC1200_CS_N_PORT                GPIO_B_NUM
#define CC1200_CS_N_PIN                 1
/* PC4 */
#define CC1200_RESET_N_PORT             GPIO_C_NUM
#define CC1200_RESET_N_PIN              4
/* PB3 */
#define CC1200_GPIO0_PORT               GPIO_B_NUM
#define CC1200_GPIO0_PIN                3
/* PC6 */
#define CC1200_GPIO2_PORT               GPIO_C_NUM
#define CC1200_GPIO2_PIN                6
/* PC5 */
#define CC1200_GPIO3_PORT               GPIO_C_NUM
#define CC1200_GPIO3_PIN                5
/* No changes needed below.
   See ssi.h / spi-arch.c for the corresponding macros */
/* New SPI API */
#define CC1200_SPI_TXBUF                SPIX_BUF(CC1200_SPI_INSTANCE)
#define CC1200_SPI_RXBUF                SPIX_BUF(CC1200_SPI_INSTANCE)
#define CC1200_SPI_WAITFORTxREADY()     SPIX_WAITFORTxREADY(CC1200_SPI_INSTANCE)
#define CC1200_SPI_WAITFOREORx()        SPIX_WAITFOREORx(CC1200_SPI_INSTANCE)
#define CC1200_SPI_FLUSH                SPIX_FLUSH(CC1200_SPI_INSTANCE)
#if (CC1200_SPI_INSTANCE == 0)
/* The following macros are needed by spi.c */
#define SPI0_CLK_PORT                   CC1200_SCLK_PORT
#define SPI0_CLK_PIN                    CC1200_SCLK_PIN
#define SPI0_TX_PORT                    CC1200_MOSI_PORT
#define SPI0_TX_PIN                     CC1200_MOSI_PIN
#define SPI0_RX_PORT                    CC1200_MISO_PORT
#define SPI0_RX_PIN                     CC1200_MISO_PIN
#if (SYS_CTRL_CONF_SYS_DIV == SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ)
/* Max SPI speed is 10 MHz */
#define SPI0_CONF_CPRS_CPSDVSR          4
#else
#define SPI0_CONF_CPRS_CPSDVSR          2
#endif
#elif (CC1200_SPI_INSTANCE == 1)
/* The following macros are needed by spi.c */
#define SPI1_CLK_PORT                   CC1200_SCLK_PORT
#define SPI1_CLK_PIN                    CC1200_SCLK_PIN
#define SPI1_TX_PORT                    CC1200_MOSI_PORT
#define SPI1_TX_PIN                     CC1200_MOSI_PIN
#define SPI1_RX_PORT                    CC1200_MISO_PORT
#define SPI1_RX_PIN                     CC1200_MISO_PIN
#if (SYS_CTRL_CONF_SYS_DIV == SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ)
/* Max SPI speed is 10 MHz */
#define SPI1_CONF_CPRS_CPSDVSR          4
#else
#define SPI1_CONF_CPRS_CPSDVSR          2
#endif
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ENC28J60 configuration
 *
 * These macros configure the interface to the ENC28J60 over SPI module 1.
 *
 * @{
 */
/* Which SSI module to use? Valid values are 0 and 1 */
#define ENC28J60_SPI_INSTANCE           1
/* PD1 */
#define ENC28J60_SCLK_PORT              GPIO_D_NUM
#define ENC28J60_SCLK_PIN               1
/* PD3 */
#define ENC28J60_MOSI_PORT              GPIO_D_NUM
#define ENC28J60_MOSI_PIN               3
/* PD2 */
#define ENC28J60_MISO_PORT              GPIO_D_NUM
#define ENC28J60_MISO_PIN               2
/* PD0 */
#define ENC28J60_CS_N_PORT              GPIO_D_NUM
#define ENC28J60_CS_N_PIN               0
/* PD4 */
#define ENC28J60_INT_N_PORT             GPIO_D_NUM
#define ENC28J60_INT_N_PIN              4
/* PD5 */
#define ENC28J60_RESET_N_PORT           GPIO_D_NUM
#define ENC28J60_RESET_N_PIN            5
/* No changes needed below.
   See ssi.h / spi-arch.c for the corresponding macros */
#define ENC28J60_SPI_TXBUF              SPIX_BUF(ENC28J60_SPI_INSTANCE)
#define ENC28J60_SPI_RXBUF              SPIX_BUF(ENC28J60_SPI_INSTANCE)
#define ENC28J60_SPI_WAITFORTxREADY()   SPIX_WAITFORTxREADY(ENC28J60_SPI_INSTANCE)
#define ENC28J60_SPI_WAITFOREORx()      SPIX_WAITFOREORx(ENC28J60_SPI_INSTANCE)
#define ENC28J60_SPI_FLUSH              SPIX_FLUSH(ENC28J60_SPI_INSTANCE)
#if (ENC28J60_SPI_INSTANCE == 0)
/* The following macros are needed by spi.c */
#define SPI0_CLK_PORT                   ENC28J60_SCLK_PORT
#define SPI0_CLK_PIN                    ENC28J60_SCLK_PIN
#define SPI0_TX_PORT                    ENC28J60_MOSI_PORT
#define SPI0_TX_PIN                     ENC28J60_MOSI_PIN
#define SPI0_RX_PORT                    ENC28J60_MISO_PORT
#define SPI0_RX_PIN                     ENC28J60_MISO_PIN
#define SPI0_CPRS_CPSDVSR               2
#elif (ENC28J60_SPI_INSTANCE == 1)
/* The following macros are needed by spi.c */
#define SPI1_CLK_PORT                   ENC28J60_SCLK_PORT
#define SPI1_CLK_PIN                    ENC28J60_SCLK_PIN
#define SPI1_TX_PORT                    ENC28J60_MOSI_PORT
#define SPI1_TX_PIN                     ENC28J60_MOSI_PIN
#define SPI1_RX_PORT                    ENC28J60_MISO_PORT
#define SPI1_RX_PIN                     ENC28J60_MISO_PIN
#define SPI1_CPRS_CPSDVSR               2
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name EEPROM I2C configuration
 */
/* PB4 */
#define I2C_SCL_PORT                    GPIO_B_NUM
#define I2C_SCL_PIN                     4
/* PB5 */
#define I2C_SDA_PORT                    GPIO_B_NUM
#define I2C_SDA_PIN                     5
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING                    "saker"
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
