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
 *         Implementation of the Contiki OMA LWM2M TLV writer
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include "oma-tlv.h"
/*---------------------------------------------------------------------------*/
static size_t
write_boolean_tlv(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
                  int value)
{
  return oma_tlv_write_int32(ctx->resource_id, value != 0 ? 1 : 0,
                             outbuf, outlen);
}
/*---------------------------------------------------------------------------*/
static size_t
write_int_tlv(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
              int32_t value)
{
  return oma_tlv_write_int32(ctx->resource_id, value, outbuf, outlen);
}
/*---------------------------------------------------------------------------*/
static size_t
write_float32fix_tlv(const lwm2m_context_t *ctx, uint8_t *outbuf,
                     size_t outlen, int32_t value, int bits)
{
  return oma_tlv_write_float32(ctx->resource_id, value, bits, outbuf, outlen);
}
/*---------------------------------------------------------------------------*/
static size_t
write_string_tlv(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
                 const char *value, size_t stringlen)
{
  oma_tlv_t tlv;
  tlv.type = OMA_TLV_TYPE_RESOURCE;
  tlv.value = (uint8_t *) value;
  tlv.length = (uint32_t) stringlen;
  tlv.id = ctx->resource_id;
  return oma_tlv_write(&tlv, outbuf, outlen);
}
/*---------------------------------------------------------------------------*/
const lwm2m_writer_t oma_tlv_writer = {
  write_int_tlv,
  write_string_tlv,
  write_float32fix_tlv,
  write_boolean_tlv
};
/*---------------------------------------------------------------------------*/
/** @} */
