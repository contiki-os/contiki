/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``as-is'' AND
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf52dk-devices Device drivers
 * @{
 *
 * \addtogroup nrf52dk-devices-button Buttons driver
 * @{
 *
 * \file
 *         Driver for nRF52 DK buttons.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_drv_gpiote.h"
#include "nrf_assert.h"
#include "boards.h"
#include "contiki.h"
#include "lib/sensors.h"
#include "button-sensor.h"

/*---------------------------------------------------------------------------*/
#define DEBOUNCE_DURATION (CLOCK_SECOND >> 5) /**< Delay before button state is assumed to be stable */

/*---------------------------------------------------------------------------*/
struct btn_timer
{
  struct timer debounce;
  clock_time_t start;
  clock_time_t duration;
};

static struct btn_timer btn_timer[BUTTONS_NUMBER];
static int btn_state = 0;

/*---------------------------------------------------------------------------*/
/**
 * \brief Button toggle handler
 * \param pin GPIO pin which has been triggered
 * \param action toggle direction
 *
 */
static void
gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  int id = pin - BUTTON_START;

  if(!timer_expired(&(btn_timer[id].debounce))) {
    return;
  }

  /* Set timer to ignore consecutive changes for
   * DEBOUNCE_DURATION.
   */
  timer_set(&(btn_timer[id].debounce), DEBOUNCE_DURATION);

  /*
   * Start measuring duration on falling edge, stop on rising edge.
   */
  if(nrf_drv_gpiote_in_is_set(pin) == 0) {
    btn_timer[id].start = clock_time();
    btn_timer[id].duration = 0;
  } else {
    btn_timer[id].duration = clock_time() - btn_timer[id].start;
  }
  sensors_changed(&buttons[id]);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the button sensor for all buttons.
 *
 * \param type if \a SENSORS_HW_INIT is passed the function will initialize
 *             given button
 *             if \a SENSORS_ACTIVE is passed then \p c parameter defines
 *             whether button should be set active or inactive
 * \param c    0 to disable the button, non-zero: enable
 * \param pin  GPIOE pin number
 */
static int
config(int type, int c, nrf_drv_gpiote_pin_t pin)
{
  int id = pin - BUTTON_START;

  switch(type) {
    case SENSORS_HW_INIT: {
      nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
      config.pull = NRF_GPIO_PIN_PULLUP;
      nrf_drv_gpiote_in_init(pin, &config, gpiote_event_handler);
      timer_set(&(btn_timer[id].debounce), DEBOUNCE_DURATION);
      return 1;
    }
    case SENSORS_ACTIVE: {
      if(c) {
        nrf_drv_gpiote_in_event_enable(pin, true);
        btn_state |= (1 << id);
      } else {
        nrf_drv_gpiote_in_event_disable(pin);
        btn_state &= ~(1 << id);
      }
      return 1;
    }
    default:
      return 0;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for button 1
 *
 * \param type passed to config() as-is
 * \param value passed to config() as-is
 * \return same as config() return value
 */
static int
config_button_1(int type, int value)
{
  return config(type, value, BSP_BUTTON_0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for button 2
 *
 * \param type passed to config() as-is
 * \param value passed to config() as-is
 * \return same as config() return value
 */
static int
config_button_2(int type, int value)
{
  return config(type, value, BSP_BUTTON_1);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for button 3
 *
 * \param type passed to config() as-is
 * \param value passed to config() as-is
 * \return same as config() return value
 */
static int
config_button_3(int type, int value)
{
  return config(type, value, BSP_BUTTON_2);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for button 4
 *
 * \param type passed to config() as-is
 * \param value passed to config() as-is
 * \return same as config() return value
 */
static int
config_button_4(int type, int value)
{
  return config(type, value, BSP_BUTTON_3);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Return current state of a button
 * \param type pass \ref BUTTON_SENSOR_VALUE_STATE to get current button state
 *             or \ref BUTTON_SENSOR_VALUE_DURATION to get active state duration
 * \param pin GPIOE pin number
 *
 * \retval BUTTON_SENSOR_VALUE_PRESSED
 * \retval BUTTON_SENSOR_VALUE_RELEASED when \a type is \ref BUTTON_SENSOR_VALUE_STATE
 * \retval duration Active state duration in clock ticks
 */
static int
value(int type, nrf_drv_gpiote_pin_t pin)
{

  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return nrf_drv_gpiote_in_is_set(pin) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return btn_timer[pin - BUTTON_START].duration;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Return current state of a button 1
 * \param type passed to value() as-is
 * \return same as value returned by value()
 */
static int
value_button_1(int type)
{
  return value(type, BSP_BUTTON_0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Return current state of a button 2
 * \param type passed to value() as-is
 * \return same as value returned by value()
 */
static int
value_button_2(int type)
{
  return value(type, BSP_BUTTON_1);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Return current state of a button 3
 * \param type passed to value() as-is
 * \return same as value returned by value()
 */
static int
value_button_3(int type)
{
  return value(type, BSP_BUTTON_2);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Return current state of a button 4
 * \param type passed to value() as-is
 * \return same as value returned by value()
 */
static int
value_button_4(int type)
{
  return value(type, BSP_BUTTON_3);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get status of a given button
 * \param type \a SENSORS_ACTIVE or \a SENSORS_READY
 * \param pin GPIOE pin number
 * \return 1 if the button's port interrupt is enabled
 */
static int
status(int type, nrf_drv_gpiote_pin_t pin)
{
  switch(type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return (btn_state & (1 << (pin - BUTTON_START)));
    default:
      break;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for button 1
 * \param type passed to state() as-is
 * \return value returned by state()
 */
static int
status_button_1(int type)
{
  return status(type, BSP_BUTTON_0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for button 2
 * \param type passed to state() as-is
 * \return value returned by state()
 */
static int
status_button_2(int type)
{
  return status(type, BSP_BUTTON_1);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for button 3
 * \param type passed to state() as-is
 * \return value returned by state()
 */
static int
status_button_3(int type)
{
  return status(type, BSP_BUTTON_2);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for button 3
 * \param type passed to state() as-is
 * \return value returned by state()
 */
static int
status_button_4(int type)
{
  return status(type, BSP_BUTTON_3);
}
/*---------------------------------------------------------------------------*/
const struct sensors_sensor buttons[BUTTONS_NUMBER] = {
    {BUTTON_SENSOR, value_button_1, config_button_1, status_button_1},
    {BUTTON_SENSOR, value_button_2, config_button_2, status_button_2},
    {BUTTON_SENSOR, value_button_3, config_button_3, status_button_3},
    {BUTTON_SENSOR, value_button_4, config_button_4, status_button_4}, };
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
