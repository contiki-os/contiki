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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup remote-rtcc
 * @{
 *
 * Driver for the RE-Mote RTCC (Real Time Clock Calendar)
 * @{
 *
 * \file
 * Driver for the RE-Mote RF Real Time Clock Calendar (RTCC)
 *
 * \author
 *
 * Antonio Lignan <alinan@zolertia.com>
 * Aitor Mejias <amejias@zolertia.com>
 * Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/gpio.h"
#include "dev/i2c.h"
#include "rtcc.h"
#include "rtcc-config.h"
#include "dev/leds.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define RTC_INT1_PORT_BASE  GPIO_PORT_TO_BASE(RTC_INT1_PORT)
#define RTC_INT1_PIN_MASK   GPIO_PIN_MASK(RTC_INT1_PIN)
/*---------------------------------------------------------------------------*/
/* Callback pointers when interrupt occurs */
void (*rtcc_int1_callback)(uint8_t value);
/* -------------------------------------------------------------------------- */
static const char *ab080x_td_register_name[] =
{
  "Mseconds",
  "Seconds",
  "Minutes",
  "Hours",
  "Days",
  "Months",
  "Years",
  "Weekdays",
};
/* -------------------------------------------------------------------------- */
static const char *ab080x_config_register_name[] =
{
  "STATUS",
  "CTRL1",
  "CTRL2",
  "INTMASK",
  "SQW",
  "CAL_XT",
  "CAL_RCU",
  "CAL_RCL",
  "INTPOL",
  "TIMER_CTRL",
  "TIMER_CDOWN",
  "TIMER_INIT",
  "WDT",
  "OSC_CTRL",
  "OSC_STAT",
  "CONF_KEY",
  "TRICKLE",
  "BREF",
};
/*---------------------------------------------------------------------------*/
static uint8_t
bcd_to_dec(uint8_t val)
{
  return (uint8_t)(((val >> 4) * 10) + (val % 16));
}
/*---------------------------------------------------------------------------*/
static uint8_t
dec_to_bcd(uint8_t val)
{
  return (uint8_t)(((val / 10) << 4) + (val % 10));
}
/*---------------------------------------------------------------------------*/
static uint8_t
check_leap_year(uint8_t val)
{
  return ((val % 4) && (val % 100)) || (val % 400);
}
/*---------------------------------------------------------------------------*/
static uint16_t
ab08_read_reg(uint8_t reg, uint8_t *buf, uint8_t regnum)
{
  i2c_master_enable();
  if(i2c_single_send(AB08XX_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(AB08XX_ADDR, buf, regnum) == I2C_MASTER_ERR_NONE) {
      return AB08_SUCCESS;
    }
  }
  return AB08_ERROR;
}
/*---------------------------------------------------------------------------*/
static int8_t
ab08_write_reg(uint8_t reg, uint8_t *buf, uint8_t regnum)
{
  uint8_t i, buff[INT_BUFF_SIZE];

  if(regnum > (INT_BUFF_SIZE - 1)) {
    return AB08_ERROR;
  }

  /* FIXME: Replace by single_send/burst_send */

  buff[0] = reg;
  for(i = 0; i < regnum; i++) {
    buff[(i + 1)] = buf[i];
  }

  i2c_master_enable();
  if(i2c_burst_send(AB08XX_ADDR, buff, (regnum + 1)) == I2C_MASTER_ERR_NONE) {
    return AB08_SUCCESS;
  }

  return AB08_ERROR;
}
/*---------------------------------------------------------------------------*/
static void
write_default_config(void)
{
  const ab080x_register_config_t *settings;
  settings = ab080x_default_setting;
  uint8_t i, len = (sizeof(ab080x_default_setting) / sizeof(ab080x_register_config_t));

  for(i = 0; i < len; i++) {
    ab08_write_reg(settings[i].reg, (uint8_t *)&settings[i].val, 1);
  }
}
/*---------------------------------------------------------------------------*/
static int8_t
ab08_key_reg(uint8_t unlock)
{
  if((unlock != RTCC_CONFKEY_OSCONTROL) && (unlock != RTCC_CONFKEY_SWRESET) &&
     (unlock != RTCC_CONFKEY_DEFREGS)) {
    PRINTF("RTC: invalid confkey values\n");
    return AB08_ERROR;
  }

  if(ab08_write_reg((CONFIG_MAP_OFFSET + CONF_KEY_ADDR), &unlock, 1)) {
    PRINTF("RTC: failed to write to confkey register\n");
    return AB08_ERROR;
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int8_t
ab08_read_status(uint8_t *buf)
{
  return ab08_read_reg((STATUS_ADDR + CONFIG_MAP_OFFSET), buf, 1);
}
/*---------------------------------------------------------------------------*/
static int8_t
ab08_ctrl1_config(uint8_t cmd)
{
  uint8_t ctrl1 = 0;

  if(cmd >= RTCC_CMD_MAX) {
    return AB08_ERROR;
  }

  if(ab08_read_reg((CONFIG_MAP_OFFSET + CTRL_1_ADDR), &ctrl1, 1)) {
    PRINTF("RTC: failed to retrieve CTRL1 register\n");
    return AB08_ERROR;
  }

  switch(cmd) {
  case RTCC_CMD_LOCK:
    ctrl1 &= ~CTRL1_WRTC;
    break;
  case RTCC_CMD_UNLOCK:
    ctrl1 |= CTRL1_WRTC;
    break;
  case RTCC_CMD_ENABLE:
    ctrl1 &= ~CTRL1_STOP;
    break;
  case RTCC_CMD_STOP:
    ctrl1 |= CTRL1_STOP;
    break;
  default:
    return AB08_ERROR;
  }

  if(ab08_write_reg((CONFIG_MAP_OFFSET + CTRL_1_ADDR),
                    &ctrl1, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to write to the CTRL1 register\n");
    return AB08_ERROR;
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int8_t
ab08_check_td_format(simple_td_map *data, uint8_t alarm_state)
{
  /* Using fixed values as these are self-indicative of the variable */
  if((data->seconds > 59) || (data->minutes > 59) || (data->hours > 23)) {
    return AB08_ERROR;
  }

  if((data->months > 12) || (data->weekdays > 7) || (data->day > 31)) {
    return AB08_ERROR;
  }

  /* Fixed condition for February (month 2) */
  if(data->months == 2) {
    if(check_leap_year(data->years)) {
      if(data->day > 29) {
        return AB08_ERROR;
      }
    } else {
      if(data->day > 28) {
        return AB08_ERROR;
      }
    }
  }

  /* Alarm doesn't care about year */
  if(!alarm_state) {
    /* AB08X5 Real-Time Clock Family, page 55 (year up to 2199) */
    if(data->years > 199) {
      return AB08_ERROR;
    }
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_set_time_date(simple_td_map *data)
{
  uint8_t aux = 0;
  uint8_t rtc_buffer[RTCC_TD_MAP_SIZE];

  if(ab08_check_td_format(data, 0) == AB08_ERROR) {
    PRINTF("RTC: Invalid time/date values\n");
    return AB08_ERROR;
  }

  if(ab08_read_reg((CTRL_1_ADDR + CONFIG_MAP_OFFSET),
                   &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve CONTROL1 register\n");
    return AB08_ERROR;
  }

  rtc_buffer[WEEKDAYLS_ADDR] = dec_to_bcd(data->weekdays);
  rtc_buffer[YEAR_ADDR] = dec_to_bcd(data->years);
  rtc_buffer[MONTHS_ADDR] = dec_to_bcd(data->months);
  rtc_buffer[DAY_ADDR] = dec_to_bcd(data->day);
  rtc_buffer[HOUR_ADDR] = dec_to_bcd(data->hours);
  rtc_buffer[MIN_ADDR] = dec_to_bcd(data->minutes);
  rtc_buffer[SEC_ADDR] = dec_to_bcd(data->seconds);
  rtc_buffer[CENTHS_ADDR] = dec_to_bcd(data->miliseconds);

  /* Check if we are to set the time in 12h/24h format */
  if(data->mode == RTCC_24H_MODE) {
    aux &= ~CTRL1_1224;
  } else {
    if((data->hours == 0) || (data->hours > 12)) {
      PRINTF("RTC: Invalid hour configuration (12h mode selected)\n");
      return AB08_ERROR;
    }
    aux |= CTRL1_1224;
    if(data->mode == RTCC_12H_MODE_PM) {
      /* Toggle bit for PM */
      rtc_buffer[HOUR_ADDR] |= RTCC_TOGGLE_PM_BIT;
    } else {
      PRINTF("RTC: Invalid time mode selected\n");
      return AB08_ERROR;
    }
  }

  /* Write the 12h/24h config */
  if(ab08_write_reg((CTRL_1_ADDR + CONFIG_MAP_OFFSET),
                    &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to write 12h/24h configuration\n");
    return AB08_ERROR;
  }

  /* Reading the STATUS register with the CONTROL1.ARST set will clear the
   * interrupt flags, we write directly to the register without caring its
   * actual status and let the interrupt handler take care of any pending flag
   */

  if(ab08_read_reg((STATUS_ADDR + CONFIG_MAP_OFFSET), &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve STATUS register\n");
    return AB08_ERROR;
  }

  if(data->century == RTCC_CENTURY_20XX) {
    aux |= STATUS_CB;
  } else if(data->century == RTCC_CENTURY_19XX_21XX) {
    aux |= ~STATUS_CB;
  } else {
    PRINTF("RTC: invalid century value\n");
    return AB08_ERROR;
  }

  PRINTF("RTC: current STATUS value 0x%02X\n", aux);

  if(ab08_write_reg((STATUS_ADDR + CONFIG_MAP_OFFSET), &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to write century to STATUS register\n");
    return AB08_ERROR;
  }

  /* Set the WRTC bit to enable writting to the counters */
  if(ab08_ctrl1_config(RTCC_CMD_UNLOCK) == AB08_ERROR) {
    return AB08_ERROR;
  }

  /* Write the buffers but the mode and century fields (used only for config) */
  if(ab08_write_reg(CENTHS_ADDR, rtc_buffer,
                    RTCC_TD_MAP_SIZE) == AB08_ERROR) {
    PRINTF("RTC: failed to write date configuration\n");
    return AB08_ERROR;
  }

  /* Lock the RTCC and return */
  if(ab08_ctrl1_config(RTCC_CMD_LOCK) == AB08_ERROR) {
    return AB08_ERROR;
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_get_time_date(simple_td_map *data)
{
  uint8_t rtc_buffer[RTCC_TD_MAP_SIZE];

  if(ab08_read_reg(CENTHS_ADDR, rtc_buffer,
                   RTCC_TD_MAP_SIZE) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve date and time values\n");
    return AB08_ERROR;
  }

  data->weekdays = bcd_to_dec(rtc_buffer[WEEKDAYLS_ADDR]);
  data->years = bcd_to_dec(rtc_buffer[YEAR_ADDR]);
  data->months = bcd_to_dec(rtc_buffer[MONTHS_ADDR]);
  data->day = bcd_to_dec(rtc_buffer[DAY_ADDR]);
  data->hours = bcd_to_dec(rtc_buffer[HOUR_ADDR]);
  data->minutes = bcd_to_dec(rtc_buffer[MIN_ADDR]);
  data->seconds = bcd_to_dec(rtc_buffer[SEC_ADDR]);
  data->miliseconds = bcd_to_dec(rtc_buffer[CENTHS_ADDR]);

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_set_alarm_time_date(simple_td_map *data, uint8_t state, uint8_t repeat,
                         uint8_t trigger)
{
  uint8_t aux[4], buf[RTCC_ALARM_MAP_SIZE];

  if((trigger != RTCC_TRIGGER_INT2) && (trigger != RTCC_TRIGGER_INT1) &&
     (trigger != RTCC_TRIGGER_BOTH)) {
    PRINTF("RTC: invalid trigger pin\n");
    return AB08_ERROR;
  }

  if(state == RTCC_ALARM_OFF) {
    if(ab08_read_reg((INT_MASK_ADDR + CONFIG_MAP_OFFSET),
                     &aux[0], 1) == AB08_ERROR) {
      PRINTF("RTC: failed to retrieve INTMASK register\n");
      return AB08_ERROR;
    }

    aux[0] &= ~INTMASK_AIE;

    if(ab08_write_reg((INT_MASK_ADDR + CONFIG_MAP_OFFSET),
                      &aux[0], 1) == AB08_ERROR) {
      PRINTF("RTC: failed to clear the alarm config\n");
      return AB08_ERROR;
    }
    return AB08_SUCCESS;
  }

  if((data == NULL) || (ab08_check_td_format(data, 1) == AB08_ERROR)) {
    PRINTF("RTC: invalid alarm values\n");
    return AB08_ERROR;
  }

  if((state >= RTCC_ALARM_MAX) || (repeat >= RTCC_REPEAT_100THS)) {
    PRINTF("RTC: invalid alarm config type or state\n");
    return AB08_ERROR;
  }

  /* Stop the RTCC */
  ab08_ctrl1_config(RTCC_CMD_STOP);

  buf[WEEKDAYS_ALARM_ADDR] = dec_to_bcd(data->weekdays);
  buf[MONTHS_ALARM_ADDR] = dec_to_bcd(data->months);
  buf[DAY_ALARMS_ADDR] = dec_to_bcd(data->day);
  buf[HOURS_ALARM_ADDR] = dec_to_bcd(data->hours);
  buf[MINUTES_ALARM_ADDR] = dec_to_bcd(data->minutes);
  buf[SECONDS_ALARM_ADDR] = dec_to_bcd(data->seconds);
  buf[HUNDREDTHS_ALARM_ADDR] = dec_to_bcd(data->miliseconds);

  /* Check if the 12h/24h match the current configuration */
  if(ab08_read_reg((CTRL_1_ADDR + CONFIG_MAP_OFFSET),
                   &aux[0], 1) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve CONTROL1 register\n");
    return AB08_ERROR;
  }

  if(((aux[0] & CTRL1_1224) && (data->mode == RTCC_24H_MODE)) ||
     (!(aux[0] & CTRL1_1224) && ((data->mode == RTCC_12H_MODE_AM) ||
                                 (data->mode == RTCC_12H_MODE_PM)))) {
    PRINTF("RTC: 12/24h mode and present date config mismatch\n");
    return AB08_ERROR;
  }

  if(data->mode != RTCC_24H_MODE) {
    if((data->hours == 0) || (data->hours > 12)) {
      PRINTF("RTC: Invalid hour configuration (12h mode selected)\n");
      return AB08_ERROR;
    }

    /* Toggle the PM bit */
    if(data->mode == RTCC_12H_MODE_PM) {
      buf[HOURS_ALARM_ADDR] |= RTCC_TOGGLE_PM_BIT;
    }
  }

  /* Clear the RPT field */
  if(ab08_read_reg((TIMER_CONTROL_ADDR + CONFIG_MAP_OFFSET),
                   &aux[0], 1) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve TIMER CTRL register\n");
    return AB08_ERROR;
  }

  aux[0] &= ~COUNTDOWN_TIMER_RPT_SECOND;

  /* AB08XX application manual, table 76 */
  if(repeat == RTCC_REPEAT_10THS) {
    buf[HUNDREDTHS_ALARM_ADDR] |= RTCC_FIX_10THS_HUNDRETHS;
    repeat = RTCC_REPEAT_SECOND;
  } else if(repeat == RTCC_REPEAT_100THS) {
    buf[HUNDREDTHS_ALARM_ADDR] |= RTCC_FIX_100THS_HUNDRETHS;
    repeat = RTCC_REPEAT_SECOND;
  }

  if(repeat != RTCC_REPEAT_NONE) {
    aux[0] |= (repeat << COUNTDOWN_TIMER_RPT_SHIFT);
  }

  /* We are using as default the level interrupt instead of pulses */
  /* FIXME: make this selectable */
  aux[0] |= COUNTDOWN_TIMER_TM;
  aux[0] &= ~COUNTDOWN_TIMER_TRPT;

  if(ab08_write_reg((TIMER_CONTROL_ADDR + CONFIG_MAP_OFFSET),
                    &aux[0], 1) == AB08_ERROR) {
    PRINTF("RTC: failed to clear the alarm config\n");
    return AB08_ERROR;
  }

  if(ab08_read_reg((STATUS_ADDR + CONFIG_MAP_OFFSET),
                   aux, 4) == AB08_ERROR) {
    PRINTF("RTC: failed to read configuration registers\n");
    return AB08_ERROR;
  }

  /* Clear ALM field if any */
  aux[STATUS_ADDR] &= ~STATUS_ALM;

#if RTCC_CLEAR_INT_MANUALLY
  aux[CTRL_1_ADDR] &= ~CTRL1_ARST;
#endif

  /* Clear the AIE alarm bit */
  aux[INT_MASK_ADDR] &= ~INTMASK_AIE;

  /* Configure Interrupt parameters for Alarm Interrupt Mode in nIRQ
   * or nAIRQ pins and fixed level until interrupt flag is cleared
   * RTC_INT1 is connected to the CC2538
   * RTC_INT2 is connected to the power management PIC in revision B
   */
  if (trigger == RTCC_TRIGGER_INT2) { 
    aux[CTRL_2_ADDR] |= CTRL2_OUT2S_NAIRQ_OUTB;
  /* Only options left enable the INT1 interrupt pin */
  } else {
    GPIO_ENABLE_INTERRUPT(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);
    ioc_set_over(RTC_INT1_PORT, RTC_INT1_PIN, IOC_OVERRIDE_PUE);
    nvic_interrupt_enable(RTC_INT1_VECTOR);
  }

  if (trigger == RTCC_TRIGGER_INT1) {
    aux[CTRL_2_ADDR] |= CTRL2_OUT1S_NIRQ_NAIRQ_OUT;
  } else if (trigger == RTCC_TRIGGER_BOTH) {
    aux[CTRL_2_ADDR] |= (CTRL2_OUT1S_NIRQ_NAIRQ_OUT + CTRL2_OUT2S_NAIRQ_OUTB);
  }

  if(repeat != RTCC_REPEAT_NONE) {
    aux[INT_MASK_ADDR] &= ~INTMASK_IM_LOW;
  } else {
    aux[INT_MASK_ADDR] |= INTMASK_IM_LOW;
  }

  if(ab08_write_reg((STATUS_ADDR + CONFIG_MAP_OFFSET), aux, 4) == AB08_ERROR) {
    PRINTF("RTC: failed to clear alarm config\n");
    return AB08_ERROR;
  }

  /* Write to the alarm counters */
  if(ab08_write_reg((HUNDREDTHS_ALARM_ADDR + ALARM_MAP_OFFSET), buf,
                    RTCC_ALARM_MAP_SIZE) == AB08_ERROR) {
    PRINTF("RTC: failed to set the alarm\n");
    return AB08_ERROR;
  }

  /* And finally enable the AIE bit */
  aux[INT_MASK_ADDR] |= INTMASK_AIE;
  if(ab08_write_reg((INT_MASK_ADDR + CONFIG_MAP_OFFSET),
                    &aux[INT_MASK_ADDR], 1) == AB08_ERROR) {
    PRINTF("RTC: failed to enable the alarm\n");
    return AB08_ERROR;
  }

  /* Enable back the RTCC */
  ab08_ctrl1_config(RTCC_CMD_ENABLE);

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_date_increment_seconds(simple_td_map *data, uint16_t seconds)
{
  uint16_t aux;

  if(data == NULL) {
    PRINTF("RTC: invalid argument\n");
    return AB08_ERROR;
  }

  if(rtcc_get_time_date(data) == AB08_ERROR) {
    return AB08_ERROR;
  }

  /* Nothing to do here but congratulate the user */
  if(!seconds) {
    return AB08_SUCCESS;
  }

  aux = data->seconds + seconds;
  data->seconds = (uint8_t)(aux % 60);

  /* Add the remainder seconds to the minutes counter */
  if(aux > 59) {
    aux /= 60;
    aux += data->minutes;
    data->minutes = (uint8_t)(aux % 60);
  }

  /* Add the remainder minutes to the hours counter */
  if(aux > 59) {
    aux /= 60;
    aux += data->hours;
    data->hours = (uint8_t)(aux % 24);
  }

  if(aux > 23) {
    aux /= 24;
    aux += data->day;

    if(data->months == 2) {
      if(check_leap_year(data->years)) {
        if(aux > 29) {
          data->day = (uint8_t)(aux % 29);
          data->months++;
        }
      } else if(aux > 28) {
        data->day = (uint8_t)(aux % 28);
        data->months++;
      }
    } else if((data->months == 4) || (data->months == 6) ||
             (data->months == 9) || (data->months == 11)) {
      if(aux > 30) {
        data->day = (uint8_t)(aux % 30);
        data->months++;
      }
    } else if(aux > 31) {
      data->day = (uint8_t)(aux % 31);
      data->months++;
    }
  }

  if(data->months > 12) {
    data->months = data->months % 12;
    data->years++;
  }
  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS(rtcc_int_process, "RTCC interruption process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rtcc_int_process, ev, data)
{
  static uint8_t buf;
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    if(ab08_read_status(&buf) == AB08_ERROR) {
      PRINTF("RTC: failed to retrieve ARST value\n");
      PROCESS_EXIT();
    }

    /* We only handle the AIE (alarm interrupt) only */
    if((buf & STATUS_ALM) && (rtcc_int1_callback != NULL)) {
#if RTCC_CLEAR_INT_MANUALLY
      buf &= ~STATUS_ALM;
      if(ab08_write_reg((STATUS_ADDR + CONFIG_MAP_OFFSET),
                        &buf, 1) == AB08_ERROR) {
        PRINTF("RTC: failed to clear the alarm\n");
        return AB08_ERROR;
      }
#endif
      rtcc_int1_callback(0);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_print(uint8_t value)
{
  uint8_t i, len, reg;
  char **name;
  uint8_t rtc_buffer[RTCC_CONFIG_MAP_SIZE];

  if(value >= RTCC_PRINT_MAX) {
    return AB08_ERROR;
  }

  switch(value) {
  case RTCC_PRINT_CONFIG:
    len = (RTCC_CONFIG_MAP_SIZE - 1);
    reg = STATUS_ADDR + CONFIG_MAP_OFFSET;
    name = (char **)ab080x_config_register_name;
    break;
  case RTCC_PRINT_ALARM:
  case RTCC_PRINT_ALARM_DEC:
    len = RTCC_ALARM_MAP_SIZE;
    reg = HUNDREDTHS_ALARM_ADDR + ALARM_MAP_OFFSET;
    name = (char **)ab080x_td_register_name;
    break;
  case RTCC_PRINT_DATE:
  case RTCC_PRINT_DATE_DEC:
    len = RTCC_TD_MAP_SIZE;
    reg = CENTHS_ADDR;
    name = (char **)ab080x_td_register_name;
    break;
  default:
    return AB08_ERROR;
  }

  if(ab08_read_reg(reg, rtc_buffer, len) == AB08_ERROR) {
    PRINTF("RTC: failed to retrieve values to print\n");
    return AB08_ERROR;
  }

  if(value == RTCC_PRINT_ALARM_DEC) {
    printf("%02u/%02u (%02u) %02u:%02u:%02u/%02u\n",
           bcd_to_dec(rtc_buffer[MONTHS_ALARM_ADDR]),
           bcd_to_dec(rtc_buffer[DAY_ALARMS_ADDR]),
           bcd_to_dec(rtc_buffer[WEEKDAYS_ALARM_ADDR]),
           bcd_to_dec(rtc_buffer[HOURS_ALARM_ADDR]),
           bcd_to_dec(rtc_buffer[MINUTES_ALARM_ADDR]),
           bcd_to_dec(rtc_buffer[SECONDS_ALARM_ADDR]),
           bcd_to_dec(rtc_buffer[HUNDREDTHS_ALARM_ADDR]));
    return AB08_SUCCESS;
  }

  if(value == RTCC_PRINT_DATE_DEC) {
    printf("%02u/%02u/%02u (%02u) %02u:%02u:%02u/%02u\n",
           bcd_to_dec(rtc_buffer[YEAR_ADDR]),
           bcd_to_dec(rtc_buffer[MONTHS_ADDR]),
           bcd_to_dec(rtc_buffer[DAY_ADDR]),
           bcd_to_dec(rtc_buffer[WEEKDAYLS_ADDR]),
           bcd_to_dec(rtc_buffer[HOUR_ADDR]),
           bcd_to_dec(rtc_buffer[MIN_ADDR]),
           bcd_to_dec(rtc_buffer[SEC_ADDR]),
           bcd_to_dec(rtc_buffer[CENTHS_ADDR]));
    return AB08_SUCCESS;
  }

  for(i = 0; i < len; i++) {
    printf("0x%02X <- %s\n", rtc_buffer[i], name[i]);
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static void
rtcc_interrupt_handler(uint8_t port, uint8_t pin)
{
  process_poll(&rtcc_int_process);
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_set_autocalibration(uint8_t period)
{
  uint8_t aux;

  if(period > RTCC_AUTOCAL_9_MIN) {
    PRINTF("RTC: invalid autocal value\n");
    return AB08_ERROR;
  }

  if(ab08_read_reg((OSC_CONTROL_ADDR + CONFIG_MAP_OFFSET),
                   &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to read oscillator registers\n");
    return AB08_ERROR;
  }

  /* Clear ACAL */
  aux &= ~OSCONTROL_ACAL_9_MIN;

  /* Unlock the key register */
  ab08_key_reg(RTCC_CONFKEY_OSCONTROL);

  switch(period) {
  case RTCC_AUTOCAL_DISABLE:
    break;
  case RTCC_AUTOCAL_ONCE:
  case RTCC_AUTOCAL_17_MIN:
    aux |= OSCONTROL_ACAL_17_MIN;
    break;
  case RTCC_AUTOCAL_9_MIN:
    aux |= OSCONTROL_ACAL_9_MIN;
    break;
  default:
    return AB08_ERROR;
  }

  if(ab08_write_reg((OSC_CONTROL_ADDR + CONFIG_MAP_OFFSET),
                    &aux, 1) == AB08_ERROR) {
    PRINTF("RTC: failed to clear the autocalibration\n");
    return AB08_ERROR;
  }

  if(period == RTCC_AUTOCAL_ONCE) {
    clock_delay_usec(10000);
    ab08_key_reg(RTCC_CONFKEY_OSCONTROL);
    aux &= ~OSCONTROL_ACAL_9_MIN;
    if(ab08_write_reg((OSC_CONTROL_ADDR + CONFIG_MAP_OFFSET),
                      &aux, 1) == AB08_ERROR) {
      PRINTF("RTC: failed to clear the autocalibration\n");
      return AB08_ERROR;
    }
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_set_calibration(uint8_t mode, int32_t adjust)
{
  int32_t adjint;
  uint8_t adjreg[2];
  uint8_t xtcal;

  if(mode > RTCC_CAL_RC_OSC) {
    PRINTF("RTC: invalid calibration mode\n");
    return AB08_ERROR;
  }

  /* Fixed values dependant on the oscillator source (Application Manual) */
  if((mode == RTCC_CAL_XT_OSC) && ((adjust <= -610) || (adjust >= 242))) {
    PRINTF("RTC: invalid adjust value for XT oscillator\n");
    return AB08_ERROR;
  }

  if((mode == RTCC_CAL_RC_OSC) && ((adjust <= -65536) || (adjust >= 65520))) {
    PRINTF("RTC: invalid adjust value for XT oscillator\n");
    return AB08_ERROR;
  }

  /* Calibration routine taken from the Application manual */
  if(adjust < 0) {
    adjint = ((adjust) * 1000 - 953);
  } else {
    adjint = ((adjust) * 1000 + 953);
  }

  adjint = adjint / 1907;

  if(mode == RTCC_CAL_XT_OSC) {
    if(adjint > 63) {
      xtcal = 0;
      /* CMDX = 1 */
      adjreg[0] = ((adjint >> 1) & 0x3F) | 0x80;
    } else if(adjint > -65) {
      xtcal = 0;
      adjreg[0] = (adjint & 0x7F);
    } else if(adjint > -129) {
      xtcal = 1;
      adjreg[0] = ((adjint + 64) & 0x7F);
    } else if(adjint > -193) {
      xtcal = 2;
      adjreg[0] = ((adjint + 128) & 0x7F);
    } else if(adjint > -257) {
      xtcal = 3;
      adjreg[0] = ((adjint + 192) & 0x7F);
    } else {
      xtcal = 3;
      adjreg[0] = ((adjint + 192) >> 1) & 0xFF;
    }

    if(ab08_write_reg((CAL_XT_ADDR + CONFIG_MAP_OFFSET),
                      &adjreg[0], 1) == AB08_ERROR) {
      PRINTF("RTC: failed to clear the autocalibration\n");
      return AB08_ERROR;
    }

    if(ab08_read_reg((OSC_STATUS_ADDR + CONFIG_MAP_OFFSET),
                     &adjreg[0], 1) == AB08_ERROR) {
      PRINTF("RTC: failed to read oscillator registers\n");
      return AB08_ERROR;
    }

    /* Clear XTCAL and write new value */
    adjreg[0] &= 0x3F;
    adjreg[0] |= (xtcal << 6);

    if(ab08_write_reg((OSC_STATUS_ADDR + CONFIG_MAP_OFFSET),
                      &adjreg[0], 1) == AB08_ERROR) {
      PRINTF("RTC: failed to clear the autocalibration\n");
      return AB08_ERROR;
    }
  } else if(mode == RTCC_CAL_RC_OSC) {
    if(adjint > 32767) {
      adjreg[1] = ((adjint >> 3) & 0xFF);
      adjreg[0] = ((adjint >> 11) | 0xC0);
    } else if(adjint > 16383) {
      adjreg[1] = ((adjint >> 2) & 0xFF);
      adjreg[0] = ((adjint >> 10) | 0x80);
    } else if(adjint > 8191) {
      adjreg[1] = ((adjint >> 1) & 0xFF);
      adjreg[0] = ((adjint >> 9) | 0x40);
    } else if(adjint >= 0) {
      adjreg[1] = ((adjint) & 0xFF);
      adjreg[0] = (adjint >> 8);
    } else if(adjint > -8193) {
      adjreg[1] = ((adjint) & 0xFF);
      adjreg[0] = (adjint >> 8) & 0x3F;
    } else if(adjint > -16385) {
      adjreg[1] = ((adjint >> 1) & 0xFF);
      adjreg[0] = (adjint >> 9) & 0x7F;
    } else if(adjint > -32769) {
      adjreg[1] = ((adjint >> 2) & 0xFF);
      adjreg[0] = (adjint >> 10) & 0xBF;
    } else {
      adjreg[1] = ((adjint >> 3) & 0xFF);
      adjreg[0] = (adjint >> 11) & 0xFF;
    }

    if(ab08_write_reg((CAL_RC_HI_ADDR + CONFIG_MAP_OFFSET),
                      adjreg, 2) == AB08_ERROR) {
      PRINTF("RTC: failed to set the RC calibration\n");
      return AB08_ERROR;
    }

    /* This should not happen */
  } else {
    return AB08_ERROR;
  }

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
rtcc_init(void)
{
  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
           I2C_SCL_NORMAL_BUS_SPEED);

#if RTCC_SET_DEFAULT_CONFIG
  write_default_config();
#endif

#if RTCC_SET_AUTOCAL
  rtcc_set_autocalibration(RTCC_AUTOCAL_17_MIN);
#endif

  /* Initialize interrupts handlers */
  rtcc_int1_callback = NULL;

  /* Configure the interrupts pins */
  GPIO_SOFTWARE_CONTROL(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);
  GPIO_SET_INPUT(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);

  /* Pull-up resistor, detect falling edge */
  GPIO_DETECT_EDGE(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);
  GPIO_TRIGGER_SINGLE_EDGE(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);
  GPIO_DETECT_FALLING(RTC_INT1_PORT_BASE, RTC_INT1_PIN_MASK);
  gpio_register_callback(rtcc_interrupt_handler, RTC_INT1_PORT, RTC_INT1_PIN);

  /* Spin process until an interrupt is received */
  process_start(&rtcc_int_process, NULL);

  return AB08_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

