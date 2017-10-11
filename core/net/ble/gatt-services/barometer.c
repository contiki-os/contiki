/*
 * Copyright (c) 2017, Arthur Courtel
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
 *
 * Author: Arthur Courtel <arthurcourtel@gmail.com>
 *
 */
/*---------------------------------------------------------------------------*/
#include "gatt_config.h"
#ifdef GATT_SENSORS_BAROMETER
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "../ble-att.h"
#include "barometer.h"
#include "sensortag/bmp-280-sensor.h"
#include "notify.h"
#include SOURCE_LL_DISCONNECT_EVENT
/* process for temp notification */
PROCESS(barometer_notify_process, "temp_barometer_process");
/* process callback on disconnect event */
PROCESS(on_disconnect_barometer_process, "Disconnect barometer notify");

static uint16_t handle_to_notify;
static bt_size_t previous_value;
static uint32_t period_notify;
static uint8_t g_sensor_activated;
static inline void enable_sensor();
static inline void disable_sensor();
/*---------------------------------------------------------------------------*/
uint8_t
get_value_barometer_pressure(bt_size_t *database)
{
  uint32_t value;
  if(g_sensor_activated) {
    value = GATT_SENSORS_BAROMETER.value(BMP_280_SENSOR_TYPE_PRESS);

    if(value != CC26XX_SENSOR_READING_ERROR) {
      PRINTF("BAR: Pressure=%ld.%02ld hPa\n", value / 100, value % 100);
    } else {
      PRINTF("BAR: Pressure Read Error\n");
      return ATT_ECODE_SENSOR_READINGS;
    }

    database->type = BT_SIZE32;
    database->value.u32 = value;
    /* I need to disable/enable the sensor otherwise it doesn't work */
    disable_sensor();
    enable_sensor();

    return SUCCESS;
  }
  return ATT_ECODE_SENSOR_READINGS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_value_barometer_temp(bt_size_t *database)
{
  uint16_t temp;
  if(g_sensor_activated) {
    temp = GATT_SENSORS_BAROMETER.value(BMP_280_SENSOR_TYPE_TEMP);

    if(temp != CC26XX_SENSOR_READING_ERROR) {
      PRINTF("BAR: Temp=%d.%02d C\n", temp / 100, temp % 100);
    } else {
      PRINTF("BAR: Temperature Read Error\n");
      return ATT_ECODE_SENSOR_READINGS;
    }
    database->type = BT_SIZE16;
    database->value.u16 = temp;

    /* I need to disable/enable the sensor otherwise it doesn't work */
    disable_sensor();
    enable_sensor();

    return SUCCESS;
  }
  return ATT_ECODE_SENSOR_READINGS;
}
/*---------------------------------------------------------------------------*/
static inline void
enable_sensor()
{
  SENSORS_ACTIVATE(GATT_SENSORS_BAROMETER);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_sensor()
{
  SENSORS_DEACTIVATE(GATT_SENSORS_BAROMETER);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_barometer_sensor(const bt_size_t *new_value)
{
  switch(new_value->value.u8) {
  case 1:
    PRINTF("SENSOR ACTIVATION\n");
    enable_sensor();
    g_sensor_activated = 1;
    break;
  case 0:
    PRINTF("SENSOR DEACTIVATION\n");
    disable_sensor();
    g_sensor_activated = 0;
    break;
  default:
    return ATT_ECODE_BAD_NUMBER;
  }
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_barometer_sensor(bt_size_t *database)
{
  database->type = BT_SIZE8;
  database->value.u8 = (uint8_t)GATT_SENSORS_BAROMETER.status(SENSORS_ACTIVE);
  PRINTF("status temp barometer : 0x%X\n", GATT_SENSORS_BAROMETER.status(SENSORS_ACTIVE));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_barometer_notify(bt_size_t *status_value)
{
  status_value->type = BT_SIZE16;
  if(process_is_running(&barometer_notify_process) == 0) {
    status_value->value.u16 = 0;
  } else {
    status_value->value.u16 = 1;
  }

  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
static inline void
enable_notification()
{
  PRINTF("ACTIVATION barometer NOTIFICATIONS\n");
  handle_to_notify = g_current_att->att_handle - HANDLE_SPACE_TO_DATA_ATTRIBUTE;
  process_start(&barometer_notify_process, NULL);
  process_start(&on_disconnect_barometer_process, NULL);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_notification()
{
  PRINTF("DEACTIVATION barometer NOTIFICATIONS\n");
  process_exit(&barometer_notify_process);
  process_exit(&on_disconnect_barometer_process);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_barometer_notify(const bt_size_t *new_value)
{
  uint8_t error;
  error = SUCCESS;
  switch(new_value->value.u8) {
  case 1:
    enable_notification();
    break;
  case 0:
    disable_notification();
    break;
  default:
    return ATT_ECODE_BAD_NUMBER;   /*ERROR */
  }

  if(error != SUCCESS) {
    return error;
  }

  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
set_period_barometer(const bt_size_t *new_period)
{
  /* convert period received in system seconds */
  period_notify = (swap32(new_period->value.u32)) * CLOCK_SECOND;
  /* period mini = CLOCK_SECOND */
  if(period_notify < (uint32_t)CLOCK_SECOND) {
    period_notify = (uint32_t)CLOCK_SECOND;
  }

  PRINTF("new period : %lX\n", period_notify);
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_period_barometer(bt_size_t *period_to_send)
{
  /* convert period in seconds */
  period_to_send->value.u32 = (period_notify) / CLOCK_SECOND;
  period_to_send->type = BT_SIZE32;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(barometer_notify_process, ev, data)
{
  static struct etimer notify_timer;
  bt_size_t sensor_value;
  uint8_t error;

  PROCESS_BEGIN();
  /* initiate notify period to CLOCK_SECOND */
  period_notify = CLOCK_SECOND;
  etimer_set(&notify_timer, (clock_time_t)period_notify);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&notify_timer));
    /* update notification period with possible new period */
    etimer_reset_with_new_interval(&notify_timer, (clock_time_t)period_notify);

    error = get_value_barometer_pressure(&sensor_value);
    if(is_values_equals(&sensor_value, &previous_value) != 0) {
      if(error != SUCCESS) {
        prepare_error_resp_notif(handle_to_notify, error);
        /* If error, disable notifications */
        disable_notification();
      } else {
        prepare_notification(handle_to_notify, &sensor_value);
        previous_value = sensor_value;
      }

      send_notify();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Disable notifications when disconnection event show up */
PROCESS_THREAD(on_disconnect_barometer_process, ev, data){

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == ll_disconnect_event);
    disable_notification();
  }
  PROCESS_END();
}
#endif
