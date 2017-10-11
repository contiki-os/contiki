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
#ifndef ATT_DATABASE_H_
#define ATT_DATABASE_H_
#include "uuid.h"
/* Struct of each ATT attributes */
typedef const struct attribute_s {
  /* action attached in order to get the attribute value */
  uint8_t (*get_action)(bt_size_t *value);
  /* action attached in order to set the attribute value */
  uint8_t (*set_action)(const bt_size_t *value);
  /* size of the value, used for checking the new value size in set fuction */
  uint8_t att_value_len;
  /* current UUID of the attribute */
  uint16_t att_uuid;
  /* current handle of the attribute */
  uint16_t att_handle;

  /* Specific parameters for attributes */
  union {
    /* desctiption used for Descriptor attributes */
    const char *description;
    /* current_service used in primary declaration attributes */
    uint16_t current_service;
  }specific;

/* Properties */
  union {
    struct {
      uint8_t broadcast : 1;
      uint8_t read : 1;
      uint8_t write_without_response : 1;
      uint8_t write : 1;
      uint8_t notify : 1;
      uint8_t indicate : 1;
      uint8_t authenticated_signed_writes : 1;
      uint8_t extended_properties : 1;
    };
    uint8_t raw;
  }properties;
} attribute_t;
#endif /* ATT_DATABASE_H_ */
