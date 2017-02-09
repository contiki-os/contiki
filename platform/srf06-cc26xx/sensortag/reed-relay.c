/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-reed-relay
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26XX Reed Relay
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/clock.h"
#include "sys/timer.h"
#include "lib/sensors.h"
#include "sensortag/reed-relay.h"
#include "gpio-interrupt.h"
#include "sys/timer.h"

#include "ti-lib.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
#define REED_IO_CFG             (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO |       \
                                 IOC_IOPULL_DOWN  | IOC_SLEW_DISABLE  |       \
                                 IOC_HYST_DISABLE | IOC_BOTH_EDGES    |       \
                                 IOC_INT_DISABLE  | IOC_IOMODE_NORMAL |       \
                                 IOC_NO_WAKE_UP   | IOC_INPUT_ENABLE)
/*---------------------------------------------------------------------------*/
/**
 * \brief Handler for Sensortag-CC26XX reed interrupts
 */
static void
reed_interrupt_handler(uint8_t ioid)
{
  if(!timer_expired(&debouncetimer)) {
    ENERGEST_OFF(ENERGEST_TYPE_IRQ);
    return;
  }

  sensors_changed(&reed_relay_sensor);
  timer_set(&debouncetimer, CLOCK_SECOND / 2);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return (int)ti_lib_gpio_read_dio(BOARD_IOID_REED_RELAY);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the button sensor for all buttons.
 *
 * \param type SENSORS_HW_INIT: Initialise. SENSORS_ACTIVE: Enables/Disables
 *        depending on 'value'
 * \param value 0: disable, non-zero: enable
 * \return Always returns 1
 */
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    ti_lib_ioc_int_disable(BOARD_IOID_REED_RELAY);
    ti_lib_gpio_clear_event_dio(BOARD_IOID_REED_RELAY);

    /* Enable the GPIO clock when the CM3 is running */
    ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

    /* S/W control, input, pull-down */
    ti_lib_ioc_port_configure_set(BOARD_IOID_REED_RELAY, IOC_PORT_GPIO,
                                  REED_IO_CFG);

    gpio_interrupt_register_handler(BOARD_IOID_REED_RELAY,
                                    reed_interrupt_handler);
    break;
  case SENSORS_ACTIVE:
    if(value) {
      ti_lib_ioc_int_enable(BOARD_IOID_REED_RELAY);
    } else {
      ti_lib_ioc_int_disable(BOARD_IOID_REED_RELAY);
    }
    break;
  default:
    break;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the reed
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 Interrupt enabled, 0: Disabled
 */
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return (ti_lib_ioc_port_configure_get(BOARD_IOID_REED_RELAY)
            & IOC_INT_ENABLE) == IOC_INT_ENABLE;
    break;
  default:
    break;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(reed_relay_sensor, "REED", value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
