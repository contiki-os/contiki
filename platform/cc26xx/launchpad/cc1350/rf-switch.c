/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup rf-switch
 * @{
 *
 * \file
 * CC1350 LP RF switch driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "lpm.h"
#include "rf-core/rf-switch.h"
#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define POWER_PIN  IOID_30
#define SELECT_PIN IOID_1
/*---------------------------------------------------------------------------*/
static void
shutdown_handler(uint8_t mode)
{
  ti_lib_gpio_clear_dio(POWER_PIN);
}
/*---------------------------------------------------------------------------*/
/*
 * Declare a data structure to register with LPM. Always turn off the switch
 * when we are dropping to deep sleep. We let the RF driver turn it on though.
 */
LPM_MODULE(rf_switch_module, NULL, shutdown_handler, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
void
rf_switch_init()
{
  ti_lib_rom_ioc_pin_type_gpio_output(POWER_PIN);
  ti_lib_gpio_clear_dio(POWER_PIN);
  ti_lib_rom_ioc_pin_type_gpio_output(SELECT_PIN);
  ti_lib_gpio_clear_dio(SELECT_PIN);

  lpm_register_module(&rf_switch_module);
}
/*---------------------------------------------------------------------------*/
void
rf_switch_power_up()
{
  ti_lib_gpio_set_dio(POWER_PIN);
}
/*---------------------------------------------------------------------------*/
void
rf_switch_power_down()
{
  ti_lib_gpio_clear_dio(POWER_PIN);
}
/*---------------------------------------------------------------------------*/
void
rf_switch_select_path(uint8_t path)
{
  ti_lib_gpio_write_dio(SELECT_PIN, path);
}
/*---------------------------------------------------------------------------*/
/** @} */
