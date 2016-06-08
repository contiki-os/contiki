/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

#define BIT(n) (1UL << (n))

void halt(void) __attribute__((__noreturn__));

#define STRINGIFY(x) #x
/* The C preprocessor will not expand macro arguments that are converted to
 * strings in the macro body using the '#' operator.  The EXP_STRINGIFY macro
 * introduces an additional level of argument expansion for instances where
 * the developer wishes to convert the expanded argument to a string.
 */
#define EXP_STRINGIFY(x) STRINGIFY(x)

#define ALIGN(x, amt) \
  (((x) & ~((amt) - 1)) + ((((x) & ((amt) - 1)) == 0) ? 0 : (amt)))

/** Wrappers for the assembly 'out' instruction. */
void outb(uint16_t port, uint8_t val);
void outl(uint16_t port, uint32_t val);

/** Wrappers for the assembly 'in' instruction */
uint8_t inb(uint16_t port);
uint32_t inl(uint16_t port);

#endif /* HELPERS_H */
