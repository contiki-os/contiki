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
#ifdef GATT_SENSORS_HUM
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "../ble-att.h"
#include "humidity.h"
#include "sensortag/hdc-1000-sensor.h"
#include "notify.h"
#include SOURCE_LL_DISCONNECT_EVENT
/* process for temp notification */
PROCESS(humidity_notify_process, "temp_humidity_process");
/* process callback on disconnect event */
PROCESS(humidity_disconnect_process, "Disconnect humidity notify");

static uint16_t handle_to_notify;
static bt_size_t previous_value;
static uint32_t period_notify;
static uint8_t g_sensor_activated;
static inline void enable_sensor();
static inline void disable_sensor();

/*---------------------------------------------------------------------------*/
uint8_t
get_value_humidity_hum(bt_size_t *database)
{
  uint16_t hum;

  if(g_sensor_activated) {
    hum = GATT_SENSORS_HUM.value(HDC_1000_SENSOR_TYPE_HUMIDITY);
    if(hum != CC26XX_SENSOR_READING_ERROR) {
      PRINTF("HDC: Humidity=%d.%02d %%RH\n", hum / 100, hum % 100);
    } else {
      PRINTF("HDC: Humidity Read Error\n");
      return ATT_ECODE_SENSOR_READINGS;
    }
    database->type = BT_SIZE32;
    database->value.u16 = (uint16_t)hum;
    /* I need to disable/enable the sensor otherwise it doesn't work */
    disable_sensor();
    enable_sensor();

    return SUCCESS;
  }
  return ATT_ECODE_SENSOR_READINGS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_value_humidity_temp(bt_size_t *database)
{
  uint16_t temp;
  if(g_sensor_activated) {
    temp = GATT_SENSORS_HUM.value(HDC_1000_SENSOR_TYPE_TEMP);
    if(temp != CC26XX_SENSOR_READING_ERROR) {
      PRINTF("HDC: Temp=%d.%02d C\n", ((uint16_t)temp) / 100, ((uint16_t)temp) % 100);
    } else {
      PRINTF("HDC: Temp Read Error\n");
      return ATT_ECODE_SENSOR_READINGS;
    }

    database->type = BT_SIZE32;
    database->value.u16 = (uint16_t)temp;
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
  SENSORS_ACTIVATE(GATT_SENSORS_HUM);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_sensor()
{
  SENSORS_DEACTIVATE(GATT_SENSORS_HUM);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_humidity_sensor(const bt_size_t *new_value)
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
get_status_humidity_sensor(bt_size_t *database)
{
  database->type = BT_SIZE8;
  database->value.u8 = (uint8_t)GATT_SENSORS_HUM.status(SENSORS_ACTIVE);
  PRINTF("status humidity barometer : 0x%X\n", GATT_SENSORS_HUM.status(SENSORS_ACTIVE));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_humidity_notify(bt_size_t *status_value)
{
  status_value->type = BT_SIZE16;
  if(process_is_running(&humidity_notify_process) == 0) {
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
  PRINTF("ACTIVATION HUMIDITY NOTIFICATIONS\n");
  handle_to_notify = g_current_att->att_handle - HANDLE_SPACE_TO_DATA_ATTRIBUTE;
  process_start(&humidity_notify_process, NULL);
  process_start(&humidity_disconnect_process, NULL);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_notification()
{
  PRINTF("DEACTIVATION HUMIDITY NOTIFICATIONS\n");
  process_exit(&humidity_notify_process);
  process_exit(&humidity_disconnect_process);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_humidity_notify(const bt_size_t *new_value)
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
set_period_humidity(const bt_size_t *new_period)
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
get_period_humidity(bt_size_t *period_to_send)
{
  /* convert period in seconds */
  period_to_send->value.u32 = (period_notify) / CLOCK_SECOND;
  period_to_send->type = BT_SIZE32;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(humidity_notify_process, ev, data)
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

    error = get_value_humidity_hum(&sensor_value);
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
PROCESS_THREAD(humidity_disconnect_process, ev, data){

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == ll_disconnect_event);
    disable_notification();
  }
  PROCESS_END();
}
#endif
