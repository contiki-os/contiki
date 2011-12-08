/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
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
 *	Definitions for attributes.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <stdint.h>
#include <stdlib.h>

#include "lib/list.h"

#include "db-options.h"

typedef enum {
  DOMAIN_UNSPECIFIED = 0,
  DOMAIN_INT = 1,
  DOMAIN_LONG = 2,
  DOMAIN_STRING = 3,
  DOMAIN_FLOAT = 4
} domain_t;

#define ATTRIBUTE_FLAG_NO_STORE		0x1
#define ATTRIBUTE_FLAG_INVALID		0x2
#define ATTRIBUTE_FLAG_PRIMARY_KEY	0x4
#define ATTRIBUTE_FLAG_UNIQUE		0x8

struct attribute {
  struct attribute *next;
  void *index;
  long aggregation_value;
  uint8_t aggregator;
  uint8_t domain;
  uint8_t element_size;
  uint8_t flags;
  char name[ATTRIBUTE_NAME_LENGTH + 1];
};

typedef struct attribute attribute_t;
typedef uint8_t attribute_id_t;

struct attribute_value {
  union {
    int int_value;
    long long_value;
    unsigned char *string_value;
  } u;
  domain_t domain;
};

typedef struct attribute_value attribute_value_t;

#define VALUE_LONG(value)   (value)->u.long_value
#define VALUE_INT(value)    (value)->u.int_value
#define VALUE_STRING(value) (value)->u.string_value

#endif /* ATTRIBUTES_H */
