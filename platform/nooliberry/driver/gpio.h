/*
 * Copyright (c) 2013, NooliTIC
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file gpio.h
 *  Provides the functionality for GPIOs
 *
 * \author
 *  Ludovic WIART <ludovic.wiart@noolitic.biz>
 *
 *  History:
 *     19/03/2012 L. Wiart - Created
 *     08/02/2013 S. Dawans - Code Style & Integration in Contiki fork
 */

#ifndef __GPIO_H__
#define __GPIO_H__

/* GPIO port */

#define GPIO_0_DDR DDRB
#define GPIO_0_PORT PORTB
#define GPIO_0_PIN PINB
#define GPIO_0_BIT 5
#define GPIO_0 0
#define GPIO_1_DDR DDRB
#define GPIO_1_PORT PORTB
#define GPIO_1_PIN PINB
#define GPIO_1_BIT 6
#define GPIO_1 1
#define GPIO_2_DDR DDRB
#define GPIO_2_PORT PORTB
#define GPIO_2_PIN PINB
#define GPIO_2_BIT 7
#define GPIO_2 2

#define GPIO_3_DDR DDRG
#define GPIO_3_PORT PORTG
#define GPIO_3_PIN PING
#define GPIO_3_BIT 0
#define GPIO_3 3
#define GPIO_4_DDR DDRG
#define GPIO_4_PORT PORTG
#define GPIO_4_PIN PING
#define GPIO_4_BIT 1
#define GPIO_4 4
#define GPIO_5_DDR DDRG
#define GPIO_5_PORT PORTG
#define GPIO_5_PIN PING
#define GPIO_5_BIT 2
#define GPIO_5 5

#define GPIO_6_DDR DDRD
#define GPIO_6_PORT PORTD
#define GPIO_6_PIN PIND
#define GPIO_6_BIT 6
#define GPIO_6 6
#define GPIO_7_DDR DDRD
#define GPIO_7_PORT PORTD
#define GPIO_7_PIN PIND
#define GPIO_7_BIT 7
#define GPIO_7 7

#define GPIO_8_DDR DDRE
#define GPIO_8_PORT PORTE
#define GPIO_8_PIN PINE
#define GPIO_8_BIT 3
#define GPIO_8 8

#define GPIO_1WR_DDR DDRG
#define GPIO_1WR_PORT PORTG
#define GPIO_1WR_PIN PING
#define GPIO_1WR_BIT 5
#define GPIO_1WR 9

#define  sbi(port, bit)   (port) |= (1 << (bit))
#define  cbi(port, bit)   (port) &= ~(1 << (bit))

void ngpio_init(void);
void gpio_set_on(int gpio);
void gpio_set_off(int gpio);
void gpio_set_output(int gpio);
void gpio_set_input(int gpio);
int gpio_get(int gpio);
void gpio_toggle(int gpio);

#endif /* __GPIO_H__ */
