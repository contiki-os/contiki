/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 * \defgroup remote-power-mgmt-revb RE-Mote power management driver
 *
 * Driver control the power management of the RE-Mote rev B platform. This
 * driver is an I2C communication with external power manager chip
 * that controls few functions of the board. Note the driver will work if
 * powered both over USB and external battery, but the shutdown mode will only
 * be actually working with external battery
 *
 * @{
 *
 * \file
 *         Header file for the RE-Mote Power Management driver
 *
 * \ingroup remote-power-mgmt-revb
 * @{
 */
/* -------------------------------------------------------------------------- */
#ifndef POWER_MGMT_H_
#define POWER_MGMT_H_
#include "dev/gpio.h"
/* -------------------------------------------------------------------------- */
/**
 * \name Power management controller
 * @{
 */
/* -------------------------------------------------------------------------- */
#define PWR_MNGMT_ADDR          0x7F     /**< Power Management slave address */
/** @} */
/* -------------------------------------------------------------------------- */
#define PM_ENABLE_PORT_BASE     GPIO_PORT_TO_BASE(PM_ENABLE_PORT)
#define PM_ENABLE_PIN_MASK      GPIO_PIN_MASK(PM_ENABLE_PIN)
/* -------------------------------------------------------------------------- */
/** \name Power Management return values
 * @{
 */
#define PM_SUCCESS              0
#define PM_ERROR                (-1)
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management Registers
 * @{
 */
typedef enum {
  PM_VBAT = 34,           /* Connect/Disconnect battery, it can be "1" (ON) or "0" (OFF) */
  PM_CURRENT_CHARGE,      /* register with pm_charge_current_states options */
  PM_CHARGE_ENABLE,       /* Enable the current charge mode on the battery */
  PM_SYSOFF,              /* Control SYSOFF pin on the battery charger */
  PM_EXTERNAL_REFERENCE,  /* Use reference fot vdd. 0: internal 1:External */
  PM_HARD_SLEEP_CONFIG,   /* Disconnect Battery until event ext2 on RTCC. */
  PM_SOFT_SLEEP_CONFIG,   /* Disconnect Batt without RTCC until PM_SOFT_TIME val */
  PM_SOFT_TIME,           /* Time Out used by system time on hard/soft config */
  PM_GET_VDD,             /* Command to get the current VDD on the system */
  PM_FW_VERSION,          /* Get the Firmware version */
  PM_GET_NUM_CYCLES,      /* Obtain the current sleep cycles at moment */
  PM_CMD_RST_HARD,        /* Disconnect and reconnect the battery */
  PM_MAX_COMMANDS         /* Maximum register commands. Register not valid */
} pm_registers_t;
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management status and masks
 * @{
 */
#define PM_ENABLE                       1    /* Pin status on */
#define PM_DISABLE                      0    /* Pin status off */

#define BATT_CHARGED_VAL                0
#define BATT_UNCHARGED_VAL              1

#define PM_REFERENCE_INTERNAL           0
#define PM_REFERENCE_EXTERNAL           1

/* The PIC interval time is 57.20ms given in PM_SOFT_SHTDN_INTERVAL, to set a
 * timeout period (using the soft shutdown mode) then calculate the time using
 * this value as base, i.e 10 minutes would be roughly 600000/57.20 ~10490.
 * The maximum value is 28 days!
 */
#define PM_SOFT_SHTDN_INTERVAL          5720

#define PM_SOFT_SHTDN_0_5_SEC           8
#define PM_SOFT_SHTDN_1_SEC             17
#define PM_SOFT_SHTDN_1_4_SEC           25
#define PM_SOFT_SHTDN_5_7_SEC           100
#define PM_SOFT_SHTDN_30_SEC            524
#define PM_SOFT_SHTDN_1_MIN             1049
#define PM_SOFT_SHTDN_5_MIN             5245
#define PM_SOFT_SHTDN_10_MIN            10490
#define PM_SOFT_SHTDN_1_H               62937
#define PM_SOFT_SHTDN_24_H              1510490
#define PM_SOFT_SHTDN_7_DAYS            10573427
#define PM_SOFT_SHTDN_14_DAYS           21146853
#define PM_SOFT_SHTDN_28_DAYS           42383709
#define PM_SOFT_SHTDN_TEST              0xCC00FFEE

typedef enum {
  PM_USB100 = 0,     /* 100mA chage */
  PM_USB500,         /* 500mA chage */
  PM_USBEXT,         /* USB set by external resistor to ILIM (1K) */
  PM_USBSUSPEND      /* USB in suspend mode */
} pm_charge_current_states;

/** @} */
/* -------------------------------------------------------------------------- */
/** \name Power Management functions
 * @{
 */
/** \brief Initializes the Power Management driver
 * \return PM_SUCCESS if initialized, else PM_ERROR
 */
int8_t pm_enable(void);

/** \brief Gets the current firmware version of power management module
 * \param fwver pointer to get the value of firmware
 * \return PM_SUCCESS if success, else PM_ERROR
 */
int8_t pm_get_fw_ver(uint8_t *fwver);

/** \brief Disconnects the board battery and enter shutdown mode
 * PM_SOFT/HARD_SLEEP_CONFIG
 * \param type Hard shutdown (RTCC based) or soft (PIC-governed)
 * \return Value byte of the register requested.
 */
int8_t pm_shutdown_now(uint8_t type);

/** \brief Configure Internal Timeout for Hard and Soft shutdown modes.
 * In Hard mode, any positive value counts as internal security timeout.
 * In Soft mode, this value is needed.
 * \param time value as timeout maximum
 * \return PM_SUCCESS if success, else PM_ERROR
 */
int8_t pm_set_timeout(uint32_t time);

/** \brief Gets the current timeout value configured in power management module
 * \return value with timeout.
 */
uint32_t pm_get_timeout(void);

/** \brief Gets current cycles. incremented each shutdown cycle
 * \return Number of shutdown cycles
 */
uint32_t pm_get_num_cycles(void);

/** \brief Reads the voltage of the external battery if connected to VIN pin
 * \param state pointer to get the voltage value
 * \return PM_SUCCESS if success, else PM_ERROR
 */
int8_t pm_get_voltage(uint16_t *state);

/** \brief Restarts the on-board low-power PIC, provoking a board reset
 * \return PM_SUCCESS if success, else PM_ERROR
 */
int8_t pm_reset_system(void);
/* -------------------------------------------------------------------------- */
/** @} */
#endif /* POWER_MGMT_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
