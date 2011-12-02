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
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *	Definitions and declarations for the Propositional Logic Engine.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef LVM_H
#define LVM_H

#include <stdlib.h>

#include "db-options.h"

enum lvm_status {
  FALSE = 0,
  TRUE = 1,
  INVALID_IDENTIFIER = 2,
  SEMANTIC_ERROR = 3,
  MATH_ERROR = 4,
  STACK_OVERFLOW = 5,
  TYPE_ERROR = 6,
  VARIABLE_LIMIT_REACHED = 7,
  EXECUTION_ERROR = 8,
  DERIVATION_ERROR = 9
};

typedef enum lvm_status lvm_status_t;

#define LVM_ERROR(x)	(x >= 2)

typedef int lvm_ip_t;

struct lvm_instance {
  unsigned char *code;
  lvm_ip_t size;
  lvm_ip_t end;
  lvm_ip_t ip;
  unsigned error;
};
typedef struct lvm_instance lvm_instance_t;

enum node_type {
  LVM_ARITH_OP = 0x10,
  LVM_OPERAND = 0x20,
  LVM_CMP_OP = 0x40,
  LVM_CONNECTIVE = 0x80
};
typedef enum node_type node_type_t;

enum operator {
  LVM_ADD = LVM_ARITH_OP | 1,
  LVM_SUB = LVM_ARITH_OP | 2,
  LVM_MUL = LVM_ARITH_OP | 3,
  LVM_DIV = LVM_ARITH_OP | 4,
  LVM_EQ  = LVM_CMP_OP | 1,
  LVM_NEQ = LVM_CMP_OP | 2,
  LVM_GE  = LVM_CMP_OP | 3,
  LVM_GEQ = LVM_CMP_OP | 4,
  LVM_LE  = LVM_CMP_OP | 5,
  LVM_LEQ = LVM_CMP_OP | 6,
  LVM_AND = LVM_CONNECTIVE | 1,
  LVM_OR  = LVM_CONNECTIVE | 2,
  LVM_NOT = LVM_CONNECTIVE | 3
};
typedef enum operator operator_t;

enum operand_type {
  LVM_VARIABLE,
  LVM_FLOAT,
  LVM_LONG
};
typedef enum operand_type operand_type_t;

typedef unsigned char variable_id_t;

typedef union {
  long l;
#if LVM_USE_FLOATS
  float f;
#endif
  variable_id_t id;
} operand_value_t;

struct operand {
  operand_type_t type;
  operand_value_t value;
};
typedef struct operand operand_t;

void lvm_reset(lvm_instance_t *p, unsigned char *code, lvm_ip_t size);
void lvm_clone(lvm_instance_t *dst, lvm_instance_t *src);
lvm_status_t lvm_derive(lvm_instance_t *p);
lvm_status_t lvm_get_derived_range(lvm_instance_t *p, char *name, 
                                   operand_value_t *min,
                                   operand_value_t *max);
void lvm_print_derivations(lvm_instance_t *p);
lvm_status_t lvm_execute(lvm_instance_t *p);
lvm_status_t lvm_register_variable(char *name, operand_type_t type);
lvm_status_t lvm_set_variable_value(char *name, operand_value_t value);
void lvm_print_code(lvm_instance_t *p);
lvm_ip_t lvm_jump_to_operand(lvm_instance_t *p);
lvm_ip_t lvm_shift_for_operator(lvm_instance_t *p, lvm_ip_t end);
lvm_ip_t lvm_get_end(lvm_instance_t *p);
lvm_ip_t lvm_set_end(lvm_instance_t *p, lvm_ip_t end);
void lvm_set_op(lvm_instance_t *p, operator_t op);
void lvm_set_relation(lvm_instance_t *p, operator_t op);
void lvm_set_operand(lvm_instance_t *p, operand_t *op);
void lvm_set_long(lvm_instance_t *p, long l);
void lvm_set_variable(lvm_instance_t *p, char *name);

#endif /* LVM_H */
