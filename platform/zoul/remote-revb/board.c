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
 * \addtogroup remote-revb
 * @{
 *
 * \file
 *  Board-initialisation for the Zolertia's RE-Mote revision B platform
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "antenna-sw.h"
#include "dev/ioc.h"
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define CLEAR(x, y, z) GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y)); \
                       ioc_set_over(x, y, z);     \
                       GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y));  \
                       GPIO_CLR_PIN(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y));

#define SET(x, y, z)   GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y)); \
                       ioc_set_over(x, y, z);     \
                       GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y));  \
                       GPIO_SET_PIN(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y));

#define INPUT(x, y, z) GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y)); \
                       GPIO_SET_INPUT(GPIO_PORT_TO_BASE(x), GPIO_PIN_MASK(y));
/*---------------------------------------------------------------------------*/
/* The following functions configure the pins to minimize current draw, modules
 * and libraries using these pins are in charge of configuring as corresponding
 */
static void
configure_unused_pins(void)
{
  /* Port A pins */
#if UART_CONF_ENABLE == 0
  INPUT(GPIO_A_NUM, 0x00, IOC_OVERRIDE_DIS);
  INPUT(GPIO_A_NUM, 0x01, IOC_OVERRIDE_DIS);
#endif

  /* ADC config as default is pin as input with analogue function enabled */
#if ADC_SENSORS_ADC3_PIN == -1
  CLEAR(GPIO_A_NUM, 0x02, IOC_OVERRIDE_DIS);
#endif

  /* Both the user button and ADC6 use PA3 as input, but ADC6 uses
   * IOC_OVERRIDE_ANA to select as analogue, thus the pin is always in the
   * lowest current draw configuration in both cases
   */
#if PLATFORM_CONF_WITH_BUTTON == 0
#if ADC_SENSORS_ADC6_PIN == -1
  INPUT(GPIO_A_NUM, 0x03, IOC_OVERRIDE_DIS);
#endif
#endif

#if ADC_SENSORS_ADC2_PIN == -1
  INPUT(GPIO_A_NUM, 0x04, IOC_OVERRIDE_DIS);
#endif
#if ADC_SENSORS_ADC1_PIN == -1
  INPUT(GPIO_A_NUM, 0x05, IOC_OVERRIDE_DIS);
#endif

#if PLATFORM_WITH_MICRO_SD == 0
#if ADC_SENSORS_ADC4_PIN == -1
  INPUT(GPIO_A_NUM, 0x06, IOC_OVERRIDE_DIS);
#endif
#endif

#if PLATFORM_WITH_MICRO_SD == 0
#if ADC_SENSORS_ADC5_PIN == -1
  INPUT(GPIO_A_NUM, 0x07, IOC_OVERRIDE_DIS);
#endif
#endif

  /* Port B pins and CC1200 related ones */
#if REMOTE_DUAL_RF_ENABLED == 0
#if ANTENNA_SW_SELECT_DEFAULT == ANTENNA_SW_SELECT_2_4GHZ
  CLEAR(GPIO_B_NUM, 0x00, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_B_NUM, 0x01, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_B_NUM, 0x02, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_B_NUM, 0x03, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_B_NUM, 0x04, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_B_NUM, 0x05, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_C_NUM, 0x07, IOC_OVERRIDE_DIS);
#endif
#endif

  CLEAR(GPIO_B_NUM, 0x06, IOC_OVERRIDE_DIS); /* LED */
  CLEAR(GPIO_B_NUM, 0x07, IOC_OVERRIDE_DIS); /* LED */

  /* Port C pins */
#if UART_CONF_ENABLE == 0
  INPUT(GPIO_C_NUM, 0x00, IOC_OVERRIDE_DIS); /* UART1 */
  INPUT(GPIO_C_NUM, 0x01, IOC_OVERRIDE_DIS); /* UART1 */
#endif

  INPUT(GPIO_C_NUM, 0x02, IOC_OVERRIDE_DIS); /* I2C */
  INPUT(GPIO_C_NUM, 0x03, IOC_OVERRIDE_DIS); /* I2C */

#if PLATFORM_WITH_MICRO_SD == 0
  INPUT(GPIO_C_NUM, 0x04, IOC_OVERRIDE_DIS);
  INPUT(GPIO_C_NUM, 0x05, IOC_OVERRIDE_DIS);
  INPUT(GPIO_C_NUM, 0x06, IOC_OVERRIDE_DIS);
#endif

  /* Port D pins */
  CLEAR(GPIO_D_NUM, 0x00, IOC_OVERRIDE_DIS);
  CLEAR(GPIO_D_NUM, 0x01, IOC_OVERRIDE_DIS);

#if ANTENNA_SW_SELECT_DEFAULT == ANTENNA_SW_SELECT_2_4GHZ
  CLEAR(GPIO_D_NUM, 0x02, IOC_OVERRIDE_DIS);
#endif

  INPUT(GPIO_D_NUM, 0x03, IOC_OVERRIDE_DIS); /* RTCC interrupt */
  CLEAR(GPIO_D_NUM, 0x04, IOC_OVERRIDE_DIS); /* LED */
  CLEAR(GPIO_D_NUM, 0x05, IOC_OVERRIDE_DIS);
}
/*---------------------------------------------------------------------------*/
void
board_init()
{
  antenna_sw_config();
  configure_unused_pins();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

