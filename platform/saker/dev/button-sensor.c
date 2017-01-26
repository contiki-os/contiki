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
 */

/**
 * \addtogroup saker-sensors
 *
 * @{
 *
 * \defgroup saker-button-sensor The platform's button driver
 *
 * Driver for the platform's button.
 *
 * For the time being, we only have one button to handle. The other one is a
 * hard wired reset button and not available for programming.
 *
 * @{
 *
 * \file
 *      Implementation of the platform's button driver
 */

#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/nvic.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "sys/timer.h"

/*---------------------------------------------------------------------------*/
static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
/**
 * \brief Common initialiser for all buttons
 *
 * \param port_base GPIO port's register offset
 * \param pin_mask Pin mask corresponding to the button's pin
 */
static void
configure_btn(uint32_t port_base, uint32_t pin_mask)
{

  /* Software controlled */
  GPIO_SOFTWARE_CONTROL(port_base, pin_mask);

  /* Set pin to input */
  GPIO_SET_INPUT(port_base, pin_mask);

  /* Enable edge detection */
  GPIO_DETECT_EDGE(port_base, pin_mask);

  /* Single edge */
  GPIO_TRIGGER_SINGLE_EDGE(port_base, pin_mask);

  /* Trigger interrupt on Falling edge */
  GPIO_DETECT_RISING(port_base, pin_mask);

  /* Enable interrupt */
  GPIO_ENABLE_INTERRUPT(port_base, pin_mask);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Callback registered with the GPIO module. Gets fired once a button
 * port/pin generates an interrupt
 *
 * \param port The port number that generated the interrupt
 * \param pin The pin number that generated the interrupt. This is the pin's
 * absolute number (i.e. 0, 1, ..., 7), not a mask
 */
static void
btn_callback(uint8_t port, uint8_t pin)
{

  if(!timer_expired(&debouncetimer)) {
    return;
  }

  timer_set(&debouncetimer, CLOCK_SECOND / 8);
  if((port == BUTTON_BTN1_PORT) && (pin == BUTTON_BTN1_PIN)) {
    sensors_changed(&button_sensor);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Init function for button1
 *
 * \param type ignored
 * \param value ignored
 * \return ignored
 */
static int
configure_btn1(int type, int value)
{

  timer_set(&debouncetimer, 0);

  configure_btn(GPIO_PORT_TO_BASE(BUTTON_BTN1_PORT),
                GPIO_PIN_MASK(BUTTON_BTN1_PIN));

  /* Disable pull up (we have one externally) */
  ioc_set_over(BUTTON_BTN1_PORT,
               BUTTON_BTN1_PIN,
               IOC_OVERRIDE_DIS);

  NVIC_EnableIRQ(BUTTON_BTN1_VECTOR);

  gpio_register_callback(btn_callback,
                         BUTTON_BTN1_PORT,
                         BUTTON_BTN1_PIN);

  return 1;
}
/*---------------------------------------------------------------------------*/
/** \brief Export a global symbol to be used by the sensor API */
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, NULL, configure_btn1, NULL);
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
