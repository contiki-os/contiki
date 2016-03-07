/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 *
 */
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup remote
 * @{
 *
 * \defgroup remote-rtcc RE-Mote Real Time Clock Calendar
 *
 * Driver for the RE-Mote on-board ultra-low power RTCC (Real Time Clock
 * Calendar)
 * @{
 *
 * \file
 * Header file for the RE-Mote RF antenna switch
 */
/* -------------------------------------------------------------------------- */
#ifndef RTCC_H_
#define RTCC_H_
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include "i2c.h"
/* -------------------------------------------------------------------------- */
/**
 * \name Callback function to handle the RTCC alarm interrupt and macro
 * @{
 */
#define RTCC_REGISTER_INT1(ptr) rtcc_int1_callback = ptr;
extern void (*rtcc_int1_callback)(uint8_t value);
/** @} */
/* -------------------------------------------------------------------------- */
/** \name AB08XX Address registers
 * @{
 */
/* -------------------------------------------------------------------------- */
/* Time/date registers (no offset) */
#define CENTHS_ADDR            0x00
#define SEC_ADDR               0x01
#define MIN_ADDR               0x02
#define HOUR_ADDR              0x03
#define DAY_ADDR               0x04
#define MONTHS_ADDR            0x05
#define YEAR_ADDR              0x06
#define WEEKDAYLS_ADDR         0x07

/* Alarm registers */
#define ALARM_MAP_OFFSET       0x08
#define HUNDREDTHS_ALARM_ADDR  0x00
#define SECONDS_ALARM_ADDR     0x01
#define MINUTES_ALARM_ADDR     0x02
#define HOURS_ALARM_ADDR       0x03
#define DAY_ALARMS_ADDR        0x04
#define MONTHS_ALARM_ADDR      0x05
#define WEEKDAYS_ALARM_ADDR    0x06

/* Configuration registers */
#define CONFIG_MAP_OFFSET      0x0F
#define STATUS_ADDR            0x00
#define CTRL_1_ADDR            0x01
#define CTRL_2_ADDR            0x02
#define INT_MASK_ADDR          0x03
#define SQW_ADDR               0x04
#define CAL_XT_ADDR            0x05
#define CAL_RC_HI_ADDR         0x06
#define CAL_RC_LO_ADDR         0x07
#define INT_POL_ADDR           0x08
#define TIMER_CONTROL_ADDR     0x09
#define TIMER_COUNTDOWN_ADDR   0x0A
#define TIMER_INITIAL_ADDR     0x0B
#define WDT_ADDR               0x0C
#define OSC_CONTROL_ADDR       0x0D
#define OSC_STATUS_ADDR        0x0E
#define CONF_KEY_ADDR          0x10
#define TRICKLE_ADDR           0x11
#define BREF_CTRL_ADDR         0x12
#define AF_CTRL_ADDR           0x17
#define BAT_MODE_IO_ADDR       0x18
#define ASTAT_ADDR             0x20
#define OCTRL_ADDR             0x21
#define EXT_ADDR               0x30
/* 256b. The upper 2 bits are taken from XADS field */
#define RAM_1_ADDR             (CONFIG_MAP_OFFSET + 0x31)
/* 256b. The upper 2 bits are taken from XADA field */
#define RAM_2_ADDR             (CONFIG_MAP_OFFSET + 0x71)
/** @} */
/* -------------------------------------------------------------------------- */
/** \name RTCC Bitmasks and shifts
 * @{
 */
#define STATUS_CB              0x80
#define STATUS_BAT             0x40
#define STATUS_WDT             0x20
#define STATUS_BL              0x10
#define STATUS_TIM             0x08
#define STATUS_ALM             0x04
#define STATUS_EX2             0x02
#define STATUS_EX1             0x01

#define CTRL1_WRTC             0x01
#define CTRL1_ARST             0x04
#define CTRL1_OUT              0x10
#define CTRL1_OUTB             0x20
#define CTRL1_1224             0x40
#define CTRL1_STOP             0x80

/* Defines the nIRQ pin control */
#define CTRL2_OUT1S_NIRQ_OUT        0x00
#define CTRL2_OUT1S_NIRQ_SQW_OUT    0x01
#define CTRL2_OUT1S_NIRQ_SQW_NIRQ   0x02
#define CTRL2_OUT1S_NIRQ_NAIRQ_OUT  0x03

/* Defines the nIRQ2 pin control */
#define CTRL2_OUT2S_SQW_OUT         0x04
#define CTRL2_OUT2S_NAIRQ_OUTB      0x0C
#define CTRL2_OUT2S_TIRQ_OUTB       0x10
#define CTRL2_OUT2S_NTIRQ_OUTB      0x14
#define CTRL2_OUT2S_OUTB            0x1C

/* Interrupt Mask */
#define INTMASK_EX1E                0x01
#define INTMASK_EX2E                0x02
#define INTMASK_AIE                 0x04
#define INTMASK_TIE                 0x08
#define INTMASK_BLIE                0x10
#define INTMASK_IM_HIGH             0x20
#define INTMASK_IM_MED              0x40
#define INTMASK_IM_LOW              0x60
#define INTMASK_CEB                 0x80

/* Timer countdown control */
#define COUNTDOWN_TIMER_TE          0x80
#define COUNTDOWN_TIMER_TM          0x40
#define COUNTDOWN_TIMER_TRPT        0x20
#define COUNTDOWN_TIMER_RPT_SECOND  0x1C
#define COUNTDOWN_TIMER_RPT_MINUTE  0x18
#define COUNTDOWN_TIMER_RPT_HOUR    0x24
#define COUNTDOWN_TIMER_RPT_DAY     0x10
#define COUNTDOWN_TIMER_RPT_WEEK    0x0C
#define COUNTDOWN_TIMER_RPT_MONTH   0x08
#define COUNTDOWN_TIMER_RPT_YEAR    0x04
#define COUNTDOWN_TIMER_RPT_SHIFT   0x02
#define COUNTDOWN_TIMER_TFS_ONE     0x01
#define COUNTDOWN_TIMER_TFS_TWO     0x02
#define COUNTDOWN_TIMER_TFS_THREE   0x03

/* Oscillator control */
#define OSCONTROL_ACIE              0x01
#define OSCONTROL_OFIE              0x02
#define OSCONTROL_FOS               0x08
#define OSCONTROL_AOS               0x10
#define OSCONTROL_ACAL_NO_CAL       0x00
#define OSCONTROL_ACAL_17_MIN       0x40
#define OSCONTROL_ACAL_9_MIN        0x60
#define OSCONTROL_OSEL              0x80

/** @} */
/* -------------------------------------------------------------------------- */
/** \name RTCC operational values
 * @{
 */
/* I2C address (7-bits) */
#define AB08XX_ADDR                0x69
#define INT_BUFF_SIZE              20L
#define TCS_DIODE_3K               (TCS_ENABLE + 0x05)
#define TCS_DIODE_6K               (TCS_ENABLE + 0x06)
#define TCS_DIODE_11K              (TCS_ENABLE + 0x07)
#define RTCC_TOGGLE_PM_BIT         0x20
#define RTCC_FIX_10THS_HUNDRETHS   0xF0
#define RTCC_FIX_100THS_HUNDRETHS  0xFF
#define RTCC_TD_MAP_SIZE           (WEEKDAYLS_ADDR + 1)
#define RTCC_ALARM_MAP_SIZE        (WEEKDAYS_ALARM_ADDR + 1)
#define RTCC_CONFIG_MAP_SIZE       (BREF_CTRL_ADDR + 1)
/** @} */
/* -------------------------------------------------------------------------- */
/** \name RTCC error values
 * @{
 */
#define AB08_ERROR                 (-1)
#define AB08_SUCCESS               0x00
/** @} */
/* -------------------------------------------------------------------------- */
/** \name RTCC enumeration and options
 * @{
 */
enum {
  RTCC_PRINT_DATE = 0,
  RTCC_PRINT_CONFIG,
  RTCC_PRINT_ALARM,
  RTCC_PRINT_ALARM_DEC,
  RTCC_PRINT_DATE_DEC,
  RTCC_PRINT_MAX,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_ALARM_OFF = 0,
  RTCC_ALARM_ON,
  RTCC_ALARM_MAX,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_CMD_UNLOCK = 0,
  RTCC_CMD_LOCK,
  RTCC_CMD_ENABLE,
  RTCC_CMD_STOP,
  RTCC_CMD_MAX,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_24H_MODE = 0,
  RTCC_12H_MODE_AM,
  RTCC_12H_MODE_PM,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_CENTURY_19XX_21XX = 1,
  RTCC_CENTURY_20XX,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_REPEAT_NONE = 0,
  RTCC_REPEAT_YEAR,
  RTCC_REPEAT_MONTH,
  RTCC_REPEAT_WEEK,
  RTCC_REPEAT_DAY,
  RTCC_REPEAT_HOUR,
  RTCC_REPEAT_MINUTE,
  RTCC_REPEAT_SECOND,
  RTCC_REPEAT_10THS,
  RTCC_REPEAT_100THS,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_CONFKEY_OSCONTROL = 0xA1,
  RTCC_CONFKEY_SWRESET   = 0x3C,
  RTCC_CONFKEY_DEFREGS   = 0x9D,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_CAL_XT_OSC = 0,
  RTCC_CAL_RC_OSC,
};
/* -------------------------------------------------------------------------- */
enum {
  RTCC_AUTOCAL_DISABLE = 0,
  RTCC_AUTOCAL_ONCE,
  RTCC_AUTOCAL_17_MIN,
  RTCC_AUTOCAL_9_MIN,
};
/** @} */
/* -------------------------------------------------------------------------- */
/** \name Readable Date and time memory map implementation
 *
 * This simplified structure allows the user to set date/alarms with a
 * reduced structure, without the bit-defined restrictions of the memory map,
 * using decimal values
 *
 * @{
 */
typedef struct ab0805_struct_simple_td_reg {
  uint8_t miliseconds;
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t day;
  uint8_t months;
  uint8_t years;
  uint8_t weekdays;
  uint8_t mode;
  uint8_t century;
}  __attribute__ ((packed)) simple_td_map;
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name RTCC User functions
 * @{
 */

/**
 * \brief Set the time and date
 * \param *data Time and date value (decimal format)
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_set_time_date(simple_td_map *data);

/**
 * \brief Get the current time and date
 * \param *data buffer to store the results
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_get_time_date(simple_td_map *data);

/**
 * \brief Print data from the RTCC module, either from the memory
 *        map (values in BCD) or actual readable data (decimal).
 * \param value value to print, see RTCC_PRINT_* options available
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_print(uint8_t value);

/**
 * \brief Configure the RTCC to match an alarm counter
 * \param data date and time values (in decimal) to match against
 * \param state set on/off the alarm interruption
 * \param repeat set the frequency of the alarm (minute, hourly, daily, etc.)
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_set_alarm_time_date(simple_td_map *data, uint8_t state,
                                uint8_t repeat);

/**
 * \brief Manually calibrate the RTCC
 * \param mode oscillator to calibrate
 * \param adjust value (in ppm) to adjust the oscillator
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_set_calibration(uint8_t mode, int32_t adjust);

/**
 * \brief Set the autocallibration period
 * \param period autocalibration configuration
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_set_autocalibration(uint8_t period);

/**
 * \brief Initialize the RTCC, configures the I2C bus, interrupts and registers
 * \return
 * \           AB08_SUCCESS date/time set
 * \           AB08_ERROR failed to set time/date (enable DEBUG for more info)
 */
int8_t rtcc_init(void);
/** @} */
/* -------------------------------------------------------------------------- */
#endif /* ifndef RTCC_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
