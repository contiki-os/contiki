/**
 * \file
 *
 * \brief Arduino Due/X LEDs support package.
 *
 * Copyright (c) 2011-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "board.h"
#include "led.h"
#include "gpio.h"

#define ACTIVE_LEVEL_IS_HIGH 1

typedef const struct
{
    uint32_t ul_port_id;      //!< LED GPIO port.
    uint32_t ul_active_level; //!< Active level of the LED.
} tLED_DESCRIPTOR;

//! Hardware descriptors of all LEDs.
static tLED_DESCRIPTOR LED_DESCRIPTOR[BOARD_NUM_OF_LED] =
{
#define INSERT_LED_DESCRIPTOR(LED_NO, unused)      \
  {                                                \
    LED##LED_NO##_GPIO, LED##LED_NO##_ACTIVE_LEVEL \
  },
  MREPEAT(BOARD_NUM_OF_LED, INSERT_LED_DESCRIPTOR, ~)
#undef INSERT_LED_DESCRIPTOR
};

/*! \brief Turns off the specified LEDs.
 *
 * \param led_gpio LED to turn off (LEDx_GPIO).
 *
 * \note The pins of the specified LEDs are set to GPIO output mode.
 */
void LED_Off(uint32_t led_gpio)
{
	uint32_t i;
	for (i = 0; i < BOARD_NUM_OF_LED; i++) {
		if (led_gpio == LED_DESCRIPTOR[i].ul_port_id) {
			if (LED_DESCRIPTOR[i].ul_active_level == ACTIVE_LEVEL_IS_HIGH) {
				gpio_set_pin_low(led_gpio);
			} else {
				gpio_set_pin_high(led_gpio);
			}
		}
	}
}

/*! \brief Turns on the specified LEDs.
 *
 * \param led_gpio LED to turn on (LEDx_GPIO).
 *
 * \note The pins of the specified LEDs are set to GPIO output mode.
 */
void LED_On(uint32_t led_gpio)
{
	uint32_t i;
	for (i = 0; i < BOARD_NUM_OF_LED; i++) {
		if (led_gpio == LED_DESCRIPTOR[i].ul_port_id) {
			if (LED_DESCRIPTOR[i].ul_active_level == ACTIVE_LEVEL_IS_HIGH) {
				gpio_set_pin_high(led_gpio);
			} else {
				gpio_set_pin_low(led_gpio);
			}
		}
	}
}

