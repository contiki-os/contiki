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
#ifdef GATT_SENSORS_BATTERY
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "../ble-att.h"
#include "battery.h"
#include "board-peripherals.h"
#include "batmon-sensor.h"
#include "notify.h"
#include SOURCE_LL_DISCONNECT_EVENT
/* process for temp notification */
PROCESS(battery_notify_process, "battery_notify_process");
/* process callback on disconnect event */
PROCESS(battery_disconnect_process, "Disconnect battery notify");

static uint16_t handle_to_notify;
static bt_size_t previous_value;
static uint32_t period_notify;

/*---------------------------------------------------------------------------*/
uint8_t
get_value_battery(bt_size_t *database)
{
  int value;
  uint16_t tmp;

  value = GATT_SENSORS_BATTERY.value(BATMON_SENSOR_TYPE_TEMP);
  PRINTF("value : 0x%X\n", value);
  if(value != 0) {
    PRINTF("Bat: Temp=%d C\n", value);
  } else {
    PRINTF("BAT: Temp Read Error\n");
    return ATT_ECODE_SENSOR_READINGS;
  }
  /* let space for Voltage value */
  value = value << 16;

  tmp = GATT_SENSORS_BATTERY.value(BATMON_SENSOR_TYPE_VOLT);
  if(tmp != 0) {
    PRINTF("Bat: Volt=%d mV\n", (tmp * 125) >> 5);
    value += ((tmp * 125) >> 5);
  } else {
    PRINTF("BAT: Voltage Read Error\n");
    return ATT_ECODE_SENSOR_READINGS;
  }
  database->type = BT_SIZE32;
  database->value.u32 = (uint32_t)value;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_battery_sensor(const bt_size_t *new_value)
{
  switch(new_value->value.u8) {
  case 1:
    PRINTF("ACTIVATION CAPTEUR\n");
    SENSORS_ACTIVATE(GATT_SENSORS_BATTERY);
    break;
  case 0:
    PRINTF("DESACTIVATION CAPTEUR");
    SENSORS_DEACTIVATE(GATT_SENSORS_BATTERY);
    break;
  default:
    return ATT_ECODE_BAD_NUMBER;
  }
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_battery_sensor(bt_size_t *database)
{
  database->type = BT_SIZE8;
  database->value.u8 = (uint8_t)GATT_SENSORS_BATTERY.status(SENSORS_ACTIVE);
  PRINTF("status temp sensor : 0x%X\n", GATT_SENSORS_BATTERY.status(SENSORS_ACTIVE));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_battery_notify(bt_size_t *status_value)
{
  status_value->type = BT_SIZE16;
  if(process_is_running(&battery_notify_process) == 0) {
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
  PRINTF("ACTIVATION battery NOTIFICATIONS\n");
  handle_to_notify = g_current_att->att_value.value.u16;
  process_start(&battery_notify_process, NULL);
  process_start(&battery_disconnect_process, NULL);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_notification()
{
  PRINTF("DESACTIVATION battery NOTIFICATIONS\n");
  process_exit(&battery_notify_process);
  process_exit(&battery_disconnect_process);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_battery_notify(const bt_size_t *new_value)
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
set_period_battery(const bt_size_t *new_period)
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
get_period_battery(bt_size_t *period_to_send)
{
  /* convert period in seconds */
  period_to_send->value.u32 = (period_notify) / CLOCK_SECOND;
  period_to_send->type = BT_SIZE32;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(battery_notify_process, ev, data)
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

    error = get_value_battery(&sensor_value);
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
PROCESS_THREAD(battery_disconnect_process, ev, data){

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == ll_disconnect_event);
    disable_notification();
  }
  PROCESS_END();
}
#endif
