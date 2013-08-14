/*
 * Copyright (c) 2013, Robert Quattlebaum
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#ifndef __CONTIKI_SETTINGS_H__
#define __CONTIKI_SETTINGS_H__

/** @file   settings.h
 *  @brief  Settings Manager
 *  @author Robert Quattlebaum <darco@deepdarc.com>
 *
 *  ## Overview ##
 *
 *  The settings manager is a EEPROM-based key-value store. Keys
 *  are 16-bit integers and values may be up to 16,383 bytes long.
 *  It is intended to be used to store configuration-related information,
 *  like network settings, radio channels, etc.
 *
 *  ## Features ##
 *
 *   * Robust data format which requires no initialization.
 *   * Supports multiple values with the same key.
 *   * Data can be appended without erasing EEPROM.
 *   * Max size of settings data can be easily increased in the future,
 *     as long as it doesn't overlap with application data.
 *
 *  ## Data Format ##
 *
 *  The format was inspired by OLPC manufacturing data, as described here:
 *  <http://wiki.laptop.org/go/Manufacturing_data>
 *
 *  Since the beginning of EEPROM often contains application-specific
 *  information, the best place to store settings is at the end of
 *  EEPROM. Because we are starting at the end of EEPROM, it makes sense
 *  to grow the list of key-value pairs downward, toward the start of
 *  EEPROM.
 *
 *  Each key-value pair is stored in memory in the following format:
 * <table>
 *  <thead>
 *   <td>Order</td>
 *   <td>Size<small> (in bytes)</small></td>
 *   <td>Name</td>
 *   <td>Description</td>
 *  </thead>
 *  <tr>
 *   <td>0</td>
 *   <td>2</td>
 *   <td>key</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>-2</td>
 *   <td>1</td>
 *   <td>size_check</td>
 *   <td>One's-complement of next byte</td>
 *  </tr>
 *  <tr>
 *   <td>-3</td>
 *   <td>1 or 2</td>
 *   <td>size</td>
 *   <td>The size of the value, in bytes.</td>
 *  </tr>
 *  <tr>
 *   <td>-4 or -5</td>
 *   <td>variable</td>
 *   <td>value</td>
 *  </tr>
 * </table>
 *
 *  The end of the key-value pairs is denoted by the first invalid entry.
 *  An invalid entry has any of the following attributes:
 *
 *   * The size_check byte doesn't match the one's compliment
 *     of the size byte (or size_low byte).
 *   * The key has a value of 0x0000.
 *
 */

#include <stdint.h>
#include <string.h>
#include "dev/eeprom.h"
#include "sys/cc.h"

/*****************************************************************************/
// MARK: - Types

typedef enum {
  SETTINGS_STATUS_OK = 0,
  SETTINGS_STATUS_FAILURE,
  SETTINGS_STATUS_INVALID_ARGUMENT,
  SETTINGS_STATUS_NOT_FOUND,
  SETTINGS_STATUS_OUT_OF_SPACE,
  SETTINGS_STATUS_VALUE_TOO_BIG,
  SETTINGS_STATUS_UNIMPLEMENTED,
} settings_status_t;

typedef uint16_t settings_key_t;

typedef uint16_t settings_length_t;

/*****************************************************************************/
// MARK: - Settings Keys

/** Two-character constant macro */
#define TCC(a,b)                   ((a)+(b)*256)

/* All-capital-letter constants are always contiki-defined. */
#define SETTINGS_KEY_EUI64         TCC('E','8') /**< EUI64 Address, 8 bytes */
#define SETTINGS_KEY_EUI48         TCC('E','6') /*!< MAC Address, 6 bytes */
#define SETTINGS_KEY_CHANNEL       TCC('C','H') /*!< Channel number, uint8_t */
#define SETTINGS_KEY_TXPOWER       TCC('T','P') /*!< Transmit power, uint8_t */
#define SETTINGS_KEY_PAN_ID        TCC('P','N') /*!< PAN ID, uint16_t */
#define SETTINGS_KEY_PAN_ADDR      TCC('P','A') /*!< PAN address, uint16_t */
#define SETTINGS_KEY_AES128KEY     TCC('S','K') /*!< AES128 key, 16 bytes */
#define SETTINGS_KEY_AES128ENABLED TCC('S','E') /*!< AES128 enabled, bool */
#define SETTINGS_KEY_HOSTNAME      TCC('H','N') /*!< Hostname, C-String */
#define SETTINGS_KEY_DOMAINNAME    TCC('D','N') /*!< Domainname, C-String */

