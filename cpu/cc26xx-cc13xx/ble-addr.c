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
#include "contiki-conf.h"
#include "net/linkaddr.h"
#include "ble-addr.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
void
ble_addr_cpy_to(uint8_t *dst)
{
  int i;
  uint8_t *location = (uint8_t *)BLE_ADDR_LOCATION;

  for(i = 0; i < BLE_ADDR_SIZE; i++) {
    dst[i] = location[BLE_ADDR_SIZE - 1 - i];
  }
}
/*---------------------------------------------------------------------------*/
void
ble_addr_to_eui64(uint8_t *dst, uint8_t *src)
{
  memcpy(dst, src, 3);
  dst[3] = 0xFF;
  dst[4] = 0xFE;
  memcpy(&dst[5], &src[3], 3);
}
/*---------------------------------------------------------------------------*/
void
ble_eui64_addr_cpy_to(uint8_t *dst)
{
  uint8_t ble_addr[BLE_ADDR_SIZE];
  ble_addr_cpy_to(ble_addr);
  ble_addr_to_eui64(dst, ble_addr);
}
