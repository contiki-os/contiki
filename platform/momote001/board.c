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
 * \addtogroup sensortag-common-peripherals
 * @{
 *
 * \file
 *  Board-initialisation for the Srf06EB with a CC13xx/CC26xx EM.
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
static void
wakeup_handler(void)
{
  /* Turn on the PERIPH PD */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
        != PRCM_DOMAIN_POWER_ON));
}
/*---------------------------------------------------------------------------*/
/*
 * Declare a data structure to register with LPM.
 * We don't care about what power mode we'll drop to, we don't care about
 * getting notified before deep sleep. All we need is to be notified when we
 * wake up so we can turn power domains back on
 */
LPM_MODULE(srf_module, NULL, NULL, wakeup_handler, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static void
configure_unused_pins(void)
{
  /* DP[0..3] */
  ti_lib_ioc_pin_type_gpio_input(IOID_0);
  ti_lib_ioc_io_port_pull_set(IOID_0, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_1);
  ti_lib_ioc_io_port_pull_set(IOID_1, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_2);
  ti_lib_ioc_io_port_pull_set(IOID_2, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_3);
  ti_lib_ioc_io_port_pull_set(IOID_3, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_4);
  ti_lib_ioc_io_port_pull_set(IOID_4, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_5);
  ti_lib_ioc_io_port_pull_set(IOID_5, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_6);
  ti_lib_ioc_io_port_pull_set(IOID_6, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_7);
  ti_lib_ioc_io_port_pull_set(IOID_7, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_8);
  ti_lib_ioc_io_port_pull_set(IOID_8, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_9);
  ti_lib_ioc_io_port_pull_set(IOID_9, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_10);
  ti_lib_ioc_io_port_pull_set(IOID_10, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_11);
  ti_lib_ioc_io_port_pull_set(IOID_11, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_12);
  ti_lib_ioc_io_port_pull_set(IOID_12, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_13);
  ti_lib_ioc_io_port_pull_set(IOID_13, IOC_IOPULL_DOWN);
  ti_lib_ioc_pin_type_gpio_input(IOID_14);
  ti_lib_ioc_io_port_pull_set(IOID_14, IOC_IOPULL_DOWN);
}

/*---------------------------------------------------------------------------*/
void
board_init()
{
  uint8_t int_disabled = ti_lib_int_master_disable();

  /* Turn on relevant PDs */
  wakeup_handler();

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  lpm_register_module(&srf_module);

  configure_unused_pins();

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
