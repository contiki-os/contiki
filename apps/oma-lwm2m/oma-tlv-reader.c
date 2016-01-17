/*
 * Copyright (c) 2015, Yanzi Networks AB.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
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
 * \addtogroup oma-lwm2m
 * @{
 *
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M TLV reader
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include "oma-tlv-reader.h"
#include "oma-tlv.h"

/*---------------------------------------------------------------------------*/
static size_t
read_int(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len,
         int32_t *value)
{
  oma_tlv_t tlv;
  size_t size;
  size = oma_tlv_read(&tlv, inbuf, len);
  if(size > 0) {
    *value = oma_tlv_get_int32(&tlv);
  }
  return size;
}
/*---------------------------------------------------------------------------*/
static size_t
read_string(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len,
            uint8_t *value, size_t stringlen)
{
  oma_tlv_t tlv;
  size_t size;
  size = oma_tlv_read(&tlv, inbuf, len);
  if(size > 0) {
    if(stringlen <= tlv.length) {
      /* The outbuffer can not contain the full string including ending zero */
      return 0;
    }
    memcpy(value, tlv.value, tlv.length);
    value[tlv.length] = '\0';
  }
  return size;
}
/*---------------------------------------------------------------------------*/
static size_t
read_float32fix(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len,
                int32_t *value, int bits)
{
  oma_tlv_t tlv;
  size_t size;
  size = oma_tlv_read(&tlv, inbuf, len);
  if(size > 0) {
    oma_tlv_float32_to_fix(&tlv, value, bits);
  }
  return size;
}
/*---------------------------------------------------------------------------*/
static size_t
read_boolean(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len,
             int *value)
{
  oma_tlv_t tlv;
  size_t size;
  size = oma_tlv_read(&tlv, inbuf, len);
  if(size > 0) {
    *value = oma_tlv_get_int32(&tlv) != 0;
  }
  return size;
}
/*---------------------------------------------------------------------------*/
const lwm2m_reader_t oma_tlv_reader = {
  read_int,
  read_string,
  read_float32fix,
  read_boolean
};
/*---------------------------------------------------------------------------*/
/** @} */
