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

#define UUID_PRIMARY_DECLARATION          0x2800
#define UUID_CHARACTERISTIC_DECLARATION   0x2803
#define UUID_CLIENT_CHARACTERISTIC_CONFIGURATION   0x2902

#define UUID_DEVICE_NAME                  0x2A00
#define UUID_CONTIKI_VERSION              0x2A28

#define UUID_USER_DESC      0x2901

#ifndef GATT_TABLE_H_
#define GATT_TABLE_H_
#include "contiki-conf.h"
#include "gatt_config.h"
#ifdef GATT_TABLE_INSERT

/*
 * This table contains all the GATT attributes
 */
static const attribute_t *list_attr[] =
{
#define ATT_GENERIC_ACCESS_DECLARATION 0x1800
#define ATT_INFORMATION_SERVICE_DECLARATION 0x180A
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : GENERIC ACCESS SERVICE */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = ATT_GENERIC_ACCESS_DECLARATION,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : DEVICE NAME */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* DEVICE NAME */
    .get_action = get_device_name,
    .set_action = NULL,
    .att_value_len = BT_SIZE_STR,
    .att_uuid = UUID_DEVICE_NAME,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : INFORMATION SERVICE */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = ATT_INFORMATION_SERVICE_DECLARATION,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : CONTIKI VERSION */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CONTIKI VERSION */
    .get_action = get_contiki_version,
    .set_action = NULL,
    .att_value_len = BT_SIZE_STR,
    .att_uuid = UUID_CONTIKI_VERSION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#ifdef GATT_SENSORS_BATTERY
#define UUID_BATTERY_SERVICE              0x180F
#define UUID_BATTERY_DATA                 0x2A19
#define UUID_BATTERY_DATA_TEMP            0xAA00
#define UUID_BATTERY_ED                   0xAA01
#define UUID_BATTERY_NOTIFY_PARAM         0xAA02

  &(attribute_t){   /* PRIMARY SERVICE DECLARATION : BATTERY */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_BATTERY_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* CHAR DECLARATION : BATTERY DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* BATTERY DATA */
    .get_action = get_value_battery,
    .set_action = NULL,
    .att_uuid = UUID_BATTERY_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Battery level",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* BATTERY NOTIFY */
    .get_action = get_status_battery_notify,
    .set_action = set_status_battery_notify,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .att_value_len = BT_SIZE16,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* CHAR DECLARATION : BATTERY Temp */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* BATTERY DATA */
    .get_action = get_value_battery_temp,
    .set_action = NULL,
    .att_uuid = UUID_BATTERY_DATA_TEMP,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Battery temp",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* CHAR DECLARATION : BATTERY ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* BATTERY ED */
    .get_action = get_status_battery_sensor,
    .set_action = set_status_battery_sensor,
    .att_uuid = UUID_BATTERY_ED,
    .att_value_len = BT_SIZE8,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "E/D Battery sensor",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* CHAR DECLARATION : BATTERY PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* BATTERY NOTIFY PARAM */
    .get_action = get_period_battery,
    .set_action = set_period_battery,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_BATTERY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Period battery notify",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#if defined GATT_SENSORS_TEMP1 || defined GATT_SENSORS_HUM || defined GATT_SENSORS_BAROMETER
#define UUID_ENVIRONMENTAL_SENSING_SERVICE 0x181A
#define UUID_TEMP_DATA                     0x2A6E
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : ENVIRONMENTAL_SERVICE */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_ENVIRONMENTAL_SENSING_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },

#ifdef GATT_SENSORS_TEMP1
#define UUID_TEMP_ED                       0xAA02
#define UUID_TEMP_NOTIFY_PARAM             0xAA03

  &(attribute_t){ /* CHAR DECLARATION : TEMP DATA AMBIANT*/
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP DATA AMBIANT */
    .get_action = get_value_temp_ambiant,
    .set_action = NULL,
    .att_uuid = UUID_TEMP_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Temp ambiant",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP NOTIFY */
    .get_action = get_status_temp_notify,
    .set_action = set_status_temp_notify,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .att_value_len = BT_SIZE16,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP DATA OBJECT*/
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP DATA */
    .get_action = get_value_temp_object,
    .set_action = NULL,
    .att_uuid = UUID_TEMP_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Temp object",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP ED */
    .get_action = get_status_temp_sensor,
    .set_action = set_status_temp_sensor,
    .att_uuid = UUID_TEMP_ED,
    .att_value_len = BT_SIZE8,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "En/dis temp sens",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : TEMP PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* TEMP NOTIFY PARAM */
    .get_action = get_period_temp,
    .set_action = set_period_temp,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_TEMP_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .specific.description = "Temp Notify",
    .att_uuid = UUID_USER_DESC,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif /*GATT_SENSORS_TEMP1 */

#ifdef GATT_SENSORS_HUM
#define UUID_HUMIDITY_DATA               0x2A6F
#define UUID_HUMIDITY_ED                 0xAA12
#define UUID_HUMIDITY_NOTIFY_PARAM       0xAA13
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY DATA */
    .get_action = get_value_humidity_hum,
    .set_action = NULL,
    .att_uuid = UUID_HUMIDITY_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Hum data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY NOTIFY */
    .get_action = get_status_humidity_notify,
    .set_action = set_status_humidity_notify,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .att_value_len = BT_SIZE16,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY Temp */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* HUMIDITY DATA temp */
    .get_action = get_value_humidity_temp,
    .set_action = NULL,
    .att_uuid = UUID_TEMP_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){   /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Hum temp",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY ED */
    .get_action = get_status_humidity_sensor,
    .set_action = set_status_humidity_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_HUMIDITY_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Hum enable",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : HUMIDITY PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* HUMIDITY NOTIFY PARAM */
    .get_action = get_period_humidity,
    .set_action = set_period_humidity,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_HUMIDITY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Hum notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_BAROMETER
#define UUID_BAROMETER_DATA               0x2A6D
#define UUID_BAROMETER_ED                 0xAA22
#define UUID_BAROMETER_NOTIFY_PARAM       0xAA23

  &(attribute_t){ /* CHAR DECLARATION : BAROMETER DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BAROMETER DATA */
    .get_action = get_value_barometer_pressure,
    .set_action = NULL,
    .att_uuid = UUID_BAROMETER_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "barometer data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* barometer NOTIFY */
    .get_action = get_status_barometer_notify,
    .set_action = set_status_barometer_notify,
    .att_value_len = BT_SIZE16,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BAROMETER DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BAROMETER DATA */
    .get_action = get_value_barometer_temp,
    .set_action = NULL,
    .att_uuid = UUID_TEMP_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "barometer temp",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BAROMETER ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BAROMETER ED */
    .get_action = get_status_barometer_sensor,
    .set_action = set_status_barometer_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_BAROMETER_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "barometer enable",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : barometer PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* barometer NOTIFY PARAM */
    .get_action = get_period_barometer,
    .set_action = set_period_barometer,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_BAROMETER_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "barometer notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif
#endif

#ifdef GATT_SENSORS_LUXOMETER
#define UUID_LUXOMETER_SERVICE            0xAA30
#define UUID_LUXOMETER_DATA               0xAA31
#define UUID_LUXOMETER_ED                 0xAA32
#define UUID_LUXOMETER_NOTIFY_PARAM       0xAA33

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : LUXOMETER */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_LUXOMETER_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LUXOMETER DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER DATA */
    .get_action = get_value_luxometer,
    .set_action = NULL,
    .att_uuid = UUID_LUXOMETER_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Lux sensor data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER NOTIFY */
    .get_action = get_status_luxometer_notify,
    .set_action = set_status_luxometer_notify,
    .att_value_len = BT_SIZE16,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LUXOMETER ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LUXOMETER ED */
    .get_action = get_status_luxometer_sensor,
    .set_action = set_status_luxometer_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_LUXOMETER_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "en/dis lux sensor",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : luxometer PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* luxometer NOTIFY PARAM */
    .get_action = get_period_luxometer,
    .set_action = set_period_luxometer,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_LUXOMETER_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "period lux notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_MPU
#define UUID_MPU_SERVICE                  0xAA40
#define UUID_MPU_DATA                     0xAA41
#define UUID_MPU_ED                       0xAA42
#define UUID_MPU_NOTIFY_PARAM             0xAA43

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : MPU */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_MPU_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU DATA */
    .get_action = get_value_mpu,
    .set_action = NULL,
    .att_uuid = UUID_MPU_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "MPU sensor data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU NOTIFY */
    .get_action = get_status_mpu_notify,
    .set_action = set_status_mpu_notify,
    .att_value_len = BT_SIZE16,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU ED */
    .get_action = get_status_mpu_sensor,
    .set_action = set_status_mpu_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_MPU_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "en/dis mpu sensor",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : MPU PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* MPU NOTIFY PARAM */
    .get_action = get_period_mpu,
    .set_action = set_period_mpu,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_MPU_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "period mpu notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif
#ifdef GATT_LEDS
#define UUID_LED_SERVICE                  0xAA50
#define UUID_LED_DATA                     0xAA51
#define UUID_LED_ED                       0xAA52
  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : LED */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_LED_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : LED DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* LED DATA */
    .get_action = get_status_leds,
    .set_action = set_status_leds,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_LED_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Led data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_BUTTONS
#define UUID_BUTTONS_SERVICE              0xAA60
#define UUID_BUTTONS_DATA                 0xAA61
#define UUID_BUTTONS_ED                   0xAA62
#define UUID_BUTTONS_NOTIFY_PARAM         0xAA63

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : BUTTONS */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_BUTTONS_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS DATA */
    .get_action = get_value_buttons,
    .set_action = NULL,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_BUTTONS_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Buttons data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS NOTIFY */
    .get_action = get_status_buttons_notify,
    .set_action = set_status_buttons_notify,
    .att_value_len = BT_SIZE16,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS ED */
    .get_action = get_status_buttons_sensor,
    .set_action = set_status_buttons_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_BUTTONS_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "En/dis buttons sens",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : BUTTONS PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* BUTTONS NOTIFY PARAM */
    .get_action = get_period_buttons,
    .set_action = set_period_buttons,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_BUTTONS_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Period buttons notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif

#ifdef GATT_SENSORS_REED_RELAY
#define UUID_REED_RELAY_SERVICE           0xAA70
#define UUID_REED_RELAY_DATA              0xAA71
#define UUID_REED_RELAY_ED                0xAA72
#define UUID_REED_RELAY_NOTIFY_PARAM      0xAA73

  &(attribute_t){ /* PRIMARY SERVICE DECLARATION : REED-RELAY */
    .get_action = get_primary_service,
    .set_action = NULL,
    .att_value_len = BT_SIZE16,
    .specific.current_service = UUID_REED_RELAY_SERVICE,
    .att_uuid = UUID_PRIMARY_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY DATA */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY DATA */
    .get_action = get_value_reed_relay,
    .set_action = NULL,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_REED_RELAY_DATA,
    .properties.notify = 1,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Rrelay sensor data",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY NOTIFY */
    .get_action = get_status_buttons_notify,
    .set_action = set_status_buttons_notify,
    .att_value_len = BT_SIZE16,
    .att_uuid = UUID_CLIENT_CHARACTERISTIC_CONFIGURATION,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY ED */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY ED */
    .get_action = get_status_reed_relay_sensor,
    .set_action = set_status_reed_relay_sensor,
    .att_value_len = BT_SIZE8,
    .att_uuid = UUID_REED_RELAY_ED,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "En/dis Rrelay sens",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* CHAR DECLARATION : REED-RELAY PERIOD */
    .get_action = get_char_declaration,
    .set_action = NULL,
    .att_value_len = BT_CHARACTERISTIC,
    .att_uuid = UUID_CHARACTERISTIC_DECLARATION,
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* REED-RELAY NOTIFY PARAM */
    .get_action = get_period_reed_relay,
    .set_action = set_period_reed_relay,
    .att_value_len = BT_SIZE32,
    .att_uuid = UUID_REED_RELAY_NOTIFY_PARAM,
    .properties.write = 1,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
  &(attribute_t){ /* User description */
    .get_action = get_description,
    .set_action = NULL,
    .att_uuid = UUID_USER_DESC,
    .specific.description = "Period Rrelay notify",
    .properties.write = 0,
    .properties.read = 1,
    .att_handle = __COUNTER__ + 1,
  },
#endif /* GATT_SENSORS_REED_RELAY */
  NULL
};
#endif /* GATT_TABLE_INSERT */
#endif /* GATT_TABLE_H_ */
