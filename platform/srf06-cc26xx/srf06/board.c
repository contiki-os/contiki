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
 *  Board-initialisation for the Srf06EB with a CC26xx EM.
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "ti-lib.h"
#include "lpm.h"
#include "prcm.h"
#include "hw_sysctl.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define PRCM_DOMAINS (PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL | \
                      PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_CPU | \
                      PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_VIMS)
/*---------------------------------------------------------------------------*/
#define LPM_DOMAINS (PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH)
/*---------------------------------------------------------------------------*/
static void
power_domains_on(void)
{
  /* Turn on relevant power domains */
  ti_lib_prcm_power_domain_on(LPM_DOMAINS);

  /* Wait for domains to power on */
  while((ti_lib_prcm_power_domain_status(LPM_DOMAINS)
        != PRCM_DOMAIN_POWER_ON));
}
/*---------------------------------------------------------------------------*/
static void
lpm_wakeup_handler(void)
{
  power_domains_on();
}
/*---------------------------------------------------------------------------*/
/*
 * Declare a data structure to register with LPM.
 * We don't care about what power mode we'll drop to, we don't care about
 * getting notified before deep sleep. All we need is to be notified when we
 * wake up so we can turn power domains back on
 */
LPM_MODULE(srf_module, NULL, NULL, lpm_wakeup_handler);
/*---------------------------------------------------------------------------*/
void
board_init()
{
  uint8_t int_disabled = ti_lib_int_master_disable();

  /* Turn on all power domains */
  ti_lib_prcm_power_domain_on(PRCM_DOMAINS);

  /* Wait for power on domains */
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAINS)
        != PRCM_DOMAIN_POWER_ON));

  /* Configure all clock domains to run at full speed */
  ti_lib_prcm_clock_configure_set(PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU |
                                  PRCM_DOMAIN_CPU | PRCM_DOMAIN_TIMER |
                                  PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH,
                                  PRCM_CLOCK_DIV_1);

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get()) ;

  /* Keys (input pullup) */
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_KEY_UP);
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_KEY_DOWN);
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_KEY_LEFT);
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_KEY_RIGHT);
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_KEY_SELECT);

  /* Turn off 3.3V domain (Powers the LCD and SD card reader): Output, low */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_3V3_EN);
  ti_lib_gpio_pin_write(BOARD_3V3_EN, 0);

  /* LCD CSn (output high) */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_LCD_CS);
  ti_lib_gpio_pin_write(BOARD_LCD_CS, 1);

  /* SD Card reader CSn (output high) */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_SDCARD_CS);
  ti_lib_gpio_pin_write(BOARD_SDCARD_CS, 1);

  /* Accelerometer (PWR output low, CSn output high) */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_ACC_PWR);
  ti_lib_gpio_pin_write(BOARD_ACC_PWR, 0);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_ACC_CS);
  ti_lib_gpio_pin_write(BOARD_IOID_ACC_CS, 1);

  /* Ambient light sensor (off, output low) */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_ALS_PWR);
  ti_lib_gpio_pin_write(BOARD_ALS_PWR, 0);
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_ALS_OUT);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_ALS_OUT, IOC_NO_IOPULL);

  lpm_register_module(&srf_module);

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
