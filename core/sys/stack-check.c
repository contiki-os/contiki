/*
 * Copyright (c) 2017, University of Bristol - http://www.bris.ac.uk/
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
 */

/**
 * \addtogroup stack
 * @{
 */

/**
 * \file
 *     Implementation of the stack checker library.
 * \author
 *     Atis Elsts <atis.elsts@bristol.ac.uk>
 */

#include "sys/cc.h"
#include "sys/stack-check.h"
#include "dev/watchdog.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
#if STACK_CHECK_ENABLED
/*---------------------------------------------------------------------------*/
/* linker will provide a symbol for the end of the .bss segment */
extern uint8_t _stack;
/* the approximate starting point of stack for platforms that don't define STACK_ORIGIN */
static void *stack_origin;
/*---------------------------------------------------------------------------*/
/* The symbol with which the stack memory is initially filled */
#define STACK_FILL 0xcd
/*---------------------------------------------------------------------------*/
#ifdef STACK_ORIGIN
/* use the #defined value */
#define GET_STACK_ORIGIN() STACK_ORIGIN
#else
/* use the approximate value */
#define GET_STACK_ORIGIN() stack_origin
#endif
/*---------------------------------------------------------------------------*/
void
stack_check_init(void)
{
  uint8_t *p;

  stack_origin = &p;
  p = &_stack;

  /* printf("eoh=%p eos=%p stack=%p\n", &_stack, GET_STACK_ORIGIN(), p); */

  /* Make sure WDT is not triggered */
  watchdog_periodic();

  while(p < (uint8_t *)stack_origin) {
    *p++ = STACK_FILL;
  }

  /* Make sure WDT is not triggered */
  watchdog_periodic();
}
/*---------------------------------------------------------------------------*/
uint16_t
stack_check_get_usage(void)
{
  uint8_t *p = &_stack;

  /* Make sure WDT is not triggered */
  watchdog_periodic();

  /* Skip the bytes used after heap; it's 1 byte by default for _stack,
   * more than that means dynamic memory allocation is used somewhere.
   */
  while(*p != STACK_FILL && p < (uint8_t *)GET_STACK_ORIGIN()) {
    p++;
  }

  /* Skip the region of the memory reserved for the stack not used yet by the program */
  while(*p == STACK_FILL && p < (uint8_t *)GET_STACK_ORIGIN()) {
    p++;
  }

  /* Make sure WDT is not triggered */
  watchdog_periodic();

  return (uint8_t *)GET_STACK_ORIGIN() - p;
}
/*---------------------------------------------------------------------------*/
uint16_t
stack_check_get_reserved_size(void)
{
  return (uint8_t *)GET_STACK_ORIGIN() - &_stack;
}
/*---------------------------------------------------------------------------*/
#endif /* STACK_CHECK_ENABLED */
/*---------------------------------------------------------------------------*/
/** @} */
