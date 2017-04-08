/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 *
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
 */
/**
 * \addtogroup cc2538-ieee-addr
 * @{
 *
 * \file
 * Driver for the cc2538 IEEE addresses
 */
#include "contiki-conf.h"
#include "net/linkaddr.h"
#include "ieee-addr.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
void
ieee_addr_cpy_to(uint8_t *dst, uint8_t len)
{
  if(IEEE_ADDR_CONF_HARDCODED) {
    uint8_t ieee_addr_hc[8] = IEEE_ADDR_CONF_ADDRESS;

    memcpy(dst, &ieee_addr_hc[8 - len], len);
  } else {
    /*
     * By default, we assume that the IEEE address is stored on flash using
     * little-endian byte order.
     *
     * However, some SoCs ship with a different byte order, whereby the first
     * four bytes are flipped with the four last ones.
     *
     *     Using this address as an example: 00 12 4B 00 01 02 03 04
     *               We expect it stored as: 04 03 02 01 00 4B 12 00
     * But it is also possible to encounter: 00 4B 12 00 04 03 02 01
     *
     * Thus: read locations [3, 2, 1] and if we encounter the TI OUI, flip the
     * order of the two 4-byte sequences. Each of the 4-byte sequences is still
     * little-endian.
     */
    int i;
    uint8_t oui_ti[3] = IEEE_ADDR_OUI_TI;
    if(((uint8_t *)IEEE_ADDR_LOCATION)[3] == oui_ti[0]
       && ((uint8_t *)IEEE_ADDR_LOCATION)[2] == oui_ti[1]
       && ((uint8_t *)IEEE_ADDR_LOCATION)[1] == oui_ti[2]) {
      for(i = 0; i < len / 2; i++) {
        dst[i] = ((uint8_t *)IEEE_ADDR_LOCATION)[len / 2 - 1 - i];
      }
      for(i = 0; i < len / 2; i++) {
        dst[i + len / 2] = ((uint8_t *)IEEE_ADDR_LOCATION)[len - 1 - i];
      }
    } else {
      for(i = 0; i < len; i++) {
        dst[i] = ((uint8_t *)IEEE_ADDR_LOCATION)[len - 1 - i];
      }
    }
  }

#if IEEE_ADDR_NODE_ID
  dst[len - 1] = IEEE_ADDR_NODE_ID & 0xFF;
  dst[len - 2] = IEEE_ADDR_NODE_ID >> 8;
#endif
}

/** @} */
