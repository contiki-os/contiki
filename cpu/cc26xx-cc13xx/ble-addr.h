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
#ifndef BLE_ADDR_H_
#define BLE_ADDR_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* primary BLE address location */
#define BLE_ADDR_LOCATION   0x500012E8

/* BLE device address size */
#define BLE_ADDR_SIZE 6

/*---------------------------------------------------------------------------*/
/* Type of BLE device address */
typedef enum {
  BLE_ADDR_TYPE_PUBLIC,
  BLE_ADDR_TYPE_RANDOM
} ble_addr_type_t;

/*---------------------------------------------------------------------------*/
void ble_addr_cpy_to(uint8_t *dst);

void ble_addr_to_eui64(uint8_t *dst, uint8_t *src);

void ble_eui64_addr_cpy_to(uint8_t *dst);
/*---------------------------------------------------------------------------*/

#endif /* BLE_ADDR_H_ */
