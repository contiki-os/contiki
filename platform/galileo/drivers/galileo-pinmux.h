/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#ifndef CPU_X86_DRIVERS_GALILEO_PINMUX_H_
#define CPU_X86_DRIVERS_GALILEO_PINMUX_H_

#include <stdint.h>

typedef enum galileo_pin_group {
  GALILEO_PIN_GRP_ANALOG,
  GALILEO_PIN_GRP_DIGITAL
} galileo_pin_group_t;

#define GALILEO_NUM_ANALOG_PINS 6
#define GALILEO_NUM_DIGITAL_PINS 14
#define GALILEO_NUM_PINS (GALILEO_NUM_ANALOG_PINS + GALILEO_NUM_DIGITAL_PINS)

int galileo_pinmux_initialize(void);

/**
 * \brief     Set the indicated pin to be a digital input.
 * \param grp Indicates whether the pin is in the analog or digital group.
 * \param pin Index of pin within group.
 */
void galileo_pinmux_select_din(galileo_pin_group_t grp, unsigned pin);
/**
 * \brief Set the indicated pin to be a digital output.
 */
void galileo_pinmux_select_dout(galileo_pin_group_t grp, unsigned pin);
/**
 * \brief Set the indicated pin to be a PWM pin. Only a subset of the pins
 *        support PWM output. This implicitly operates on the digital pin
 *        group.
 */
void galileo_pinmux_select_pwm(unsigned pin);
/**
 * \brief Connect the indicated pin to a serial port. This implicitly operates
 *        on the digital pin group. Galileo Gen. 2 supports UART0 on pins 0 and
 *        1 and UART1 on pins 2 and 3.
 */
void galileo_pinmux_select_serial(unsigned pin);
/**
 * \brief Connect analog pins 4 (SDA) and 5 (SCL) to I2C.
 */
void galileo_pinmux_select_i2c(void);
/**
 * \brief Connect digital pins 11 (MOSI), 12 (MISO), and 13 (CLK) to SPI.
 */
void galileo_pinmux_select_spi(void);
/**
 * \brief Set the indicated pin to be an ADC input. This implicitly operates
 *        on the analog pin group.
 */
void galileo_pinmux_select_analog(unsigned pin);

#endif /* CPU_X86_DRIVERS_GALILEO_PINMUX_H_ */