/*****************************************************************************/
// MARK: - Experimental Settings Keys

#define SETTINGS_KEY_RDC_INDEX     TCC('R','D') /*!< RDC index, uint8_t */
#define SETTINGS_KEY_CHANNEL_MASK  TCC('C','M') /*!< Channel mask, uint16_t */

/*****************************************************************************/
// MARK: - Constants

/** Use this when you want to retrieve the last item */
#define SETTINGS_LAST_INDEX        0xFF

#define SETTINGS_INVALID_KEY       0xFFFF

#define SETTINGS_INVALID_ITER      EEPROM_NULL

#ifndef SETTINGS_CONF_SUPPORT_LARGE_VALUES
#define SETTINGS_CONF_SUPPORT_LARGE_VALUES  0
#endif

#if SETTINGS_CONF_SUPPORT_LARGE_VALUES
#define SETTINGS_MAX_VALUE_SIZE    0x3FFF        /* 16383 bytes */
#else
#define SETTINGS_MAX_VALUE_SIZE    0x7F        /* 127 bytes */
#endif

/*****************************************************************************/
// MARK: - Settings accessors

/** Fetches the value associated with the given key. */
extern settings_status_t settings_get(settings_key_t key, uint8_t index,
                                      uint8_t *value,
                                      settings_length_t * value_size);

/** Adds the given key-value pair to the end of the settings store. */
extern settings_status_t settings_add(settings_key_t key,
                                      const uint8_t *value,
                                      settings_length_t value_size);

/** Checks to see if the given key exists. */
extern uint8_t settings_check(settings_key_t key, uint8_t index);

/** Reinitializes all of the EEPROM used by settings. */
extern void settings_wipe(void);

/** Sets the value for the given key. If the key already exists in
 *  the settings store, then its value will be replaced.
 */
extern settings_status_t settings_set(settings_key_t key,
                                      const uint8_t *value,
                                      settings_length_t value_size);

/** Removes the given key (at the given index) from the settings store. */
extern settings_status_t settings_delete(settings_key_t key, uint8_t index);

/*****************************************************************************/
// MARK: - Settings traversal functions

typedef eeprom_addr_t settings_iter_t;

/** Will return extern SETTINGS_INVALID_ITER if the settings store is empty. */
extern settings_iter_t settings_iter_begin();

/** Will return extern SETTINGS_INVALID_ITER if at the end of settings list. */
extern settings_iter_t settings_iter_next(settings_iter_t iter);

extern uint8_t settings_iter_is_valid(settings_iter_t iter);

extern settings_key_t settings_iter_get_key(settings_iter_t iter);

extern settings_length_t settings_iter_get_value_length(settings_iter_t iter);

extern eeprom_addr_t settings_iter_get_value_addr(settings_iter_t iter);

extern settings_length_t settings_iter_get_value_bytes(settings_iter_t item,
                                                       void *bytes,
                                                       settings_length_t
                                                       max_length);

extern settings_status_t settings_iter_delete(settings_iter_t item);

/*****************************************************************************/
// MARK: - inline convenience functions

/* Unfortunately, some platforms don't properly drop unreferenced functions,
 * so on these broken platforms we can save a significant amount
 * of space by skipping the definition of the convenience functions.
 */
#if !SETTINGS_CONF_SKIP_CONVENIENCE_FUNCS

