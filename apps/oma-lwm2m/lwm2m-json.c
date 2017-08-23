/*
 * Copyright (c) 2016, Eistec AB.
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
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M JSON writer
 * \author
 *         Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

#include "lwm2m-object.h"
#include "lwm2m-json.h"
#include "lwm2m-plain-text.h"
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static size_t
write_boolean(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
              int value)
{
  int len = snprintf((char *)outbuf, outlen, "{\"e\":[{\"n\":\"%u\",\"bv\":%s}]}\n", ctx->resource_id, value ? "true" : "false");
  if((len < 0) || (len >= outlen)) {
    return 0;
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static size_t
write_int(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
          int32_t value)
{
  int len = snprintf((char *)outbuf, outlen, "{\"e\":[{\"n\":\"%u\",\"v\":%" PRId32 "}]}\n", ctx->resource_id, value);
  if((len < 0) || (len >= outlen)) {
    return 0;
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static size_t
write_float32fix(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
                 int32_t value, int bits)
{
  size_t len = 0;
  int res;
  res = snprintf((char *)outbuf, outlen, "{\"e\":[{\"n\":\"%u\",\"v\":", ctx->resource_id);
  if(res <= 0 || res >= outlen) {
    return 0;
  }
  len += res;
  outlen -= res;
  res = lwm2m_plain_text_write_float32fix(&outbuf[len], outlen, value, bits);
  if((res <= 0) || (res >= outlen)) {
    return 0;
  }
  len += res;
  outlen -= res;
  res = snprintf((char *)&outbuf[len], outlen, "}]}\n");
  if((res <= 0) || (res >= outlen)) {
    return 0;
  }
  len += res;
  return len;
}
/*---------------------------------------------------------------------------*/
static size_t
write_string(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen,
             const char *value, size_t stringlen)
{
  size_t i;
  size_t len = 0;
  int res;
  PRINTF("{\"e\":[{\"n\":\"%u\",\"sv\":\"", ctx->resource_id);
  res = snprintf((char *)outbuf, outlen, "{\"e\":[{\"n\":\"%u\",\"sv\":\"", ctx->resource_id);
  if(res < 0 || res >= outlen) {
    return 0;
  }
  len += res;
  for (i = 0; i < stringlen && len < outlen; ++i) {
    /* Escape special characters */
    /* TODO: Handle UTF-8 strings */
    if(value[i] < '\x20') {
      PRINTF("\\x%x", value[i]);
      res = snprintf((char *)&outbuf[len], outlen - len, "\\x%x", value[i]);
      if((res < 0) || (res >= (outlen - len))) {
        return 0;
      }
      len += res;
      continue;
    } else if(value[i] == '"' || value[i] == '\\') {
      PRINTF("\\");
      outbuf[len] = '\\';
      ++len;
      if(len >= outlen) {
        return 0;
      }
    }
    PRINTF("%c", value[i]);
    outbuf[len] = value[i];
    ++len;
    if(len >= outlen) {
      return 0;
    }
  }
  PRINTF("\"}]}\n");
  res = snprintf((char *)&outbuf[len], outlen - len, "\"}]}\n");
  if((res < 0) || (res >= (outlen - len))) {
    return 0;
  }
  len += res;
  return len;
}
/*---------------------------------------------------------------------------*/
const lwm2m_writer_t lwm2m_json_writer = {
  write_int,
  write_string,
  write_float32fix,
  write_boolean
};
/*---------------------------------------------------------------------------*/
/** @} */
