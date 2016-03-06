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
 * \addtogroup remote
 * @{
 *
 * \defgroup remote-power-mgmt RE-Mote power management driver
 *
 * The power management module is composed by a nano-watt (gating) timer and an
 * ultra-low power MCU, driving the RE-Mote power supply when connected to an
 * external battery, and allowing an application to enter a so-called "shutdown
 * mode".
 * While in shutdown mode, only the RTCC and the power management block is on,
 * effectively reducing the RE-Mote power consumption down to <~200nA.  The
 * nano Timer allows the RE-Mote to be awaken off shutdown mode after a given
 * period (from 100ms to 2 hours, default is 1 minute).  To change the shutdown
 * period, the R47 resistor (at the DELAY input pin, see the RE-Mote datasheet)
 * has to be changed.
 * See the TPL5110 datasheet ((Table 2 and 3) for more information about the R47
 * resistor value, below is a table resuming most common periods:
 *
 * +------------+------------+
 * | R47 (Ohm)  | Time       |
 * +------------+------------+
 * | 500        | 100ms      |
 * +------------+------------+
 * | 2.5K       | 500ms      |
 * +------------+------------+
 * | 5.202K     | 1s         |
 * +------------+------------+
 * | 22.021K    | 1min       |
 * +------------+------------+
 * | 42.887K    | 5min       |
 * +------------+------------+
 * | 57.434K    | 10min      |
 * +------------+------------+
 * | 92.233K    | 30min      |
 * +------------+------------+
 * | 170K       | 2h         |
 * +------------+------------+
 *
 * An application can enter the shutdown mode before the shutdown period expires
 * by invoking the PM_SHUTDOWN_NOW macro.
 * The on-board RTCC can also be used to drive the CC2538 off PM3 power mode, if
 * the application requires to retain RAM.  Note that while in shutdown mode the
 * RE-Mote will be powered off.
 *
 * @{
 *
 * \file
 * Header file for the RE-Mote Power Management driver
 */
/* -------------------------------------------------------------------------- */
#ifndef POWER_MGMT_H_
#define POWER_MGMT_H_
#include "dev/gpio.h"
/* -------------------------------------------------------------------------- */
#define PM_CMD_PORT_BASE     GPIO_PORT_TO_BASE(PM_CMD_PORT)
#define PM_CMD_PIN_MASK      GPIO_PIN_MASK(PM_CMD_PIN)
#define PM_DONE_PORT_BASE    GPIO_PORT_TO_BASE(PM_DONE_PORT)
#define PM_DONE_PIN_MASK     GPIO_PIN_MASK(PM_DONE_PIN)
/* -------------------------------------------------------------------------- */
/** \name Power Management return values
 * @{
 */
#define PM_SUCCESS           0
#define PM_ERROR             (-1)
#define PM_MAX_BITS          8
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management "done" signal
 * @{
 */
#define PM_SHUTDOWN_NOW     GPIO_SET_PIN(PM_DONE_PORT_BASE, PM_DONE_PIN_MASK)
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management timing values
 * @{
 */
#define PM_1_MILISECOND     1000L
#define PM_2_2_MILISECOND   2200L
#define PM_3_MILISECOND     3000L
#define PM_10_MILISECOND    10000L
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management commands
 * @{
 */
typedef enum {
  PM_CMD_PWR_ON             = 0x34,
  PM_CMD_PWR_OFF            = 0x35,
  PM_CMD_RST_HARD           = 0x36,
  PM_CMD_RST_TIMED          = 0x37,  /* Not implemented */
  PM_CMD_DTIMER_ON          = 0x38,
  PM_CMD_DTIMER_OFF         = 0x39,
  PM_CMD_DTIMER_TIMED       = 0x3A,  /* Not implemented */
  PM_CMD_PARAM_SET_MAX_TIME = 0x3B,  /* Not implemented */
  PM_CMD_GET_STATE          = 0x3C,
  PM_CMD_GET_FW_VERSION     = 0x3D,
  PM_MAX_NUM_CMDS
} pm_cmd_t;
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management status and masks
 * @{
 */
typedef enum {
  PM_IDLE,
  PM_SYSOFF_OFF,
  PM_SYSOFF_ON,
  PM_TIMER_DISABLED,
  PM_TIMER_ENABLED,
  PM_AWAITING_RTC_DIS,   /* Not implemented */
  PM_AWAITING_RTC_EVENT, /* Not implemented */
} pm_state_t;

#define PM_SYSOFF_ON_MASK             0x01
#define PM_TIMER_ENABLED_MASK         0x02
#define PM_AWAITING_RTC_EVENT_MASK    0x04

/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management functions
 * @{
 */
/** \brief Initializes the Power Management driver
 * \return \c   PM_SUCCESS if initialized, else \c PM_ERROR
 */
int8_t pm_init(void);
/* -------------------------------------------------------------------------- */
/** \brief Enable the shutdown mode, periodically driven by the Nano Timer
 * \return \c   PM_SUCCESS if successful, else \c PM_ERROR
 */
int8_t pm_enable_timer(void);
/* -------------------------------------------------------------------------- */
/** \brief Disable the Nano Timer
 * \return \c   PM_SUCCESS if successful, else \c PM_ERROR
 */
int8_t pm_disable_timer(void);
/* -------------------------------------------------------------------------- */
/** \brief Get the current state of the power management module
 * \param state Pointer to a variable to save the state
 * \return \c   PM_SUCCESS if successful, else \c PM_ERROR
 */
int8_t pm_get_state(uint8_t *state);
/* -------------------------------------------------------------------------- */
/** \brief Get the firmware version of the power management module
 * \param state Pointer to a variable to save the state
 * \return \c   PM_SUCCESS if successful, else \c PM_ERROR
 */
int8_t pm_get_firmware_version(uint8_t *state);
/* -------------------------------------------------------------------------- */
/** @} */
#endif /* POWER_MGMT_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
