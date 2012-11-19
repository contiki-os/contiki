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
 *	A recursive parser for AQL, the Antelope Query Language.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "attribute.h"
#include "db-options.h"
#include "index.h"
#include "aql.h"
#include "lvm.h"

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "debug.h"

#if DEBUG
static char error_message[DB_ERROR_BUF_SIZE];
static int error_line;
static const char *error_function;
#define RETURN(value)							\
  do {									\
    if(error_message[0] == '\0') {					\
      strncpy(error_message, lexer->input, sizeof(error_message) - 1);	\
      error_line = __LINE__;						\
      error_function = __func__;					\
    }									\
  } while(0);								\
  return (value)
#define RESET_ERROR()				\
  do {						\
    error_message[0] = '\0';			\
    error_line = 0;				\
    error_function = NULL;			\
  } while(0)
#else
#define RETURN(value) return (value)
#define RESET_ERROR()
#endif
#define PARSER(name)						\
	static aql_status_t					\
	parse_##name(lexer_t *lexer)
#define PARSER_ARG(name, arg)					\
	static aql_status_t					\
	parse_##name(lexer_t *lexer, arg)
#define PARSER_TOKEN(name)					\
	static token_t						\
	parse_##name(lexer_t *lexer)
#define PARSE(name)						\
	!AQL_ERROR(parse_##name(lexer))
#define PARSE_TOKEN(name)					\
	parse_##name(lexer)

#define NEXT   lexer_next(lexer)
#define REWIND lexer_rewind(lexer); RESET_ERROR()
#define TOKEN  *lexer->token
#define VALUE  *lexer->value

#define CONSUME(token)				\
  do {						\
    NEXT;					\
    if(TOKEN != (token)) {		        \
      RETURN(SYNTAX_ERROR);			\
    }						\
  } while(0)

/* The parsing of AQL results in this aql_adt_t object. */
static aql_adt_t *adt;

/* Conditional statements are compiled into VM bytecode, which is stored in
   an instance of the LogicVM. */
static lvm_instance_t p;
static unsigned char vmcode[DB_VM_BYTECODE_SIZE];

/* Parsing functions for AQL. */
PARSER_TOKEN(cmp)
{
  NEXT;
  switch(TOKEN) {
  case EQUAL:
  case NOT_EQUAL:
  case GT:
  case LT:
  case GEQ:
  case LEQ:
    return TOKEN;
  default:
    return NONE;
  }
}

PARSER_TOKEN(op)
{
  NEXT;
  switch(TOKEN) {
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case RIGHT_PAREN:
    return TOKEN;
  default:
    return NONE;
  }
}

PARSER_TOKEN(aggregator)
{
  NEXT;
  switch(TOKEN) {
  case COUNT:
  case SUM:
  case MEAN:
  case MEDIAN:
  case MAX:
  case MIN:
    return TOKEN;
  default:
    return NONE;
  }
}

PARSER(attributes)
{
  token_t token;
  aql_aggregator_t function;

  token = PARSE_TOKEN(aggregator);
  if(token != NONE) {
    switch(TOKEN) {
    case COUNT:
      function = AQL_COUNT;
      break;
    case SUM:
      function = AQL_SUM;
      break;
    case MEAN:
      function = AQL_MEAN;
      break;
    case MEDIAN:
      function = AQL_MEDIAN;
      break;
    case MAX:
      function = AQL_MAX;
      break;
    case MIN:
      function = AQL_MIN;
      break;
    default:
      RETURN(SYNTAX_ERROR);
    }

    AQL_SET_FLAG(adt, AQL_FLAG_AGGREGATE);

    PRINTF("aggregator: %d\n", TOKEN);

    /* Parse the attribute to aggregate. */
    CONSUME(LEFT_PAREN);
    CONSUME(IDENTIFIER);

    AQL_ADD_AGGREGATE(adt, function, VALUE);
    PRINTF("aggregated attribute: %s\n", VALUE);

    CONSUME(RIGHT_PAREN);
    goto check_more_attributes;
  } else {
    REWIND;
  }

  /* Plain identifier. */

  CONSUME(IDENTIFIER);

  AQL_ADD_ATTRIBUTE(adt, VALUE, DOMAIN_UNSPECIFIED, 0);

check_more_attributes:    
  NEXT;
  if(TOKEN == COMMA) {
    if(!PARSE(attributes)) {
      RETURN(SYNTAX_ERROR);
    }
  } else {
    REWIND;
  }

  RETURN(OK);
}

PARSER(relations)
{
  /* Parse comma-separated identifiers for relations. */
  CONSUME(IDENTIFIER);

  AQL_ADD_RELATION(adt, VALUE);
  NEXT;
  if(TOKEN == COMMA) {
    if(!PARSE(relations)) {
      RETURN(SYNTAX_ERROR);
    }
  } else {
    REWIND;
  }

  RETURN(OK);
}

PARSER(values)
{
  /* Parse comma-separated attribute values. */
  NEXT;
  switch(TOKEN) {
  case STRING_VALUE:
    AQL_ADD_VALUE(adt, DOMAIN_STRING, VALUE);
    break;
  case INTEGER_VALUE:
    AQL_ADD_VALUE(adt, DOMAIN_INT, VALUE);
    break;
  default:
    RETURN(SYNTAX_ERROR);
  }

  NEXT;
  if(TOKEN == COMMA) {
    return PARSE(values);
  } else {
    REWIND;
  }

  RETURN(OK);
}

PARSER(operand)
{
  NEXT;
  switch(TOKEN) {
  case IDENTIFIER:
    lvm_register_variable(VALUE, LVM_LONG);
    lvm_set_variable(&p, VALUE);
    AQL_ADD_PROCESSING_ATTRIBUTE(adt, VALUE);
    break;
  case STRING_VALUE:
    break;
  case FLOAT_VALUE:
    break;
  case INTEGER_VALUE:
    lvm_set_long(&p, *(long *)lexer->value);
    break;
  default:
    RETURN(SYNTAX_ERROR);
  }

  RETURN(OK);
}

PARSER(expr)
{
  token_t token;
  size_t saved_end;
  operator_t op;

  saved_end = lvm_get_end(&p);

  NEXT;
  if(TOKEN == LEFT_PAREN) {
    if(!PARSE(expr)) {
      RETURN(SYNTAX_ERROR);
    }
    CONSUME(RIGHT_PAREN);
  } else {
    REWIND;
    if(!PARSE(operand)) {
      RETURN(SYNTAX_ERROR);
    }
  }

  while(1) {
    token = PARSE_TOKEN(op);
    if(token == NONE) {
      saved_end = lvm_get_end(&p);
      REWIND;
      break;
    } else if (token == RIGHT_PAREN) {
      break;
    }

    if(!PARSE(operand) && !PARSE(expr)) {
	RETURN(SYNTAX_ERROR);
    }

    saved_end = lvm_shift_for_operator(&p, saved_end);

    switch(token) {
    case ADD:
      op = LVM_ADD;
      break;
    case SUB:
      op = LVM_SUB;
      break;
    case MUL:
      op = LVM_MUL;
      break;
    case DIV:
      op = LVM_DIV;
      break;
    default:
      RETURN(SYNTAX_ERROR);
    }
    lvm_set_op(&p, op);
    lvm_set_end(&p, saved_end);
  }

  return OK;
}

PARSER(comparison)
{
  token_t token;
  size_t saved_end;
  operator_t rel;

  saved_end = lvm_jump_to_operand(&p);

  if(!PARSE(expr)) {
    RETURN(SYNTAX_ERROR);
  }

  saved_end = lvm_set_end(&p, saved_end);

  token = PARSE_TOKEN(cmp);
  if(token == NONE) {
    RETURN(SYNTAX_ERROR);
  }

  switch(token) {
  case GT:
    rel = LVM_GE;
    break;
  case GEQ:
    rel = LVM_GEQ;
    break;
  case LT:
    rel = LVM_LE;
    break;
  case LEQ:
    rel = LVM_LEQ;
    break;
  case EQUAL:
    rel = LVM_EQ;
    break;
  case NOT_EQUAL:
    rel = LVM_NEQ;
    break;
  default:
    RETURN(SYNTAX_ERROR);
  }

  lvm_set_relation(&p, rel);
  lvm_set_end(&p, saved_end);

  if(!PARSE(expr)) {
    RETURN(SYNTAX_ERROR);
  }

  RETURN(OK);
}

PARSER(where)
{
  int r;
  operator_t connective;
  size_t saved_end;

  if(!PARSE(comparison)) {
    RETURN(SYNTAX_ERROR);
  }
  
  saved_end = 0;

  /* The WHERE clause can consist of multiple prepositions. */
  for(;;) {
    NEXT;
    if(TOKEN != AND && TOKEN != OR) {
      REWIND;
      break;
    }

    connective = TOKEN == AND ? LVM_AND : LVM_OR;

    saved_end = lvm_shift_for_operator(&p, saved_end);
    lvm_set_relation(&p, connective);
    lvm_set_end(&p, saved_end);
  
    NEXT;
    if(TOKEN == LEFT_PAREN) {
      r = PARSE(where);
      if(!r) {
	RETURN(SYNTAX_ERROR);
      }
      CONSUME(RIGHT_PAREN);
    } else {
      REWIND;
      r = PARSE(comparison);
      if(!r) {
	RETURN(r);
      }
    }
  }

  lvm_print_code(&p);

  return OK;
}

PARSER(join)
{
  AQL_SET_TYPE(adt, AQL_TYPE_JOIN);

  CONSUME(IDENTIFIER);

  PRINTF("Left relation: %s\n", VALUE);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(COMMA);
  CONSUME(IDENTIFIER);

  PRINTF("Right relation: %s\n", VALUE);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(ON);
  CONSUME(IDENTIFIER);

  PRINTF("Join on attribute %s\n", VALUE);
  AQL_ADD_ATTRIBUTE(adt, VALUE, DOMAIN_UNSPECIFIED, 0);

  CONSUME(PROJECT);

  /* projection attributes... */
  if(!PARSE(attributes)) {
    RETURN(SYNTAX_ERROR);
  }

  CONSUME(END);

  RETURN(OK);
}

PARSER(select)
{
  AQL_SET_TYPE(adt, AQL_TYPE_SELECT);

  /* projection attributes... */
  if(!PARSE(attributes)) {
    RETURN(SYNTAX_ERROR);
  }

  CONSUME(FROM);
  if(!PARSE(relations)) {
    RETURN(SYNTAX_ERROR);
  }

  NEXT;
  if(TOKEN == WHERE) {
    lvm_reset(&p, vmcode, sizeof(vmcode));

    if(!PARSE(where)) {
      RETURN(SYNTAX_ERROR);
    }

    AQL_SET_CONDITION(adt, &p);
  } else {
    REWIND;
    RETURN(OK);
  }

  CONSUME(END);

  return OK;
}

PARSER(insert)
{
  AQL_SET_TYPE(adt, AQL_TYPE_INSERT);

  CONSUME(LEFT_PAREN);

  if(!PARSE(values)) {
    RETURN(SYNTAX_ERROR);
  }

  CONSUME(RIGHT_PAREN);
  CONSUME(INTO);

  if(!PARSE(relations)) {
    RETURN(SYNTAX_ERROR);
  }

  RETURN(OK);
}

PARSER(remove_attribute)
{
  AQL_SET_TYPE(adt, AQL_TYPE_REMOVE_ATTRIBUTE);

  CONSUME(IDENTIFIER);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(DOT);
  CONSUME(IDENTIFIER);

  PRINTF("Removing the index for the attribute %s\n", VALUE);
  AQL_ADD_ATTRIBUTE(adt, VALUE, DOMAIN_UNSPECIFIED, 0);

  RETURN(OK);
}

#if DB_FEATURE_REMOVE
PARSER(remove_from)
{
  AQL_SET_TYPE(adt, AQL_TYPE_REMOVE_TUPLES);

  /* Use a temporary persistent relation to assign the query result to. */
  AQL_SET_FLAG(adt, AQL_FLAG_ASSIGN);
  AQL_ADD_RELATION(adt, REMOVE_RELATION);

  CONSUME(IDENTIFIER);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(WHERE);

  lvm_reset(&p, vmcode, sizeof(vmcode));
  AQL_SET_CONDITION(adt, &p);

  return PARSE(where);

}
#endif /* DB_FEATURE_REMOVE */

PARSER(remove_index)
{
  AQL_SET_TYPE(adt, AQL_TYPE_REMOVE_INDEX);

  CONSUME(IDENTIFIER);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(DOT);
  CONSUME(IDENTIFIER);

  PRINTF("remove index: %s\n", VALUE);
  AQL_ADD_ATTRIBUTE(adt, VALUE, DOMAIN_UNSPECIFIED, 0);

  RETURN(OK);
}

PARSER(remove_relation)
{
  AQL_SET_TYPE(adt, AQL_TYPE_REMOVE_RELATION);

  CONSUME(IDENTIFIER);
  PRINTF("remove relation: %s\n", VALUE);
  AQL_ADD_RELATION(adt, VALUE);

  RETURN(OK);
}

PARSER(remove)
{
  aql_status_t r;

  NEXT;
  switch(TOKEN) {
  case ATTRIBUTE:
    r = PARSE(remove_attribute);
    break;
#if DB_FEATURE_REMOVE
  case FROM:
    r = PARSE(remove_from);
    break;
#endif
  case INDEX:
    r = PARSE(remove_index);
    break;
  case RELATION:
    r = PARSE(remove_relation);
    break;
  default:
    RETURN(SYNTAX_ERROR);
  }

  if(!r) {
    RETURN(SYNTAX_ERROR);
  }

  CONSUME(END);

  RETURN(OK);
}

PARSER_TOKEN(index_type)
{
  index_type_t type;

  NEXT;
  switch(TOKEN) {
  case INLINE:
    type = INDEX_INLINE;
    break;
  case MAXHEAP:
    type = INDEX_MAXHEAP;
    break;
  case MEMHASH:
    type = INDEX_MEMHASH;
    break;
  default:
    return NONE;
  };

  AQL_SET_INDEX_TYPE(adt, type);
  return TOKEN;
}

PARSER(create_index)
{
  AQL_SET_TYPE(adt, AQL_TYPE_CREATE_INDEX);

  CONSUME(IDENTIFIER);
  AQL_ADD_RELATION(adt, VALUE);

  CONSUME(DOT);
  CONSUME(IDENTIFIER);

  PRINTF("Creating an index for the attribute %s\n", VALUE);
  AQL_ADD_ATTRIBUTE(adt, VALUE, DOMAIN_UNSPECIFIED, 0);

  CONSUME(TYPE);

  if(PARSE_TOKEN(index_type) == NONE) {
    RETURN(SYNTAX_ERROR);
  }

  RETURN(OK);
}

PARSER(create_relation)
{
  CONSUME(IDENTIFIER);

  AQL_SET_TYPE(adt, AQL_TYPE_CREATE_RELATION);
  AQL_ADD_RELATION(adt, VALUE);

  RETURN(OK);
}

PARSER_ARG(domain, char *name)
{
  domain_t domain;
  unsigned element_size;

  NEXT;
  switch(TOKEN) {
  case STRING:
    domain = DOMAIN_STRING;

    /* Parse the amount of characters for this domain. */
    CONSUME(LEFT_PAREN);
    CONSUME(INTEGER_VALUE);
    element_size = *(long *)lexer->value;
    CONSUME(RIGHT_PAREN);

    break;
  case LONG:
    domain = DOMAIN_LONG;
    element_size = 4;
    break;
  case INT:
    domain = DOMAIN_INT;
    element_size = 2;
    break;
  default:
    return NONE;
  }

  AQL_ADD_ATTRIBUTE(adt, name, domain, element_size);

  return OK;
}

PARSER(create_attributes)
{
  aql_status_t r;
  char name[ATTRIBUTE_NAME_LENGTH];

  AQL_SET_TYPE(adt, AQL_TYPE_CREATE_ATTRIBUTE);

  CONSUME(IDENTIFIER);

  strncpy(name, VALUE, sizeof(name) - 1);
  name[sizeof(name) - 1] = '\0';

  CONSUME(DOMAIN);

  r = parse_domain(lexer, name);
  if(AQL_ERROR(r)) {
    RETURN(r);
  }

  CONSUME(IN);
  CONSUME(IDENTIFIER);

  AQL_ADD_RELATION(adt, VALUE);

  RETURN(OK);
}

PARSER(create)
{
  aql_status_t r;

  NEXT;
  switch(TOKEN) {
  case ATTRIBUTE:
    r = PARSE(create_attributes);
    break;
  case INDEX:
    r = PARSE(create_index);
    break;
  case RELATION:
    r = PARSE(create_relation);
    break;
  default:
    RETURN(SYNTAX_ERROR);
  }

  if(!r) {
    RETURN(SYNTAX_ERROR);
  }

  CONSUME(END);

  RETURN(OK);
}

aql_status_t
aql_parse(aql_adt_t *external_adt, char *input_string)
{
  lexer_t lex;
  token_t token = NONE;
  value_t value;
  aql_status_t result;

  RESET_ERROR();

  PRINTF("Parsing \"%s\"\n", input_string);

  adt = external_adt;
  AQL_CLEAR(adt);
  AQL_SET_CONDITION(adt, NULL);

  lexer_start(&lex, input_string, &token, &value);

  result = lexer_next(&lex);
  if(!AQL_ERROR(result)) {
    switch(token) {
    case IDENTIFIER:
      PRINTF("Assign the result to relation %s\n", *lex.value);
      AQL_ADD_RELATION(adt, *lex.value);
      AQL_SET_FLAG(adt, AQL_FLAG_ASSIGN);
      if(AQL_ERROR(lexer_next(&lex))) {
	result = SYNTAX_ERROR;
	break;
      }
      if(*lex.token != ASSIGN) {
	result = SYNTAX_ERROR;
	break;
      }
      if(AQL_ERROR(lexer_next(&lex))) {
	result = SYNTAX_ERROR;
	break;
      }
      switch(*lex.token) {
      case SELECT:
        result = parse_select(&lex);
	break;
      case JOIN:
	result = parse_join(&lex);
	break;
      default:
	result = SYNTAX_ERROR;
      }
      break;
    case JOIN:
      result = parse_join(&lex);
      break;
    case CREATE:
      result = parse_create(&lex);
      break;
    case REMOVE:
      result = parse_remove(&lex);
      break;
    case INSERT:
      result = parse_insert(&lex);
      break;
    case SELECT:
      result = parse_select(&lex);
      break;
    case NONE:
    case COMMENT:
      result = OK;
    case END:
      break;
    default:
      result = SYNTAX_ERROR;
    }
  }

  if(AQL_ERROR(result)) {
    PRINTF("Error in function %s, line %d: input \"%s\"\n",
	   error_function, error_line, error_message);
  }

  return result;
}
