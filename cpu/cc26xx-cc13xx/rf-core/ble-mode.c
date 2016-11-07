/*
 * Copyright (c) 2016, Michael Spoerk
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
 * Author: Michael Spoerk <mi.spoerk@gmail.com>
 *
 */
/*---------------------------------------------------------------------------*/

#include <ble-hal.h>
#include <rf-core/ble-hal/ble-hal-cc26xx.h>
#include "contiki.h"
#include "dev/radio.h"

#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint16_t adv_interval;
static ble_adv_type_t adv_type;
static ble_addr_type_t adv_own_addr_type;
static uint8_t adv_channel_map;
static uint16_t buffer_size = 0;
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  int result = ble_hal.reset();
  return result == BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  uint8_t res;
  res = ble_hal.send((void *)payload, payload_len);
  PRINTF("ble-mode send() %d bytes\n", payload_len);
  if(res == BLE_RESULT_OK) {
    return RADIO_TX_OK;
  } else {
    PRINTF("ble-mode send() error: %d\n", res);
    return RADIO_TX_ERR;
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  ble_hal.disconnect(0, 0);
  return 1;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  unsigned int temp;

  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_CONST_CHANNEL_MIN:
    *value = BLE_DATA_CHANNEL_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = BLE_DATA_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_BLE_BUFFER_SIZE:
    if(buffer_size == 0) {
      ble_hal.read_buffer_size((unsigned int *)&buffer_size, &temp);
    }
    memcpy(value, &buffer_size, 2);
    return RADIO_RESULT_OK;
  case RADIO_CONST_BLE_BUFFER_AMOUNT:
    ble_hal.read_buffer_size(&temp, (unsigned int *)value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_BLE_ADV_INTERVAL:
    if((value > BLE_ADV_INTERVAL_MAX) || (value < BLE_ADV_INTERVAL_MIN)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    adv_interval = (uint16_t)value;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_BLE_ADV_TYPE:
    adv_type = value;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_BLE_ADV_OWN_ADDR_TYPE:
    adv_own_addr_type = value;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_BLE_ADV_CHANNEL_MAP:
    adv_channel_map = value;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_BLE_ADV_ENABLE:
    if(value) {
      /* set the advertisement parameter before enabling */
      ble_hal.set_adv_param(adv_interval, adv_type,
                            adv_own_addr_type, adv_channel_map);
    }
    ble_hal.set_adv_enable(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  switch(param) {
  case RADIO_CONST_BLE_BD_ADDR:
    if(size != BLE_ADDR_SIZE || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    ble_hal.read_bd_addr(dest);
    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  switch(param) {
  case RADIO_PARAM_BLE_ADV_PAYLOAD:
    if(size <= 0 || size >= BLE_ADV_DATA_LEN || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    ble_hal.set_adv_data((unsigned short)size, (char *)src);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_BLE_ADV_SCAN_RESPONSE:
    if(size <= 0 || size >= BLE_SCAN_RESP_DATA_LEN || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    ble_hal.set_scan_resp_data((unsigned short)size, (char *)src);
    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver ble_mode_driver = {
  init,
  NULL,
  NULL,
  send,
  NULL,
  NULL,
  NULL,
  NULL,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object,
};
/*---------------------------------------------------------------------------*/
