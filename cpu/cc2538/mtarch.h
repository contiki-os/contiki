/*
 * Copyright (c) 2014, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author: Andreas Dröscher <contiki@anticat.ch>
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
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
#ifndef MTARCH_H_
#define MTARCH_H_

#include "contiki.h"
#include "stdint.h"

#ifdef MTARCH_CONF_STACKSIZE
#define MTARCH_STACKSIZE MTARCH_CONF_STACKSIZE
#else
#define MTARCH_STACKSIZE 256
#endif

#define MTARCH_REGISTER_COUNT 16

/**
 * Template of initial stack layout
 */
typedef struct {
  void *r4;
  void *r5;
  void *r6;
  void *r7;
  void *r8;
  void *r9;
  void *r10;
  void *r11;
  void *r0;   /**< Ptr to argument (*data) */
  void *r1;
  void *r2;
  void *r3;
  void *r12;
  void *lr;   /**< Ptr to cleanup function */
  void *pc;   /**< Ptr to function (*function) */
  void *psr;  /**< Reset value: 0x01000000 */
} mtarch_stack_t;

struct mtarch_thread {
  void *stack[MTARCH_STACKSIZE];
  void *sp;
};

#endif /* MTARCH_H_ */
