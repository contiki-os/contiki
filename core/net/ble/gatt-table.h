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

#define UUID_PRIMARY_DECLARATION          { 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_CHARACTERISTIC_DECLARATION   { 0x00, 0x00, 0x28, 0x03, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_CLIENT_CHARACTERISTIC_CONFIGURATION   { 0x00, 0x00, 0x29, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

#define UUID_DEVICE_NAME                  { 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

#define UUID_CONTIKI_VERSION              { 0x00, 0x00, 0x2A, 0x28, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }


#define UUID_USER_DESC      { 0x00, 0x00, 0x29, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

#ifndef GATT_TABLE_H_
#define GATT_TABLE_H_
#include "contiki-conf.h"
#include "gatt_config.h"
#ifdef GATT_TABLE_INSERT
/* TODO : replace att_value by function called in get_action */
static const attribute_t *list_attr[] =
{
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : GENERIC ACCESS SERVICE */
    .get_action = get_generic_access_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ // CHAR DECLARATION : DEVICE NAME
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ // DEVICE NAME
    .get_action = get_device_name,
    .set_action = NULL,
    .att_value_len = BT_SIZE_STR,
    .att_uuid.data = UUID_DEVICE_NAME,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ // PRIMARY SERVICE DECLARATION : INFORMATION SERVICE
    .get_action = get_generic_information_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ // CHAR DECLARATION : CONTIKI VERSION
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CONTIKI VERSION */
    .get_action = NULL,
    .set_action = NULL,
    .att_value_len = BT_SIZE_STR,
    .att_uuid.data = UUID_CONTIKI_VERSION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#ifdef GATT_SENSORS_TEMP1
#define UUID_TEMP_SERVICE                 { 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_TEMP_DATA                    { 0x00, 0x00, 0x2A, 0x6E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_TEMP_ED                      { 0x00, 0x00, 0xAA, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_TEMP_NOTIFY_PARAM            { 0x00, 0x00, 0xAA, 0x03, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : TEMP IR */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_TEMP_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x1209006E2A),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP DATA */
    .get_action = get_value_temp,
    .set_action = NULL,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0,
    .att_uuid.data = UUID_TEMP_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Temp sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP NOTIFY */
    .get_action = get_status_temp_notify,
    .set_action = set_status_temp_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0009,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A0D0002AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP ED */
    .get_action = get_status_temp_sensor,
    .set_action = set_status_temp_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u64 = 0x0,
    .att_uuid.data = UUID_TEMP_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis temp sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A100003AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP NOTIFY PARAM */
    .get_action = get_period_temp,
    .set_action = set_period_temp,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_TEMP_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period temp notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif   /*GATT_SENSORS_TEMP1 */

#ifdef GATT_SENSORS_HUM
#define UUID_HUMIDITY_SERVICE             { 0x00, 0x00, 0xAA, 0x10, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_HUMIDITY_DATA                { 0x00, 0x00, 0xAA, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_HUMIDITY_ED                  { 0x00, 0x00, 0xAA, 0x12, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_HUMIDITY_NOTIFY_PARAM        { 0x00, 0x00, 0xAA, 0x13, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : HUMIDITY */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_HUMIDITY_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x1214006F2A),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY DATA */
    .get_action = get_value_humidity,
    .set_action = NULL,
    .att_value.type = BT_SIZE32,
    .att_value.value.u32 = 0x0,
    .att_uuid.data = UUID_HUMIDITY_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis hum sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY NOTIFY */
    .get_action = get_status_humidity_notify,
    .set_action = set_status_humidity_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0014,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A180012AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY ED */
    .get_action = get_status_humidity_sensor,
    .set_action = set_status_humidity_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_HUMIDITY_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis hum sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A1B0013AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY NOTIFY PARAM */
    .get_action = get_period_humidity,
    .set_action = set_period_humidity,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_HUMIDITY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period hum notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_BAROMETER
#define UUID_BAROMETER_SERVICE            { 0x00, 0x00, 0xAA, 0x20, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BAROMETER_DATA               { 0x00, 0x00, 0x2A, 0x6D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BAROMETER_ED                 { 0x00, 0x00, 0xAA, 0x22, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BAROMETER_NOTIFY_PARAM      { 0x00, 0x00, 0xAA, 0x23, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : BAROMETER */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_BAROMETER_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BAROMETER DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x121F006D2A),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BAROMETER DATA */
    .get_action = get_value_barometer,
    .set_action = NULL,
    .att_value.type = BT_SIZE32,
    .att_value.value.u32 = 0x0,
    .att_uuid.data = UUID_BAROMETER_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "barometer sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* barometer NOTIFY */
    .get_action = get_status_barometer_notify,
    .set_action = set_status_barometer_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x001F,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BAROMETER ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A230022AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BAROMETER ED */
    .get_action = get_status_barometer_sensor,
    .set_action = set_status_barometer_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_BAROMETER_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis barom sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : barometer PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A260023AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* barometer NOTIFY PARAM */
    .get_action = get_period_barometer,
    .set_action = set_period_barometer,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_BAROMETER_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period barom notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_LUXOMETER
#define UUID_LUXOMETER_SERVICE            { 0x00, 0x00, 0xAA, 0x30, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_LUXOMETER_DATA               { 0x00, 0x00, 0xAA, 0x31, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_LUXOMETER_ED                 { 0x00, 0x00, 0xAA, 0x32, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_LUXOMETER_NOTIFY_PARAM       { 0x00, 0x00, 0xAA, 0x33, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : LUXOMETER */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_LUXOMETER_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LUXOMETER DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x122A0031AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER DATA */
    .get_action = get_value_luxometer,
    .set_action = NULL,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0,
    .att_uuid.data = UUID_LUXOMETER_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "lux sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER NOTIFY */
    .get_action = get_status_luxometer_notify,
    .set_action = set_status_luxometer_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x002A,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LUXOMETER ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A2E0032AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER ED */
    .get_action = get_status_luxometer_sensor,
    .set_action = set_status_luxometer_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_LUXOMETER_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis lux sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : luxometer PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A310033AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* luxometer NOTIFY PARAM */
    .get_action = get_period_luxometer,
    .set_action = set_period_luxometer,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_LUXOMETER_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period lux notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_MPU
#define UUID_MPU_SERVICE                  { 0x00, 0x00, 0xAA, 0x40, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_MPU_DATA                     { 0x00, 0x00, 0xAA, 0x41, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_MPU_ED                       { 0x00, 0x00, 0xAA, 0x42, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_MPU_NOTIFY_PARAM             { 0x00, 0x00, 0xAA, 0x43, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : MPU */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_MPU_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x12350041AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU DATA */
    .get_action = get_value_mpu,
    .set_action = NULL,
    .att_value.type = BT_SIZEMPU,
    .att_uuid.data = UUID_MPU_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "mpu sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU NOTIFY */
    .get_action = get_status_mpu_notify,
    .set_action = set_status_mpu_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0035,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A390042AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU ED */
    .get_action = get_status_mpu_sensor,
    .set_action = set_status_mpu_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_MPU_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis mpu sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A3C0043AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU NOTIFY PARAM */
    .get_action = get_period_mpu,
    .set_action = set_period_mpu,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_MPU_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period mpu notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif
#ifdef GATT_LEDS
#define UUID_LED_SERVICE                  { 0x00, 0x00, 0xAA, 0x50, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_LED_DATA                     { 0x00, 0x00, 0xAA, 0x51, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_LED_ED                       { 0x00, 0x00, 0xAA, 0x52, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : LED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_LED_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LED DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A400051AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LED DATA */
    .get_action = get_status_leds,
    .set_action = set_status_leds,
    .att_value.type = BT_SIZE8,
    .att_uuid.data = UUID_LED_DATA,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "led data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_BATTERY
#define UUID_BATTERY_SERVICE              { 0x00, 0x00, 0xAA, 0x60, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BATTERY_DATA                 { 0x00, 0x00, 0xAA, 0x61, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BATTERY_ED                   { 0x00, 0x00, 0xAA, 0x62, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BATTERY_NOTIFY_PARAM         { 0x00, 0x00, 0xAA, 0x63, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : BATTERY */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_BATTERY_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BATTERY DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x12440061AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BATTERY DATA */
    .get_action = get_value_battery,
    .set_action = NULL,
    .att_value.type = BT_SIZE32,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "battery sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BATTERY NOTIFY */
    .get_action = get_status_battery_notify,
    .set_action = set_status_battery_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x0044,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BATTERY ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A480062AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BATTERY ED */
    .get_action = get_status_battery_sensor,
    .set_action = set_status_battery_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_BATTERY_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis battery sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BATTERY PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A4B0063AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BATTERY NOTIFY PARAM */
    .get_action = get_period_battery,
    .set_action = set_period_battery,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_BATTERY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period battery notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_BUTTONS
#define UUID_BUTTONS_SERVICE              { 0x00, 0x00, 0xAA, 0x70, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BUTTONS_DATA                 { 0x00, 0x00, 0xAA, 0x71, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BUTTONS_ED                   { 0x00, 0x00, 0xAA, 0x72, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_BUTTONS_NOTIFY_PARAM         { 0x00, 0x00, 0xAA, 0x73, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : BUTTONS */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_BUTTONS_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x124F0071AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS DATA */
    .get_action = get_value_buttons,
    .set_action = NULL,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_BUTTONS_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "buttons sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS NOTIFY */
    .get_action = get_status_buttons_notify,
    .set_action = set_status_buttons_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x004F,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A530072AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS ED */
    .get_action = get_status_buttons_sensor,
    .set_action = set_status_buttons_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_BUTTONS_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis buttons sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A560073AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS NOTIFY PARAM */
    .get_action = get_period_buttons,
    .set_action = set_period_buttons,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_BUTTONS_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period buttons notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_REED_RELAY
#define UUID_REED_RELAY_SERVICE           { 0x00, 0x00, 0xAA, 0x80, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_REED_RELAY_DATA              { 0x00, 0x00, 0xAA, 0x81, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_REED_RELAY_ED                { 0x00, 0x00, 0xAA, 0x82, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
#define UUID_REED_RELAY_NOTIFY_PARAM      { 0x00, 0x00, 0xAA, 0x83, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : REED-RELAY */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE128,
    .att_value.value.u128.data = UUID_REED_RELAY_SERVICE,
    .att_uuid.data = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY DATA */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x125A0081AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY DATA */
    .get_action = get_value_reed_relay,
    .set_action = NULL,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_REED_RELAY_DATA,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Rrelay sensor data",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY NOTIFY */
    .get_action = get_status_reed_relay_notify,
    .set_action = set_status_reed_relay_notify,
    .att_value.type = BT_SIZE16,
    .att_value.value.u16 = 0x005A,
    .att_uuid.data = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY ED */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A5E0082AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY ED */
    .get_action = get_status_reed_relay_sensor,
    .set_action = set_status_reed_relay_sensor,
    .att_value.type = BT_SIZE8,
    .att_value.value.u8 = 0x0,
    .att_uuid.data = UUID_REED_RELAY_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "En/dis Rrelay sens",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY PERIOD */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_CHARACTERISTIC,
    .att_value.value.u64 = swap40(0x0A610083AA),
    .att_uuid.data = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY NOTIFY PARAM */
    .get_action = get_period_reed_relay,
    .set_action = set_period_reed_relay,
    .att_value.type = BT_SIZE32,
    .att_uuid.data = UUID_REED_RELAY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = NULL,
    .set_action = NULL,
    .att_value.type = BT_SIZE_STR,
    .att_value.value.str = "Period Rrelay notify",
    .att_uuid.data = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif
  NULL
};
#endif /*GATT_TABLE_INSERT */
#endif /*GATT_TABLE_H_ */
/**********************************************************
 * gatt-table.h
 */
