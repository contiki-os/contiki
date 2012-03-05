/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
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
 * This file is part of the Contiki operating system.
 */

/*
 * Portions of this file build on button-sensor.c in platforms sky and esb
 * This file contains ISRs: Keep it in the HOME bank.
 */

#include "dev/models.h"
#include "lib/sensors.h"
#include "dev/hwconf.h"
#include "dev/sensinode-sensors.h"

#if BUTTON_SENSOR_ON
static uint8_t p0ien;
static uint8_t p2ien;
static __data struct timer debouncetimer[2];

#ifdef MODEL_N740
HWCONF_PIN(BUTTON_1, 1, 0)
HWCONF_PORT_1_IRQ(BUTTON_1, 0)
HWCONF_PIN(BUTTON_2, 0, 4)
HWCONF_PORT_0_IRQ(BUTTON_2, 4)
#endif /* MODEL_N740 */

#ifdef MODEL_N711
HWCONF_PIN(BUTTON_1, 0, 6)
HWCONF_PORT_0_IRQ(BUTTON_1, 6)
HWCONF_PIN(BUTTON_2, 0, 7)
HWCONF_PORT_0_IRQ(BUTTON_2, 7)
#endif /* MODEL_N711 */

/*---------------------------------------------------------------------------*/
static
int value_b1(int type)
{
  return BUTTON_1_READ() || !timer_expired(&debouncetimer[0]);
}
/*---------------------------------------------------------------------------*/
static
int status_b1(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_1_IRQ_ENABLED();
    }
  return 0;
}
/*---------------------------------------------------------------------------*/
static
int configure_b1(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    /* Generates INT when pressed */
    BUTTON_1_IRQ_EDGE_SELECTD();
    BUTTON_1_SELECT();
    BUTTON_1_MAKE_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!BUTTON_1_IRQ_ENABLED()) {
        timer_set(&debouncetimer[0], 0);
        BUTTON_1_IRQ_FLAG_OFF();
        BUTTON_1_ENABLE_IRQ();
}
    } else {
      BUTTON_1_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static
int value_b2(int type)
{
  return BUTTON_2_READ() || !timer_expired(&debouncetimer[1]);
}
/*---------------------------------------------------------------------------*/
static
int status_b2(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_2_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static
int configure_b2(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    /* Generates INT when released */
    /* BUTTON_2_IRQ_EDGE_SELECTD(); */
    BUTTON_2_SELECT();
    BUTTON_2_MAKE_INPUT();
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!BUTTON_2_IRQ_ENABLED()) {
        timer_set(&debouncetimer[1], 0);
        BUTTON_2_IRQ_FLAG_OFF();
        BUTTON_2_ENABLE_IRQ();
      }
    } else {
      BUTTON_2_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
port_0_ISR(void) __interrupt (P0INT_VECTOR)
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* This ISR is for the entire port. Check if the interrupt was caused by our
   * button's pin. */
  /* Check B1 for N711 */
#ifdef MODEL_N711
  if(BUTTON_1_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer[0])) {
      timer_set(&debouncetimer[0], CLOCK_SECOND / 4);
      sensors_changed(&button_1_sensor);
    }
  }
#endif /* MODEL_N711 */
  if(BUTTON_2_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer[1])) {
      timer_set(&debouncetimer[1], CLOCK_SECOND / 4);
      sensors_changed(&button_2_sensor);
    }
  }
  P0IFG = 0;
  IRCON_P0IF = 0;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}
/*---------------------------------------------------------------------------*/
/* We only need this ISR for N740 */
#ifdef MODEL_N740
void
port_1_ISR(void) __interrupt (P1INT_VECTOR)
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* This ISR is for the entire port. Check if the interrupt was caused by our
   * button's pin. This can only be B1 for N740 */
  if(BUTTON_1_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer[0])) {
      timer_set(&debouncetimer[0], CLOCK_SECOND / 4);
      sensors_changed(&button_1_sensor);
    }
  }
  P1IFG = 0;
  IRCON2_P1IF = 0;
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}
#endif /* MODEL_N740 */

SENSORS_SENSOR(button_1_sensor, BUTTON_1_SENSOR, value_b1, configure_b1, status_b1);
SENSORS_SENSOR(button_2_sensor, BUTTON_2_SENSOR, value_b2, configure_b2, status_b2);
#endif /* BUTTON_SENSOR_ON */
