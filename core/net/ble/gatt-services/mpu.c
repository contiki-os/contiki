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
#ifdef GATT_SENSORS_MPU
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "../ble-att.h"
#include "mpu.h"
#include "sensortag/mpu-9250-sensor.h"
#include "notify.h"
#include SOURCE_LL_DISCONNECT_EVENT
/* process for temp notification */
PROCESS(mpu_notify_process, "mpu_notify_process");
/* process callback on disconnect event */
PROCESS(mpu_disconnect_process, "Disconnect mpu notify");

static uint16_t handle_to_notify;
static bt_size_t previous_value;
static uint32_t period_notify;

/*---------------------------------------------------------------------------*/
uint8_t
get_value_mpu(bt_size_t *database)
{
  int value;
  uint8_t iterator;

  iterator = 0;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_GYRO_X);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_ACC_X);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_ACC_Y);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  value = GATT_SENSORS_MPU.value(MPU_9250_SENSOR_TYPE_ACC_Z);
  PRINTF("value : 0x%X\n", value);
  database->value.u128.data[iterator++] = value & 0x00ff;
  database->value.u128.data[iterator++] = (value & 0xff00) >> 8;

  database->type = BT_SIZEMPU;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_mpu_sensor(const bt_size_t *new_value)
{
  switch(new_value->value.u8) {
  case 1:
    PRINTF("SENSOR ACTIVATION\n");
    GATT_SENSORS_MPU.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
    break;
  case 0:
    PRINTF("SENSOR DEACTIVATION\n");
    SENSORS_DEACTIVATE(GATT_SENSORS_MPU);
    break;
  default:
    return ATT_ECODE_BAD_NUMBER;
  }
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_mpu_sensor(bt_size_t *database)
{
  database->type = BT_SIZE8;
  database->value.u8 = (uint8_t)GATT_SENSORS_MPU.status(SENSORS_ACTIVE);
  PRINTF("status mpu sensor : 0x%X\n", GATT_SENSORS_MPU.status(SENSORS_ACTIVE));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_status_mpu_notify(bt_size_t *status_value)
{
  status_value->type = BT_SIZE16;
  if(process_is_running(&mpu_notify_process) == 0) {
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
  PRINTF("ACTIVATION mpu NOTIFICATIONS\n");
  handle_to_notify = g_current_att->att_handle - HANDLE_SPACE_TO_DATA_ATTRIBUTE;
  process_start(&mpu_notify_process, NULL);
  process_start(&mpu_disconnect_process, NULL);
}
/*---------------------------------------------------------------------------*/
static inline void
disable_notification()
{
  PRINTF("DEACTIVATION mpu NOTIFICATIONS\n");
  process_exit(&mpu_notify_process);
  process_exit(&mpu_disconnect_process);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_status_mpu_notify(const bt_size_t *new_value)
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
set_period_mpu(const bt_size_t *new_period)
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
get_period_mpu(bt_size_t *period_to_send)
{
  /* convert period in seconds */
  period_to_send->value.u32 = (period_notify) / CLOCK_SECOND;
  period_to_send->type = BT_SIZE32;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mpu_notify_process, ev, data)
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

    error = get_value_mpu(&sensor_value);
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
PROCESS_THREAD(mpu_disconnect_process, ev, data){

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == ll_disconnect_event);
    disable_notification();
  }
  PROCESS_END();
}
#endif