static CC_INLINE const char *
settings_get_cstr(settings_key_t key, uint8_t index, char *c_str,
                  settings_length_t c_str_size)
{
  /* Save room for the zero termination. */
  c_str_size--;

  if(settings_get(key, index, (uint8_t *)c_str, &c_str_size) == SETTINGS_STATUS_OK) {
    /* Zero terminate. */
    c_str[c_str_size] = 0;
  } else {
    c_str = NULL;
  }
  return c_str;
}

static CC_INLINE settings_status_t
settings_set_cstr(settings_key_t key, const char* c_str)
{
  return settings_set(key, (const uint8_t *)c_str, strlen(c_str));
}

static CC_INLINE settings_status_t
settings_add_cstr(settings_key_t key, const char* c_str)
{
  return settings_add(key, (const uint8_t *)c_str, strlen(c_str));
}

static CC_INLINE uint8_t
settings_get_bool_with_default(settings_key_t key, uint8_t index,
                               uint8_t default_value)
{
  uint8_t ret = default_value;
  settings_length_t sizeof_uint8 = sizeof(uint8_t);

  settings_get(key, index, (uint8_t *)&ret, &sizeof_uint8);
  return !!ret;
}

static CC_INLINE uint8_t
settings_get_uint8(settings_key_t key, uint8_t index)
{
  uint8_t ret = 0;
  settings_length_t sizeof_uint8 = sizeof(uint8_t);

  settings_get(key, index, (uint8_t *)&ret, &sizeof_uint8);
  return ret;
}

static CC_INLINE settings_status_t
settings_add_uint8(settings_key_t key, uint8_t value)
{
  return settings_add(key, (const uint8_t *)&value, sizeof(uint8_t));
}

static CC_INLINE settings_status_t
settings_set_uint8(settings_key_t key, uint8_t value)
{
  return settings_set(key, (const uint8_t *)&value, sizeof(uint8_t));
}

static CC_INLINE uint16_t
settings_get_uint16(settings_key_t key, uint8_t index)
{
  uint16_t ret = 0;
  settings_length_t sizeof_uint16 = sizeof(uint16_t);

  settings_get(key, index, (uint8_t *)&ret, &sizeof_uint16);
  return ret;
}

static CC_INLINE settings_status_t
settings_add_uint16(settings_key_t key, uint16_t value)
{
  return settings_add(key, (const uint8_t *)&value, sizeof(uint16_t));
}

static CC_INLINE settings_status_t
settings_set_uint16(settings_key_t key, uint16_t value)
{
  return settings_set(key, (const uint8_t *)&value, sizeof(uint16_t));
}

static CC_INLINE uint32_t
settings_get_uint32(settings_key_t key, uint8_t index)
{
  uint32_t ret = 0;
  settings_length_t sizeof_uint32 = sizeof(uint32_t);

  settings_get(key, index, (uint8_t *)&ret, &sizeof_uint32);
  return ret;
}

static CC_INLINE settings_status_t
settings_add_uint32(settings_key_t key, uint32_t value)
{
  return settings_add(key, (const uint8_t *)&value, sizeof(uint32_t));
}

static CC_INLINE settings_status_t
settings_set_uint32(settings_key_t key, uint32_t value)
{
  return settings_set(key, (const uint8_t *)&value, sizeof(uint32_t));
}

#if __int64_t_defined
static CC_INLINE uint64_t
settings_get_uint64(settings_key_t key, uint8_t index)
{
  uint64_t ret = 0;
  settings_length_t sizeof_uint64 = sizeof(uint64_t);

  settings_get(key, index, (uint8_t *)&ret, &sizeof_uint64);
  return ret;
}

static CC_INLINE settings_status_t
settings_add_uint64(settings_key_t key, uint64_t value)
{
  return settings_add(key, (const uint8_t *)&value, sizeof(uint64_t));
}

static CC_INLINE settings_status_t
settings_set_uint64(settings_key_t key, uint64_t value)
{
  return settings_set(key, (const uint8_t *)&value, sizeof(uint64_t));
}
#endif /* __int64_t_defined */

#endif /* !SETTINGS_CONF_SKIP_CONVENIENCE_FUNCS */

#endif /* !defined(__CONTIKI_SETTINGS_H__) */
