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
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \file
 *  Sensortag-specific board initialisation driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "lib/sensors.h"
#include "buzzer.h"
#include "lpm.h"
#include "ti-lib.h"
#include "board-peripherals.h"
#include "board-i2c.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define PRCM_DOMAINS (PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH)
/*---------------------------------------------------------------------------*/
static void
power_domains_on(void)
{
  /* Turn on relevant power domains */
  ti_lib_prcm_power_domain_on(PRCM_DOMAINS);

  /* Wait for domains to power on */
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAINS)
        != PRCM_DOMAIN_POWER_ON));
}
/*---------------------------------------------------------------------------*/
static void
lpm_wakeup_handler(void)
{
  power_domains_on();

  board_i2c_init();
}
/*---------------------------------------------------------------------------*/
static void
shutdown_handler(uint8_t mode)
{
  if(mode == LPM_MODE_SHUTDOWN) {
    buzzer_stop();
    SENSORS_DEACTIVATE(bmp_280_sensor);
    SENSORS_DEACTIVATE(opt_3001_sensor);
    SENSORS_DEACTIVATE(tmp_007_sensor);
    SENSORS_DEACTIVATE(sht_21_sensor);
    mpu_9250_sensor.configure(MPU_9250_SENSOR_SHUTDOWN, 0);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Declare a data structure to register with LPM.
 * We don't care about what power mode we'll drop to, we don't care about
 * getting notified before deep sleep. All we need is to be notified when we
 * wake up so we can turn power domains back on for I2C and SSI, and to make
 * sure everything on the board is off before CM3 shutdown.
 */
LPM_MODULE(sensortag_module, NULL, shutdown_handler, lpm_wakeup_handler);
/*---------------------------------------------------------------------------*/
void
board_init()
{
  /* Disable global interrupts */
  uint8_t int_disabled = ti_lib_int_master_disable();

  power_domains_on();

  /* Configure all clock domains to run at full speed */
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_SYSBUS, PRCM_CLOCK_DIV_1);
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_CPU, PRCM_CLOCK_DIV_1);
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_TIMER, PRCM_CLOCK_DIV_1);
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_SERIAL, PRCM_CLOCK_DIV_1);
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_PERIPH, PRCM_CLOCK_DIV_1);

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Enable GPT0 module - Wait for the clock to be enabled */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_TIMER0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Keys (input pullup) */
  ti_lib_rom_ioc_pin_type_gpio_input(BOARD_IOID_KEY_LEFT);
  ti_lib_rom_ioc_pin_type_gpio_input(BOARD_IOID_KEY_RIGHT);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_KEY_LEFT, IOC_IOPULL_UP);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_KEY_RIGHT, IOC_IOPULL_UP);

  /* I2C controller */
  board_i2c_init();

  /* Sensor interface */
  ti_lib_rom_ioc_pin_type_gpio_input(BOARD_IOID_MPU_INT);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_MPU_INT, IOC_IOPULL_DOWN);

  ti_lib_rom_ioc_pin_type_gpio_input(BOARD_IOID_REED_RELAY);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_REED_RELAY, IOC_IOPULL_DOWN);

  ti_lib_rom_ioc_pin_type_gpio_output(BOARD_IOID_MPU_POWER);

  /* Flash interface */
  ti_lib_rom_ioc_pin_type_gpio_output(BOARD_IOID_FLASH_CS);
  ti_lib_gpio_pin_write(BOARD_FLASH_CS, 1);

  buzzer_init();

  lpm_register_module(&sensortag_module);

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
