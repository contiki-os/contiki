/*
* Copyright (c) 2014, CETIC.
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

/**
* \file
*         Simple CoAP Library
* \author
*         6LBR Team <6lbr@cetic.be>
*/

#ifndef COAP_DATA_FORMAT_H
#define COAP_DATA_FORMAT_H

struct coap_data_format_s {
  /* Data type */
  int (*accepted_type)(unsigned int type);
  int (*format_type)(unsigned int type);

  /* Write data */
  int (*format_value)(uint8_t *buffer, int buffer_size, int offset, unsigned int accepted_type, int resource_type, char const * resource_name, void *data);
  int (*start_batch)(uint8_t *buffer, int buffer_size, int offset, unsigned int accepted_type);
  int (*batch_separator)(uint8_t *buffer, int buffer_size, int offset, unsigned int accepted_type);
  int (*end_batch)(uint8_t *buffer, int buffer_size, int offset, unsigned int accepted_type);

  /* Parse data */
  int (*parse_value)(uint8_t const *buffer, uint8_t const * max, unsigned int data_type, int resource_type, char const * resource_name, void *data);
};

typedef struct coap_data_format_s coap_data_format_t;

#define ADD_CHAR_IF_POSSIBLE(c) \
  if(strpos >= offset && bufpos < buffer_size) { \
    buffer[bufpos++] = c; \
  } \
  ++strpos

#define ADD_STRING_IF_POSSIBLE(string) \
  tmplen = strlen(string); \
  if(strpos + tmplen > offset) { \
    bufpos += snprintf((char *)buffer + bufpos, \
                       buffer_size - bufpos + 1, \
                       "%s", \
                       &((char *)(string))[ \
                       (offset - (int32_t)strpos > 0 ? \
                        offset - (int32_t)strpos : 0)]); \
    if(bufpos > buffer_size) { \
      break; \
    } \
  } \
  strpos += tmplen

#define ADD_STATIC_IF_POSSIBLE(string) ADD_STRING_IF_POSSIBLE(string)

#define ADD_FORMATTED_STRING_IF_POSSIBLE(fmt, ...) \
  snprintf(tmpbuf, 16, fmt, __VA_ARGS__); \
  ADD_STRING_IF_POSSIBLE(tmpbuf)

#endif
