/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-button-sensor
 * @{
 *
 * \file
 *  Driver for the Zoul user button
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/nvic.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/button-sensor.h"
#include "sys/timer.h"
#include "sys/ctimer.h"
#include "sys/process.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define BUTTON_USER_PORT_BASE  GPIO_PORT_TO_BASE(BUTTON_USER_PORT)
#define BUTTON_USER_PIN_MASK   GPIO_PIN_MASK(BUTTON_USER_PIN)
/*---------------------------------------------------------------------------*/
#define DEBOUNCE_DURATION (CLOCK_SECOND >> 4)

static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
static clock_time_t press_duration = 0;
static struct ctimer press_counter;
static uint8_t press_event_counter;

process_event_t button_press_duration_exceeded;
/*---------------------------------------------------------------------------*/
static void
duration_exceeded_callback(void *data)
{
  press_event_counter++;
  process_post(PROCESS_BROADCAST, button_press_duration_exceeded,
               &press_event_counter);
  ctimer_set(&press_counter, press_duration, duration_exceeded_callback,
             NULL);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Retrieves the value of the button pin
 * \param type Returns the pin level or the counter of press duration events.
 *             type == BUTTON_SENSOR_VALUE_TYPE_LEVEL or
 *             type == BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION
 *             respectively
 */
static int
value(int type)
{
  switch(type) {
  case BUTTON_SENSOR_VALUE_TYPE_LEVEL:
    return GPIO_READ_PIN(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);
  case BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION:
    return press_event_counter;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Callback registered with the GPIO module. Gets fired with a button
 * port/pin generates an interrupt
 * \param port The port number that generated the interrupt
 * \param pin The pin number that generated the interrupt. This is the pin
 * absolute number (i.e. 0, 1, ..., 7), not a mask
 */
static void
btn_callback(uint8_t port, uint8_t pin)
{
  if(!timer_expired(&debouncetimer)) {
    return;
  }

  timer_set(&debouncetimer, DEBOUNCE_DURATION);

  if(press_duration) {
    press_event_counter = 0;
    if(value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) == BUTTON_SENSOR_PRESSED_LEVEL) {
      ctimer_set(&press_counter, press_duration, duration_exceeded_callback,
                 NULL);
    } else {
      ctimer_stop(&press_counter);
    }
  }

  sensors_changed(&button_sensor);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Init function for the User button.
 * \param type SENSORS_ACTIVE: Activate / Deactivate the sensor (value == 1
 *             or 0 respectively)
 *
 * \param value Depends on the value of the type argument
 * \return Depends on the value of the type argument
 */
static int
config_user(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    button_press_duration_exceeded = process_alloc_event();

    /* Software controlled */
    GPIO_SOFTWARE_CONTROL(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);

    /* Set pin to input */
    GPIO_SET_INPUT(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);

    /* Enable edge detection */
    GPIO_DETECT_EDGE(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);

    /* Both Edges */
    GPIO_TRIGGER_BOTH_EDGES(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);

    ioc_set_over(BUTTON_USER_PORT, BUTTON_USER_PIN, IOC_OVERRIDE_PUE);

    gpio_register_callback(btn_callback, BUTTON_USER_PORT, BUTTON_USER_PIN);
    break;
  case SENSORS_ACTIVE:
    if(value) {
      GPIO_ENABLE_INTERRUPT(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);
      NVIC_EnableIRQ(BUTTON_USER_VECTOR);
    } else {
      GPIO_DISABLE_INTERRUPT(BUTTON_USER_PORT_BASE, BUTTON_USER_PIN_MASK);
      NVIC_DisableIRQ(BUTTON_USER_VECTOR);
    }
    return value;
  case BUTTON_SENSOR_CONFIG_TYPE_INTERVAL:
    press_duration = (clock_time_t)value;
    break;
  default:
    break;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, config_user, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
