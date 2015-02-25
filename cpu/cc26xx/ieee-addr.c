/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-ieee-addr
 * @{
 *
 * \file
 * Driver for the CC26xx IEEE addresses
 */
/*---------------------------------------------------------------------------*/
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
    int i;

    /* Reading from primary location... */
    uint8_t *location = (uint8_t *)IEEE_ADDR_LOCATION_PRIMARY;

    /*
     * ...unless we can find a byte != 0xFF in secondary
     *
     * Intentionally checking all 8 bytes here instead of len, because we
     * are checking validity of the entire IEEE address irrespective of the
     * actual number of bytes the caller wants to copy over.
     */
    for(i = 0; i < 8; i++) {
      if(((uint8_t *)IEEE_ADDR_LOCATION_SECONDARY)[i] != 0xFF) {
        /* A byte in the secondary location is not 0xFF. Use the secondary */
        location = (uint8_t *)IEEE_ADDR_LOCATION_SECONDARY;
        break;
      }
    }

    /*
     * We have chosen what address to read the IEEE address from. Do so,
     * inverting byte order
     */
    for(i = 0; i < len; i++) {
      dst[i] = location[len - 1 - i];
    }
  }

#if IEEE_ADDR_NODE_ID
  dst[len - 1] = IEEE_ADDR_NODE_ID & 0xFF;
  dst[len - 2] = IEEE_ADDR_NODE_ID >> 8;
#endif
}
/*---------------------------------------------------------------------------*/
/** @} */
