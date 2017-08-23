/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup remote-power-mgmt-reva
 * @{
 *
 * RE-Mote power management and shutdown mode
 * @{
 *
 * \author
 *         Aitor Mejias   <amejias@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/gpio.h"
#include "sys/rtimer.h"
#include "power-mgmt.h"
/* -------------------------------------------------------------------------- */
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/* -------------------------------------------------------------------------- */
#define PM_CMD_LINE_SET      GPIO_SET_PIN(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK)
#define PM_CMD_LINE_CLR      GPIO_CLR_PIN(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK)
#define PM_CMD_LINE_READ     GPIO_READ_PIN(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK)
#define PM_CMD_AS_OUTPUT     GPIO_SET_OUTPUT(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK)
#define PM_CMD_AS_INPUT      GPIO_SET_INPUT(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK)
/* -------------------------------------------------------------------------- */
#define PM_NUMBITS(X)        (1 << ((X) - 1))
/* -------------------------------------------------------------------------- */
static uint8_t initialized = 0;
static uint8_t getData = 0;
/* -------------------------------------------------------------------------- */
static int8_t
pm_get_ack(void)
{
  uint16_t error = PM_ERROR;
  PM_CMD_AS_INPUT;
  clock_delay_usec(PM_3_MILISECOND);
  if(PM_CMD_LINE_READ) {
    error = PM_SUCCESS;
  }

  clock_delay_usec(PM_10_MILISECOND);
  PM_CMD_AS_OUTPUT;
  return error;
}
/* -------------------------------------------------------------------------- */
static int8_t
pm_send_cmd(uint8_t cmd)
{
  uint8_t i;

  PRINTF("PM: cmd %u\n", cmd);

  /* Enter command mode */
  PM_CMD_LINE_SET;
  clock_delay_usec(PM_1_MILISECOND);
  PM_CMD_LINE_CLR;

  /* Send command */
  for (i = PM_MAX_BITS; i > 0; i--) {
    clock_delay_usec(PM_1_MILISECOND);

    if (cmd & PM_NUMBITS(i)) {
      PM_CMD_LINE_SET;
    }
    else PM_CMD_LINE_CLR;
  }

  clock_delay_usec(PM_1_MILISECOND);
  PM_CMD_LINE_CLR;

  /* Receive command reply if any */
  if((cmd == PM_CMD_GET_STATE)  || (cmd == PM_CMD_GET_FW_VERSION)) {
    PM_CMD_AS_INPUT;
    clock_delay_usec(PM_2_2_MILISECOND);
    for (i = PM_MAX_BITS; i > 0; i--) {
      clock_delay_usec(PM_1_MILISECOND);
      PM_CMD_LINE_READ ? (getData |= PM_NUMBITS(i)) : (getData &= ~PM_NUMBITS(i));
    }

    PRINTF("PM: getData = 0x%02X\n", getData);
    clock_delay_usec(PM_2_2_MILISECOND);
    PM_CMD_AS_OUTPUT;
    PM_CMD_LINE_CLR;

    clock_delay_usec(PM_2_2_MILISECOND);

    return PM_SUCCESS;
  }

  /* Default case */
  clock_delay_usec(PM_1_MILISECOND);

  return pm_get_ack();
}
/* -------------------------------------------------------------------------- */
int8_t
pm_init(void)
{
  /* Configure and clear immediately */
  GPIO_SOFTWARE_CONTROL(PM_DONE_PORT_BASE, PM_DONE_PIN_MASK);
  GPIO_SET_OUTPUT(PM_DONE_PORT_BASE, PM_DONE_PIN_MASK);
  GPIO_CLR_PIN(PM_DONE_PORT_BASE, PM_DONE_PIN_MASK);

  /* Set as output/low to set IDLE state */
  GPIO_SOFTWARE_CONTROL(PM_CMD_PORT_BASE, PM_CMD_PIN_MASK);
  PM_CMD_AS_OUTPUT;
  PM_CMD_LINE_CLR;

  /* Ensure the battery charger is on, so we don't lock ourselves out and left
   * stranded in a state the CC2538 and components are kept off
   */

  if(pm_send_cmd(PM_CMD_PWR_ON) == PM_SUCCESS) {
    initialized = 1;
    PRINTF("PM: Initialized\n");
    return PM_SUCCESS;
  }

  PRINTF("PM: Failed to initialize\n");
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_enable_timer(void)
{
  if(!initialized) {
    return PM_ERROR;
  }

  if(pm_send_cmd(PM_CMD_DTIMER_ON) == PM_SUCCESS) {
    return PM_SUCCESS;
  }
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_disable_timer(void)
{
  if(!initialized) {
    return PM_ERROR;
  }

  if(pm_send_cmd(PM_CMD_DTIMER_OFF) == PM_SUCCESS) {
    return PM_SUCCESS;
  }
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_get_state(uint8_t *state)
{
  if(!initialized) {
    return PM_ERROR;
  }

  if(pm_send_cmd(PM_CMD_GET_STATE) == PM_SUCCESS) {
    *state = getData;
    PRINTF("PM: state %u\n", getData);

    return PM_SUCCESS;
  }
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_get_firmware_version(uint8_t *state)
{
  if(!initialized) {
    return PM_ERROR;
  }

  if (pm_send_cmd(PM_CMD_GET_FW_VERSION) == PM_SUCCESS) {
    *state = getData;
    printf("PM: FW Version %u\n", getData);
    return PM_SUCCESS;
  }
  return PM_ERROR;
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
