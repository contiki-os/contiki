/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include "lib/sensors.h"
#include "dev/button-sensor.h"
#include "platform-conf.h"

#include <signal.h>

const struct sensors_sensor button_sensor;
static struct timer debouncetimer;

/* e.g. pADI_GP0 */
#define GPIO CC_CONCAT(pADI_GP, BUTTON_GPIO)

/* e.g. Ext_Int2_Handler */
#define HANDLER CC_CONCAT(CC_CONCAT(Ext_Int, BUTTON_INT), _Handler)

/* e.g. EINT2_IRQn */
#define IRQN CC_CONCAT(CC_CONCAT(EINT, BUTTON_INT), _IRQn)

void
HANDLER(void)
{
  pADI_INTERRUPT->EICLR = (1 << BUTTON_INT);
  if(!timer_expired(&debouncetimer)) {
    return;
  }
  timer_set(&debouncetimer, CLOCK_SECOND / 8);
  sensors_changed(&button_sensor);
}
static int
config(int type, int c)
{
  switch(type) {
  case SENSORS_HW_INIT:
    timer_set(&debouncetimer, 0);
    return 1;
  case SENSORS_ACTIVE:
    /* Set button as a GPIO input with pullup */
    GPIO->GPCON &= ~(3UL << (BUTTON_PIN * 2));
    GPIO->GPOEN &= ~(1UL << BUTTON_PIN);
    GPIO->GPPUL |= (1UL << BUTTON_PIN);

    /* Enable interrupt for the button (rise and fall) */
#if BUTTON_INT <= 3
    pADI_INTERRUPT->EI0CFG |= (0xaUL << (4 * (BUTTON_INT - 0)));
#elif BUTTON_INT <= 7
    pADI_INTERRUPT->EI1CFG |= (0xaUL << (4 * (BUTTON_INT - 4)));
#endif
    pADI_INTERRUPT->EICLR = (1 << BUTTON_INT);
    NVIC_EnableIRQ(IRQN);
    return 1;
  }
  return 0;
}
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    if(GPIO->GPIN & (1UL << BUTTON_PIN)) {
      return 1;
    }
    return 0;
  }
  return 0;
}
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, NULL, config, status);
