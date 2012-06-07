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
 *	Lexical analyzer for AQL, the Antelope Query Language.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "aql.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct keyword {
  char *string;
  token_t token;
};

/* The keywords are arranged primarily by length and
   secondarily by expected lookup frequency. */
static const struct keyword keywords[] = {
  {";", END},
  {"(", LEFT_PAREN},
  {")", RIGHT_PAREN},
  {",", COMMA},
  {"=", EQUAL},
  {">", GT},
  {"<", LT},
  {".", DOT},
  {"+", ADD},
  {"-", SUB},
  {"*", MUL},
  {"/", DIV},
  {"#", COMMENT},

  {">=", GEQ},
  {"<=", LEQ},
  {"<>", NOT_EQUAL},
  {"<-", ASSIGN},
  {"OR", OR},
  {"IS", IS},
  {"ON", ON},
  {"IN", IN},

  {"AND", AND},
  {"NOT", NOT},
  {"SUM", SUM},
  {"MAX", MAX},
  {"MIN", MIN},
  {"INT", INT},

  {"INTO", INTO},
  {"FROM", FROM},
  {"MEAN", MEAN},
  {"JOIN", JOIN},
  {"LONG", LONG},
  {"TYPE", TYPE},

  {"WHERE", WHERE},
  {"COUNT", COUNT},
  {"INDEX", INDEX},

  {"INSERT", INSERT},
  {"SELECT", SELECT},
  {"REMOVE", REMOVE},
  {"CREATE", CREATE},
  {"MEDIAN", MEDIAN},
  {"DOMAIN", DOMAIN},
  {"STRING", STRING},
  {"INLINE", INLINE},

  {"PROJECT", PROJECT},
  {"MAXHEAP", MAXHEAP},
  {"MEMHASH", MEMHASH},

  {"RELATION", RELATION},

  {"ATTRIBUTE", ATTRIBUTE}
};

/* Provides a pointer to the first keyword of a specific length. */
static const int8_t skip_hint[] = {0, 13, 21, 27, 33, 36, 44, 47, 48};

static char separators[] = "#.;,() \t\n";

int
lexer_start(lexer_t *lexer, char *input, token_t *token, value_t *value)
{
  lexer->input = input;
  lexer->prev_pos = input;
  lexer->token = token;
  lexer->value = value;

  return 0;
}

static token_t
get_token_id(const char *string, const size_t length)
{
  int start, end;
  int i;

  if(sizeof(skip_hint) < length || length < 1) {
    return NONE;
  }


  start = skip_hint[length - 1];
  if(sizeof(skip_hint) == length) {
    end = sizeof(keywords) / sizeof(keywords[0]);
  } else {
    end = skip_hint[length];
  }

  for(i = start; i < end; i++) {
    if(strncasecmp(keywords[i].string, string, length) == 0) {
      return keywords[i].token;
    }
  }

  return NONE;
}

static int
next_real(lexer_t *lexer, const char *s)
{
  char *end;
  long long_value;
#if DB_FEATURE_FLOATS
  float float_value;
#endif /* DB_FEATURE_FLOATS */

  errno = 0;
  long_value = strtol(s, &end, 10);

#if DB_FEATURE_FLOATS
  if(*end == '.') {
    /* Process a float value. */
    float_value = strtof(s, &end);
    if(float_value == 0 && s == end) {
      return -1;
    }
    memcpy(lexer->value, &float_value, sizeof(float_value));
    *lexer->token = FLOAT_VALUE;
    lexer->input = end;

    return 1;
  }
#endif /* DB_FEATURE_FLOATS */

  /* Process an integer value. */
  if(long_value == 0 && errno != 0) {
      return -1;
  }
  memcpy(lexer->value, &long_value, sizeof(long_value));
  *lexer->token = INTEGER_VALUE;
  lexer->input = end;

  return 1;
}

static int
next_string(lexer_t *lexer, const char *s)
{
  char *end;
  size_t length;

  end = strchr(s, '\'');
  if(end == NULL) {
    return -1;
  }

  length = end - s;
  *lexer->token = STRING_VALUE;
  lexer->input = end + 1; /* Skip the closing delimiter. */

  memcpy(lexer->value, s, length);
  (*lexer->value)[length] = '\0';

  return 1;
}

static int
next_token(lexer_t *lexer, const char *s)
{
  size_t length;

  length = strcspn(s, separators);
  if(length == 0) {
    /* We encountered a separator, so we try to get a token of 
       precisely 1 byte. */
    length = 1;
  }

  *lexer->token = get_token_id(s, length);
  lexer->input = s + length;
  if(*lexer->token != NONE) {
    return 1;
  }

  /* The input did not constitute a valid token,
     so we regard it as an identifier. */

  *lexer->token = IDENTIFIER;

  memcpy(lexer->value, s, length);
  (*lexer->value)[length] = '\0';

  return 1;
}

int
lexer_next(lexer_t *lexer)
{
  const char *s;

  *lexer->token = NONE;
  s = lexer->input;
  s += strspn(s, " \t\n");
  lexer->prev_pos = s;

  switch(*s) {
  case '\'':
    /* Process the string that follows the delimiter. */
    return next_string(lexer, s + 1);
  case '\0':
    return 0;
  default:
    if(isdigit((int)*s) || (*s == '-' && isdigit((int)s[1]))) {
      return next_real(lexer, s);
    }

    /* Process a token. */
    return next_token(lexer, s);
  }
}

void
lexer_rewind(lexer_t *lexer)
{
  lexer->input = lexer->prev_pos;
}
