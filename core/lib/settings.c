/*
 * Copyright (c) 2013, Robert Quattlebaum.
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

#ifdef SETTINGS_CONF_SKIP_CONVENIENCE_FUNCS
#undef SETTINGS_CONF_SKIP_CONVENIENCE_FUNCS
#endif

#define SETTINGS_CONF_SKIP_CONVENIENCE_FUNCS 1

#include "contiki.h"
#include "settings.h"
#include "dev/eeprom.h"

#if CONTIKI_CONF_SETTINGS_MANAGER

#if !EEPROM_CONF_SIZE
#error CONTIKI_CONF_SETTINGS_MANAGER has been set, but EEPROM_CONF_SIZE hasnt!
#endif

#ifndef EEPROM_END_ADDR
#define EEPROM_END_ADDR         (EEPROM_CONF_SIZE - 1)
#endif

#ifndef SETTINGS_MAX_SIZE
/** The maximum amount EEPROM dedicated to settings. */
#define SETTINGS_MAX_SIZE	(127)  /**< Defaults to 127 bytes */
#endif

#ifndef SETTINGS_TOP_ADDR
/** The top address in EEPROM that settings should use. Inclusive. */
#define SETTINGS_TOP_ADDR	(settings_iter_t)(EEPROM_END_ADDR)
#endif

#ifndef SETTINGS_BOTTOM_ADDR
/** The lowest address in EEPROM that settings should use. Inclusive. */
#define SETTINGS_BOTTOM_ADDR	(SETTINGS_TOP_ADDR + 1 - SETTINGS_MAX_SIZE)
#endif

typedef struct {
#if SETTINGS_CONF_SUPPORT_LARGE_VALUES
  uint8_t size_extra;
#endif
  uint8_t size_low;
  uint8_t size_check;
  settings_key_t key;
} item_header_t;

/*****************************************************************************/
// MARK: - Public Travesal Functions
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
settings_iter_t
settings_iter_begin()
{
  return settings_iter_is_valid(SETTINGS_TOP_ADDR) ? SETTINGS_TOP_ADDR : 0;
}

/*---------------------------------------------------------------------------*/
settings_iter_t
settings_iter_next(settings_iter_t ret)
{
  if(ret) {
    /* A settings iterator always points to the first byte
     * after the actual key-value pair in memory. This means that
     * the address of our value in EEPROM just happens
     * to be the address of our next iterator.
     */
    ret = settings_iter_get_value_addr(ret);
    return settings_iter_is_valid(ret) ? ret : 0;
  }
  return SETTINGS_INVALID_ITER;
}

/*---------------------------------------------------------------------------*/
uint8_t
settings_iter_is_valid(settings_iter_t iter)
{
  item_header_t header = { 0 };

  if(iter == EEPROM_NULL) {
    return 0;
  }

  if(iter < SETTINGS_BOTTOM_ADDR + sizeof(header)) {
    return 0;
  }

  eeprom_read(iter - sizeof(header), (uint8_t *)&header, sizeof(header));

  if((uint8_t) header.size_check != (uint8_t) ~ header.size_low) {
    return 0;
  }

  if(iter < SETTINGS_BOTTOM_ADDR + sizeof(header) + settings_iter_get_value_length(iter)) {
    return 0;
  }

  return 1;
}

/*---------------------------------------------------------------------------*/
settings_key_t
settings_iter_get_key(settings_iter_t iter)
{
  item_header_t header;

  eeprom_read(iter - sizeof(header), (uint8_t *)&header, sizeof(header));

  if((uint8_t) header.size_check != (uint8_t)~header.size_low) {
    return SETTINGS_INVALID_KEY;
  }

  return header.key;
}

