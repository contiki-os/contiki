/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

#include "contiki.h"
#include "net/packetbuf.h"
#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/
int
packetutils_serialize_atts(uint8_t *data, int size)
{
  int i;
  /* set the length first later */
  int pos = 1;
  int cnt = 0;
  /* assume that values are 16-bit */
  uint16_t val;
  PRINTF("packetutils: serializing packet atts");
  for(i = 0; i < PACKETBUF_NUM_ATTRS; i++) {
    val = packetbuf_attr(i);
    if(val != 0) {
      if(pos + 3 > size) {
        return -1;
      }
      data[pos++] = i;
      data[pos++] = val >> 8;
      data[pos++] = val & 255;
      cnt++;
      PRINTF(" %d=%d", i, val);
    }
  }
  PRINTF(" (%d)\n", cnt);

  data[0] = cnt;
  return pos;
}
/*---------------------------------------------------------------------------*/
int
packetutils_deserialize_atts(const uint8_t *data, int size)
{
  int i, cnt, pos;

  pos = 0;
  cnt = data[pos++];
  PRINTF("packetutils: deserializing %d packet atts:", cnt);
  if(cnt > PACKETBUF_NUM_ATTRS) {
    PRINTF(" *** too many: %u!\n", PACKETBUF_NUM_ATTRS);
    return -1;
  }
  for(i = 0; i < cnt; i++) {
    if(data[pos] >= PACKETBUF_NUM_ATTRS) {
      /* illegal attribute identifier */
      PRINTF(" *** unknown attribute %u\n", data[pos]);
      return -1;
    }
    PRINTF(" %d=%d", data[pos], (data[pos + 1] << 8) | data[pos + 2]);
    packetbuf_set_attr(data[pos], (data[pos + 1] << 8) | data[pos + 2]);
    pos += 3;
  }
  PRINTF("\n");
  return pos;
}
/*---------------------------------------------------------------------------*/
