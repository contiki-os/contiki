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
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "contiki-conf.h"
#include "net/ble/ble-att.h"
#include "net/ble/gatt.h"
#include "uuid.h"
#include "board.h"
#include <stdlib.h>
#include <string.h>

#define GET_NEXT_START_GROUP(x) get_attribute(x + 1)
#define GET_NEXT_BY_UUID(x, y, z) get_attribute_by_uuid(x + 1, y, z)
#define GET_NEXT(x) get_attribute(x + 1)
#define UUID_PRIMARY_16 uuid_16_to_128(PRIMARY_GROUP_TYPE)

static attribute_t *get_attribute_by_uuid(const uint16_t starting_handle, const uint128_t *uuid_to_match, const uint16_t ending_handle);
static attribute_t *get_attribute(const uint16_t handle);

#define GATT_TABLE_INSERT
#include "gatt-table.h"

/*---------------------------------------------------------------------------*/
static attribute_t *
get_attribute(const uint16_t handle)
{
  for(uint16_t i = 0; list_attr[i] != NULL; i++) {
    if(list_attr[i]->att_handle == handle) {
      return list_attr[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_value(const uint16_t handle, bt_size_t *value_ptr)
{
  attribute_t *att;
  PRINTF("GET VALUE\n");
  PRINTF("Handle : %d\n", handle);
  att = get_attribute(handle);

  g_current_att = att;

  if (!att){
    return ATT_ECODE_ATTR_NOT_FOUND;
  }

  if(!att->properties.read) {
    return ATT_ECODE_READ_NOT_PERM;
  }

  if (att->get_action == NULL){
    return ATT_ECODE_ACTION_NOT_SET;
  }

  return att->get_action(value_ptr);
}
/*---------------------------------------------------------------------------*/
uint8_t
set_value(const uint16_t handle, const bt_size_t *new_value)
{
  attribute_t *att;
  PRINTF("SET VALUE\n");
  att = get_attribute(handle);

  if(!att) {
    return ATT_ECODE_ATTR_NOT_FOUND;
  }

  if(!att->properties.write) {
    return ATT_ECODE_WRITE_NOT_PERM;
  }

  if(att->set_action == NULL) {
    return ATT_ECODE_ACTION_NOT_SET;
  }

  g_current_att = att;
  if(new_value->type != att->att_value_len){
    return ATT_ECODE_INVAL_ATTR_VALUE_LEN;
  }


  return att->set_action(new_value);
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_group_end(const uint16_t handle, const uint128_t *uuid_to_match)
{
  uint16_t i;
  for(i = handle; list_attr[i] != NULL; i++) {

    if(uuid_128_compare(list_attr[i]->att_uuid, *uuid_to_match) == 1) {
      return list_attr[i - 1]->att_handle;
    }
  }
  return list_attr[i - 1]->att_handle;
}
/*---------------------------------------------------------------------------*/
static attribute_t *
get_attribute_by_uuid(const uint16_t starting_handle, const uint128_t *uuid_to_match, const uint16_t ending_handle)
{
  attribute_t *att;

  for(uint16_t i = starting_handle; list_attr[i] != NULL && i < ending_handle; i++) {
    att = get_attribute(i);

    if(!att) {
      return NULL;
    }

    if(uuid_128_compare(att->att_uuid, *uuid_to_match) == 1) {
      return att;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
fill_response_tab_group(attribute_t *att, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer)
{
  uint8_t type_previous_value;
  uint16_t group_end_handle;
  uint128_t tmp;
  bt_size_t value_tmp;


  g_tx_buffer->sdu[1] = sizeof(att->att_handle)*2 + att->att_value_len;
  PRINTF("taille : %d",g_tx_buffer->sdu[1]);
  g_tx_buffer->sdu_length += 1;

  while((g_tx_buffer->sdu_length + att->att_value_len) < serveur_mtu){
    /* Look for the end handle of the group */
    group_end_handle = get_group_end(att->att_handle, uuid_to_match);

    /* Copy start handle of current group */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &att->att_handle, sizeof(att->att_handle));
    g_tx_buffer->sdu_length += sizeof(att->att_handle);

    /* Copy end handle of current group */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &group_end_handle, sizeof(group_end_handle));
    g_tx_buffer->sdu_length += sizeof(group_end_handle);

    att->get_action(&value_tmp);
    PRINTF("type : %X, value : 0x%X\n", value_tmp.type, value_tmp.value.u16);
    if(value_tmp.type == BT_SIZE128){
      PRINTF("UUID 128 !\n");
      tmp = swap128(&value_tmp.value.u128);
    }else{
      tmp = value_tmp.value.u128;
    }
    /* Copy value */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &tmp, value_tmp.type);
    g_tx_buffer->sdu_length += value_tmp.type;
    type_previous_value = value_tmp.type;
    att = GET_NEXT_START_GROUP(group_end_handle);

    /* Check if next group is not null or contain other value type */

    if (                 (att == NULL)                                  // verrify if next attribute is null
                      || (value_tmp.type != type_previous_value)   // verrify if next attribute's value is different type
                      || (att->att_handle > ending_handle)              // verrify if next attribute exceed ending_handle
                      || !(att->properties.read)){                      // verrify if next attribute can't be read
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
fill_response_tab(attribute_t *att, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer)
{
  uint8_t type_previous_value;
  bt_size_t value_tmp;


  g_tx_buffer->sdu[1] = sizeof(att->att_handle) + att->att_value_len;
  g_tx_buffer->sdu_length += 1;

  while((g_tx_buffer->sdu_length + att->att_value_len) < serveur_mtu){
    /* Copy start handle of current group */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &att->att_handle, sizeof(att->att_handle));
    g_tx_buffer->sdu_length += sizeof(att->att_handle);

    g_current_att = att;
    /* Find value */
    att->get_action(&value_tmp);
    PRINTF("taille : %X", value_tmp.type);

    /* Copy value */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &value_tmp.value, value_tmp.type);
    g_tx_buffer->sdu_length += value_tmp.type;

    type_previous_value = value_tmp.type;
    att = GET_NEXT_BY_UUID(att->att_handle, uuid_to_match, ending_handle);

    /* Check if next group is not null or contain other value type */
    if (                 (att == NULL)                                  // verrify if next attribute is null
                      || (value_tmp.type != type_previous_value)   // verrify if next attribute's value is different type
                      || (att->att_handle > ending_handle)              // verrify if next attribute exceed ending_handle
                      || !(att->properties.read)){                      // verrify if next attribute can't be read
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
#define SIZE_16BITS_UUID sizeof(uint16_t)
static void
fill_response_find(attribute_t *att, const uint16_t ending_handle, att_buffer_t *g_tx_buffer)
{
  uint16_t current_uuid;

  while((g_tx_buffer->sdu_length + SIZE_16BITS_UUID) < serveur_mtu) {
    /* Copy handle */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &att->att_handle, sizeof(att->att_handle));
    g_tx_buffer->sdu_length += sizeof(att->att_handle);

    current_uuid = uuid_128_to_16(att->att_uuid);
    /* Copy value */
    memcpy(&g_tx_buffer->sdu[g_tx_buffer->sdu_length], &current_uuid, SIZE_16BITS_UUID);
    g_tx_buffer->sdu_length += SIZE_16BITS_UUID;

    att = GET_NEXT(att->att_handle);

    /* Check if next group is not null or contain other value type */
    if((att == NULL)                                                    /* verrify if next attribute is null */
       || (att->att_handle > ending_handle)                             /* verrify if next attribute exceed ending_handle */
       || !(att->properties.read)) {                                    /* verrify if next attribute can't be read */
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
uint8_t
get_group_type_response_values(const uint16_t starting_handle, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer)
{
  attribute_t *att_groupe_start;
  PRINTF("GET GROUP\n");

  /* Change this to support other group type */
  if(uuid_128_compare(*uuid_to_match, UUID_PRIMARY_16) == 0) {
    return ATT_ECODE_UNSUPP_GRP_TYPE;
  }

  /* check if attribute is not null */
  att_groupe_start = get_attribute_by_uuid(starting_handle, uuid_to_match, ending_handle);
  if(!att_groupe_start) {
    return ATT_ECODE_ATTR_NOT_FOUND;
  }

  /* Fill in table */
  fill_response_tab_group(att_groupe_start, ending_handle, uuid_to_match, g_tx_buffer);

  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_type_response_values(const uint16_t starting_handle, const uint16_t ending_handle, const uint128_t *uuid_to_match, att_buffer_t *g_tx_buffer)
{
  attribute_t *att_groupe_start;
  PRINTF("GET type GROUP\n");

  /* check if attribute is not null */
  att_groupe_start = get_attribute_by_uuid(starting_handle, uuid_to_match, ending_handle);
  if(!att_groupe_start) {
    return ATT_ECODE_ATTR_NOT_FOUND;
  }

  /* Fill in table */
  fill_response_tab(att_groupe_start, ending_handle, uuid_to_match, g_tx_buffer);

  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
get_find_info_values(const uint16_t starting_handle, const uint16_t ending_handle, att_buffer_t *g_tx_buffer)
{
  attribute_t *att_groupe_start;
  PRINTF("FIND INFO\n");

  /* check if attribute is not null */
  att_groupe_start = get_attribute(starting_handle);
  if(!att_groupe_start) {
    return ATT_ECODE_ATTR_NOT_FOUND;
  }

  /* Fill in table */
  fill_response_find(att_groupe_start, ending_handle, g_tx_buffer);

  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
#define ATT_GENERIC_ACCESS_DECLARATION 0x1800
uint8_t get_generic_access_service(bt_size_t *value_ptr){
  PRINTF("\nGeneric access service\n");
  value_ptr->type = BT_SIZE16;
  value_ptr->value.u16 = ATT_GENERIC_ACCESS_DECLARATION;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
#define ATT_INFORMATION_SERVICE_DECLARATION 0x180A
uint8_t get_generic_information_service(bt_size_t *value_ptr){
  PRINTF("\nGeneric information service\n");
  value_ptr->type = BT_SIZE16;
  value_ptr->value.u16 = ATT_INFORMATION_SERVICE_DECLARATION;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t get_device_name(bt_size_t *value_ptr){
  value_ptr->type = BT_SIZE_STR;
  memcpy(&value_ptr->value.str, &BOARD_STRING, strlen(BOARD_STRING));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t get_contiki_version(bt_size_t *value_ptr){
  value_ptr->type = BT_SIZE_STR;
  memcpy(&value_ptr->value.str, &CONTIKI_VERSION_STRING, strlen(CONTIKI_VERSION_STRING));
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t get_char_declaration(bt_size_t *value_ptr){
  uint64_t tmp;
  attribute_t *data = GET_NEXT(g_current_att->att_handle);
  value_ptr->type = BT_CHARACTERISTIC;
  tmp =  uuid_128_to_16(data->att_uuid);
  PRINTF("tmp1 : 0x%llX\n", tmp);
  tmp = tmp << 8 * BT_SIZE16;
  PRINTF("handle : 0x%X\n", data->att_handle);
  tmp = tmp + data->att_handle;
  PRINTF("tmp2 : 0x%llX\n", tmp);
  tmp = tmp << 8 * BT_SIZE8;

  tmp = tmp + data->properties.raw;
  // PRINTF("properties : 0x%X\n", data->properties);
  PRINTF("tmp3 : 0x%llX\n", tmp);
  value_ptr->value.u64 = tmp;
  return SUCCESS;
}
/*---------------------------------------------------------------------------*/
