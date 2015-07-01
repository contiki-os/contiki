/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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

/**
 * \file
 *         CCM* convenience functions for LLSEC use
 * \author
 *         Justin King-Lacroix <justin.kinglacroix@gmail.com>
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "llsec/ccm-star-packetbuf.h"
#include "lib/ccm-star.h"
#include "net/packetbuf.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
static const uint8_t *
get_extended_address(const linkaddr_t *addr)
#if LINKADDR_SIZE == 2
{
  /* workaround for short addresses: derive EUI64 as in RFC 6282 */
  static linkaddr_extended_t template = { { 0x00 , 0x00 , 0x00 ,
                                            0xFF , 0xFE , 0x00 , 0x00 , 0x00 } };
  
  template.u16[3] = LLSEC802154_HTONS(addr->u16);
  
  return template.u8;
}
#else /* LINKADDR_SIZE == 2 */
{
  return addr->u8;
}
#endif /* LINKADDR_SIZE == 2 */
/*---------------------------------------------------------------------------*/
/* Inits the 13-byte CCM* nonce as of 802.15.4-2011. */
static void
set_nonce(uint8_t *nonce, const linkaddr_t *source_addr)
{
  memcpy(nonce, get_extended_address(source_addr), 8);
  nonce[8] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) >> 8;
  nonce[9] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) & 0xff;
  nonce[10] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) >> 8;
  nonce[11] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) & 0xff;
  nonce[12] = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);
}
/*---------------------------------------------------------------------------*/
void
ccm_star_packetbuf_mic(const linkaddr_t *source_addr,
    uint8_t *result,
    uint8_t mic_len)
{
  uint8_t nonce[CCM_STAR_NONCE_LENGTH];
  uint8_t *m;
  uint8_t m_len;
  uint8_t *a;
  uint8_t a_len;

  set_nonce(nonce, source_addr);
  
  a = packetbuf_hdrptr();
  if(packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL) & (1 << 2)) {
    m = packetbuf_dataptr();
    m_len = packetbuf_datalen();
    a_len = packetbuf_hdrlen();
  } else {
    m = NULL;
    m_len = 0;
    a_len = packetbuf_totlen();
  }
  
  CCM_STAR.mic(nonce,
      m, m_len,
      a, a_len,
      result,
      mic_len);
}
/*---------------------------------------------------------------------------*/
void
ccm_star_packetbuf_ctr(const linkaddr_t *source_addr)
{
  uint8_t nonce[CCM_STAR_NONCE_LENGTH];
  
  set_nonce(nonce, source_addr);
  CCM_STAR.ctr(nonce, packetbuf_dataptr(), packetbuf_datalen());
}
/*---------------------------------------------------------------------------*/
