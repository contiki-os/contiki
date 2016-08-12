/*
 * Copyright (c) 2015, Weptech elektronik GmbH Germany
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

#ifndef CC1200_ARCH_H
#define CC1200_ARCH_H

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/*
 * Initialize SPI module & Pins.
 *
 * The function has to accomplish the following tasks:
 * - Enable SPI and configure SPI (CPOL = 0, CPHA = 0)
 * - Configure MISO, MOSI, SCLK accordingly
 * - Configure GPIOx (input)
 * - Configure RESET_N (output high)
 * - Configure CSn (output high)
 */
void
cc1200_arch_init(void);
/*---------------------------------------------------------------------------*/
/* Select CC1200 (pull down CSn pin). */
void
cc1200_arch_spi_select(void);
/*---------------------------------------------------------------------------*/
/* De-select CC1200 (release CSn pin). */
void
cc1200_arch_spi_deselect(void);
/*---------------------------------------------------------------------------*/
/*
 * Configure port IRQ for GPIO0.
 * If rising == 1: configure IRQ for rising edge, else falling edge
 * Interrupt has to call cc1200_rx_interrupt()!
 */
void
cc1200_arch_gpio0_setup_irq(int rising);
/*---------------------------------------------------------------------------*/
/*
 * Configure port IRQ for GPIO2.
 *
 * GPIO2 might not be needed at all depending on the driver's
 * configuration (see cc1200-conf.h)
 *
 * If rising == 1: configure IRQ for rising edge, else falling edge
 * Interrupt has to call cc1200_rx_interrupt()!
 */
void
cc1200_arch_gpio2_setup_irq(int rising);
/*---------------------------------------------------------------------------*/
/* Reset interrupt flag and enable GPIO0 port IRQ. */
void
cc1200_arch_gpio0_enable_irq(void);
/*---------------------------------------------------------------------------*/
/* Disable GPIO0 port IRQ. */
void
cc1200_arch_gpio0_disable_irq(void);
/*---------------------------------------------------------------------------*/
/*
 * Reset interrupt flag and enable GPIO2 port IRQ
 *
 * GPIO2 might not be needed at all depending on the driver's
 * configuration (see cc1200-conf.h)
 */
void
cc1200_arch_gpio2_enable_irq(void);
/*---------------------------------------------------------------------------*/
/*
 * Disable GPIO2 port IRQ.
 *
 * GPIO2 might not be needed at all depending on the driver's
 * configuration (see cc1200-conf.h)
 */
void
cc1200_arch_gpio2_disable_irq(void);
/*---------------------------------------------------------------------------*/
/*
 * Read back the status of the GPIO0 pin.
 * Returns 0 if the pin is low, otherwise 1
 */
int
cc1200_arch_gpio0_read_pin(void);
/*---------------------------------------------------------------------------*/
/*
 * Read back the status of the GPIO2 pin.
 *
 * GPIO2 might not be needed at all depending on the driver's
 * configuration (see cc1200-conf.h)
 *
 * Returns 0 if the pin is low, otherwise 1
 */
int
cc1200_arch_gpio2_read_pin(void);
/*---------------------------------------------------------------------------*/
/*
 * Read back the status of the GPIO3 pin.
 *
 * Currently only used for rf test modes.
 *
 * Returns 0 if the pin is low, otherwise 1
 */
int
cc1200_arch_gpio3_read_pin(void);
/*---------------------------------------------------------------------------*/
/* Write a single byte via SPI, return response. */
int
cc1200_arch_spi_rw_byte(uint8_t c);
/*---------------------------------------------------------------------------*/
/* 
 * Write a sequence of bytes while reading back the response.
 * Either read_buf or write_buf can be NULL.
 */
int 
cc1200_arch_spi_rw(uint8_t *read_buf,
                   const uint8_t *write_buf,
                   uint16_t len);
/*---------------------------------------------------------------------------*/
/*
 * The CC1200 interrupt handler exported from the cc1200 driver.
 *
 * To be called by the hardware interrupt handler(s),
 * which are defined as part of the cc1200-arch interface.
 */
int 
cc1200_rx_interrupt(void);

#endif /* CC1200_ARCH_H */