/*---------------------------------------------------------------------------*/
settings_length_t
settings_iter_get_value_length(settings_iter_t iter)
{
  item_header_t header;

  settings_length_t ret = 0;

  eeprom_read(iter - sizeof(header), (uint8_t *)&header, sizeof(header) );

  if((uint8_t)header.size_check == (uint8_t)~header.size_low) {
    ret = header.size_low;

#if SETTINGS_CONF_SUPPORT_LARGE_VALUES
    if(ret & (1 << 7)) {
      ret = ((ret & ~(1 << 7)) << 7) | header.size_extra;
    }
#endif
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
eeprom_addr_t
settings_iter_get_value_addr(settings_iter_t iter)
{
  settings_length_t len = settings_iter_get_value_length(iter);
#if SETTINGS_CONF_SUPPORT_LARGE_VALUES
  len += (len >= 128);
#endif
  return iter - sizeof(item_header_t) - len;
}

/*---------------------------------------------------------------------------*/
settings_length_t
settings_iter_get_value_bytes(settings_iter_t iter, void *bytes,
                              settings_length_t max_length)
{
  max_length = MIN(max_length, settings_iter_get_value_length(iter));

  eeprom_read(settings_iter_get_value_addr(iter), bytes, max_length);

  return max_length;
}

/*---------------------------------------------------------------------------*/
settings_status_t
settings_iter_delete(settings_iter_t iter)
{
  settings_status_t ret = SETTINGS_STATUS_FAILURE;

  settings_iter_t next = settings_iter_next(iter);

  if(!next) {
    /* Special case: we are the last item. we can get away with
     * just wiping out our own header.
     */
    item_header_t header;

    memset(&header, 0xFF, sizeof(header));

    eeprom_write(iter - sizeof(header), (uint8_t *)&header, sizeof(header));

    ret = SETTINGS_STATUS_OK;
  }

  /* This case requires the settings store to be shifted.
   * Currently unimplemented. TODO: Writeme!
   */
  ret = SETTINGS_STATUS_UNIMPLEMENTED;

  return ret;
}

/*****************************************************************************/
// MARK: - Public Functions
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
uint8_t
settings_check(settings_key_t key, uint8_t index)
{
  uint8_t ret = 0;

  settings_iter_t iter;

  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    if(settings_iter_get_key(iter) == key) {
      if(!index) {
        ret = 1;
        break;
      } else {
        index--;
      }
    }
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
settings_status_t
settings_get(settings_key_t key, uint8_t index, uint8_t *value,
             settings_length_t * value_size)
{
  settings_status_t ret = SETTINGS_STATUS_NOT_FOUND;

  settings_iter_t iter;

  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    if(settings_iter_get_key(iter) == key) {
      if(!index) {
        /* We found it! */
        *value_size = settings_iter_get_value_bytes(iter,
                                                    (void *)value,
                                                    *value_size);
        ret = SETTINGS_STATUS_OK;
        break;
      } else {
        /* Nope, keep looking */
        index--;
      }
    }
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
settings_status_t
settings_add(settings_key_t key, const uint8_t *value,
             settings_length_t value_size)
{
  settings_status_t ret = SETTINGS_STATUS_FAILURE;

  settings_iter_t iter;

  item_header_t header;

  /* Find the last item. */
  for(iter = settings_iter_begin(); settings_iter_next(iter);
      iter = settings_iter_next(iter)) {
    /* This block intentionally left blank. */
  }

  if(iter) {
    /* Value address of item is the same as the iterator for next item. */
    iter = settings_iter_get_value_addr(iter);
  } else {
    /* This will be the first setting! */
    iter = SETTINGS_TOP_ADDR;
  }

  if(iter < SETTINGS_BOTTOM_ADDR + value_size + sizeof(header)) {
    /* This value is too big to store. */
    ret = SETTINGS_STATUS_OUT_OF_SPACE;
    goto bail;
  }

  header.key = key;

  if(value_size < 0x80) {
    /* If the value size is less than 128, then
     * we can get away with only using one byte
     * to store the size.
     */
    header.size_low = value_size;
  }
#if SETTINGS_CONF_SUPPORT_LARGE_VALUES
  else if(value_size <= SETTINGS_MAX_VALUE_SIZE) {
    /* If the value size is larger than or equal to 128,
     * then we need to use two bytes. Store
     * the most significant 7 bits in the first
     * size byte (with MSB set) and store the
     * least significant bits in the second
     * byte (with LSB clear)
     */
    header.size_low = (value_size >> 7) | 0x80;
    header.size_extra = value_size & ~0x80;
  }
#endif
  else {
    /* Value size way too big! */
    ret = SETTINGS_STATUS_VALUE_TOO_BIG;
    goto bail;
  }

  header.size_check = ~header.size_low;

  /* Write the header first */
  eeprom_write(iter - sizeof(header), (uint8_t *)&header, sizeof(header));

  /* Sanity check, remove once confident */
  if(settings_iter_get_value_length(iter) != value_size) {
    goto bail;
  }

  /* Now write the data */
  eeprom_write(settings_iter_get_value_addr(iter), (uint8_t *)value, value_size);

  /* This should be the last item. If this is not the case,
   * then we need to clear out the phantom setting.
   */
  if((iter = settings_iter_next(iter))) {
    memset(&header, 0xFF, sizeof(header));

    eeprom_write(iter - sizeof(header),(uint8_t *)&header, sizeof(header));
  }

  ret = SETTINGS_STATUS_OK;

bail:
  return ret;
}

/*---------------------------------------------------------------------------*/
settings_status_t
settings_set(settings_key_t key, const uint8_t *value,
             settings_length_t value_size)
{
  settings_status_t ret = SETTINGS_STATUS_FAILURE;

  settings_iter_t iter;

  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    if(settings_iter_get_key(iter) == key) {
      break;
    }
  }

  if((iter == EEPROM_NULL) || !settings_iter_is_valid(iter)) {
    ret = settings_add(key, value, value_size);
    goto bail;
  }

  if(value_size != settings_iter_get_value_length(iter)) {
    /* Requires the settings store to be shifted. Currently unimplemented. */
    ret = SETTINGS_STATUS_UNIMPLEMENTED;
    goto bail;
  }

  /* Now write the data */
  eeprom_write(settings_iter_get_value_addr(iter),
               (uint8_t *)value, value_size);

  ret = SETTINGS_STATUS_OK;

bail:
  return ret;
}

/*---------------------------------------------------------------------------*/
settings_status_t
settings_delete(settings_key_t key, uint8_t index)
{
  settings_status_t ret = SETTINGS_STATUS_NOT_FOUND;

  settings_iter_t iter;

  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    if(settings_iter_get_key(iter) == key) {
      if(!index) {
        /* We found it! */
        ret = settings_iter_delete(iter);
        break;
      } else {
        /* Nope, keep looking */
        index--;
      }
    }
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
void
settings_wipe(void)
{
  /* Simply making the first item invalid will effectively
   * clear the key-value store.
   */
  const uint32_t x = 0xFFFFFF;

  eeprom_write(SETTINGS_TOP_ADDR - sizeof(x), (uint8_t *)&x, sizeof(x));
}

/*****************************************************************************/
// MARK: - Other Functions
/*****************************************************************************/

#if DEBUG
#include <stdio.h>
/*---------------------------------------------------------------------------*/
void
settings_debug_dump(void)
{
  settings_iter_t iter;

  printf("{\n");
  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    settings_length_t len = settings_iter_get_value_length(iter);
    eeprom_addr_t addr = settings_iter_get_value_addr(iter);
    uint8_t byte;

    union {
      settings_key_t key;
      char bytes[0];
    } u;

    u.key = settings_iter_get_key(iter);

    printf("\t\"%c%c\" = <", u.bytes[0], u.bytes[1]);

    for(; len; len--, addr++) {
      eeprom_read(addr, &byte, 1);
      printf("%02X", byte);
      if(len != 1) {
        printf(" ");
      }
    }

    printf(">;\n");
  }
  printf("}\n");
}
#endif /* DEBUG */

#endif /* CONTIKI_CONF_SETTINGS_MANAGER */
