/**
 * \addtogroup mb851-platform
 *
 * @{
 */

/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#include "dev/button-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"
#include "dev/leds.h"

/*---------------------------------------------------------------------------*/
static uint8_t sensors_status;

#define BUTTON_STATUS_ACTIVE 	(1 << 0)
#define TEMP_STATUS_ACTIVE 		(1 << 1)
#define ACC_STATUS_ACTIVE 		(1 << 2)
/*---------------------------------------------------------------------------*/
/**
 * \brief      Init board (currently does nothing)
 */
void
board_init(void)
{
  return;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Power down board
 */
void
board_power_down(void)
{
  /* Set everything except LEDs to input */
  GPIO_PACFGL = (GPIOCFG_IN << PA0_CFG_BIT) |
    (GPIOCFG_IN << PA1_CFG_BIT) |
    (GPIOCFG_IN << PA2_CFG_BIT) | (GPIOCFG_IN << PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN << PA4_CFG_BIT) |                       /* PTI EN */
    (GPIOCFG_IN << PA5_CFG_BIT) |                                 /* PTI_DATA */
    (GPIOCFG_IN << PA6_CFG_BIT) | (GPIOCFG_IN << PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN << PB0_CFG_BIT) | (GPIOCFG_IN << PB1_CFG_BIT) |
    (GPIOCFG_IN << PB2_CFG_BIT) |                              /* Uart Tx, RX */
    (GPIOCFG_IN << PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN << PB4_CFG_BIT) |
    (GPIOCFG_IN << PB5_CFG_BIT) |
    (GPIOCFG_IN << PB6_CFG_BIT) | (GPIOCFG_IN << PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN << PC0_CFG_BIT) |
    (GPIOCFG_IN << PC1_CFG_BIT) |
    (GPIOCFG_IN << PC2_CFG_BIT) | (GPIOCFG_IN << PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN << PC4_CFG_BIT) | (GPIOCFG_IN << PC5_CFG_BIT) |
    (GPIOCFG_IN << PC6_CFG_BIT) | (GPIOCFG_IN << PC7_CFG_BIT);      /* OSC32K */

  leds_init();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Power down sensors
 */
void
board_sensors_power_down(void)
{
  /* 
   * Remember state of sensors (if active or not), in order to
   * resume their original state after calling powerUpSensors().
   * Useful when entering in sleep mode, since all system
   * peripherals have to be reinitialized.
   */
  sensors_status = 0;

  if(button_sensor.status(SENSORS_READY)) {
    sensors_status |= BUTTON_STATUS_ACTIVE;
  }
  if(temperature_sensor.status(SENSORS_READY)) {
    sensors_status |= TEMP_STATUS_ACTIVE;
  }
  if(acc_sensor.status(SENSORS_READY)) {
    sensors_status |= ACC_STATUS_ACTIVE;
    /* Power down accelerometer to save power */
    SENSORS_DEACTIVATE(acc_sensor);
  }
}

/*---------------------------------------------------------------------------*/
/**
 * \brief      Power up sensors
 */
void
board_sensors_power_up(void)
{
  button_sensor.configure(SENSORS_HW_INIT, 0);
  temperature_sensor.configure(SENSORS_HW_INIT, 0);
  acc_sensor.configure(SENSORS_HW_INIT, 0);

  if(sensors_status & BUTTON_STATUS_ACTIVE) {
    SENSORS_ACTIVATE(button_sensor);
  }
  if(sensors_status & TEMP_STATUS_ACTIVE) {
    SENSORS_ACTIVATE(temperature_sensor);
  }
  if(sensors_status & ACC_STATUS_ACTIVE) {
    SENSORS_ACTIVATE(acc_sensor);
  }
}

/*---------------------------------------------------------------------------*/
/**
 * \brief      Power up board
 */
void
board_power_up(void)
{
  /* Set everything to inputs */
  GPIO_PACFGL = (GPIOCFG_IN << PA0_CFG_BIT) |
    (GPIOCFG_IN << PA1_CFG_BIT) |
    (GPIOCFG_IN << PA2_CFG_BIT) | (GPIOCFG_IN << PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN << PA4_CFG_BIT) |                       /* PTI EN */
    (GPIOCFG_IN << PA5_CFG_BIT) |                                 /* PTI_DATA */
    (GPIOCFG_IN << PA6_CFG_BIT) | (GPIOCFG_IN << PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN << PB0_CFG_BIT) | (GPIOCFG_OUT_ALT << PB1_CFG_BIT) |
    (GPIOCFG_IN << PB2_CFG_BIT) |                              /* Uart Rx, Tx */
    (GPIOCFG_IN << PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN << PB4_CFG_BIT) |
    (GPIOCFG_IN << PB5_CFG_BIT) |
    (GPIOCFG_IN << PB6_CFG_BIT) | (GPIOCFG_IN << PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN << PC0_CFG_BIT) |
    (GPIOCFG_IN << PC1_CFG_BIT) |
    (GPIOCFG_IN << PC2_CFG_BIT) | (GPIOCFG_IN << PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN << PC4_CFG_BIT) | (GPIOCFG_IN << PC5_CFG_BIT) |
    (GPIOCFG_IN << PC6_CFG_BIT) | (GPIOCFG_IN << PC7_CFG_BIT);      /* OSC32K */
}
/*---------------------------------------------------------------------------*/
/* STM32W108 HAL functions needed to get use the
   hal/micro/cortexm3/micro.c functionality. */
void halBoardInit(void)      { board_init(); }
void halBoardPowerUp(void)   { board_power_up(); }
void halBoardPowerDown(void) { board_power_down(); }
/*---------------------------------------------------------------------------*/
/** @} */
