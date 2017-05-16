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
#ifndef GATT_H_
#define GATT_H_
/*---------------------------------------------------------------------------*/
#include "net/ble/att-database.h"
#include "net/ble/ble-att.h"
#include "net/ble/gatt-services/temp.h"
#include "net/ble/gatt-services/humidity.h"
#include "net/ble/gatt-services/barometer.h"
#include "net/ble/gatt-services/luxometer.h"
#include "net/ble/gatt-services/mpu.h"
#include "net/ble/gatt-services/led.h"
#include "net/ble/gatt-services/battery.h"
#include "net/ble/gatt-services/buttons.h"
#include "net/ble/gatt-services/reedrelay.h"
/*---------------------------------------------------------------------------*/
/* Store the current selectioned attribute */
attribute_t *g_current_att;
/*---------------------------------------------------------------------------*/
/* See read response SpecV5 p 2194 */
uint8_t get_value(const uint16_t handle, bt_size_t *value_ptr);
/* See write response SpecV5 p 2204 */
uint8_t set_value(const uint16_t handle, const bt_size_t *new_value);
/* See read by group type response SpecV5 p 2200 */
uint8_t get_group_type_response_values(const uint16_t starting_handle, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer);
/* See read by type response SpecV5 p 2193 */
uint8_t get_type_response_values(const uint16_t starting_handle, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer);
/* See find information response SpecV5 p 2187 */
uint8_t get_find_info_values(const uint16_t starting_handle, const uint16_t ending_handle, att_buffer_t *g_tx_buffer);

/* See characteristic descriptor discovery SpecV5 p 2256 */
uint8_t get_description(bt_size_t *value_ptr);
/* See characteristic discovery SpecV5 p 2253 */
uint8_t get_char_declaration(bt_size_t *value_ptr);
/* See primary service discovery SpecV5 p 2249 */
uint8_t get_primary_service(bt_size_t *value_ptr);
/* return the device name */
uint8_t get_device_name(bt_size_t *value_ptr);
/* return the contiki version */
uint8_t get_contiki_version(bt_size_t *value_ptr);
/*---------------------------------------------------------------------------*/
#endif /* GATT_H_ */
