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
#ifdef GATT_SENSORS_TEMP1
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "../ble-att.h"
#include "temp.h"
#include "sensortag/tmp-007-sensor.h"
#include "notify.h"
#include SOURCE_LL_DISCONNECT_EVENT
/* process for temp notification */
PROCESS(temp_notify_process, "temp_notify_process");
/* process callback on disconnect event */
PROCESS(temp_disconnect_process, "temp_disconnect_process");

static uint16_t handle_to_notify;
static bt_size_t previous_value;
static uint32_t period_notify;

/*---------------------------------------------------------------------------*/
uint8_t
get_value_temp(bt_size_t *value)
{
  int temp, tobj;
  temp = GATT_SENSORS_TEMP1.value(TMP_007_SENSOR_TYPE_ALL);

  if(temp == CC26XX_SENSOR_READING_ERROR) {
    return ATT_ECODE_SENSOR_READINGS; /*ERROR */
  }
  temp = GATT_SENSORS_TEMP1.value(TMP_007_SENSOR_TYPE_AMBIENT);
  PRINTF("TEMP Ambiant: %02X\n", temp);

  temp = temp << 16;

  tobj = GATT_SENSORS_TEMP1.value(TMP_007_SENSOR_TYPE_OBJECT);
  PRINTF("TEMP Ambiant: %02X\n", tobj);
  temp += tobj;
  value->type = BT_SIZE32;
  value->value.u32 = temp;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_temp_sensor(const bt_size_t *new_value)
{
  switch(new_value->value.u8) {
  case 1:
    PRINTF("ACTIVATION CAPTEUR\n");
    SENSORS_ACTIVATE(GATT_SENSORS_TEMP1);
    break;
  case 0:
    PRINTF("DESACTIVATION CAPTEUR");
    SENSORS_DEACTIVATE(GATT_SENSORS_TEMP1);
    break;
  default:
    return ATT_ECODE_BAD_NUMBER;
  }
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_temp_sensor(bt_size_t *database)
{
  database->type = BT_SIZE8;
  database->value.u8 = (uint8_t)GATT_SENSORS_TEMP1.status(SENSORS_ACTIVE);
  PRINTF("status temp sensor : 0x%X\n", GATT_SENSORS_TEMP1.status(SENSORS_ACTIVE));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_temp_notify(bt_size_t *status_value)
{
  status_value->type = BT_SIZE16;
  if(process_is_running(&temp_notify_process) == 0) {
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
  PRINTF("ACTIVATION TEMP NOTIFICATIONS\n");
  handle_to_notify = g_current_att->att_value.value.u16;
  process_start(&temp_notify_process, NULL);
  process_start(&temp_disconnect_process, NULL);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_notification()
{
  PRINTF("DESACTIVATION TEMP NOTIFICATIONS\n");
  process_exit(&temp_notify_process);
  process_exit(&temp_disconnect_process);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_temp_notify(const bt_size_t *new_value)
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
set_period_temp(const bt_size_t *new_period)
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
get_period_temp(bt_size_t *period_to_send)
{
  /* convert period in seconds */
  period_to_send->value.u32 = (period_notify) / CLOCK_SECOND;
  period_to_send->type = BT_SIZE32;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp_notify_process, ev, data)
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

    error = get_value_temp(&sensor_value);
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
PROCESS_THREAD(temp_disconnect_process, ev, data){

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == ll_disconnect_event);
    disable_notification();
  }
  PROCESS_END();
}
#endif
