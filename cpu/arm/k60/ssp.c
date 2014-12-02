/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

#if 1 || defined(__SSP__) || defined(__SSP_ALL__)

#include "K60.h"

void * __stack_chk_guard = 0;

void __stack_chk_guard_setup(void) {
  unsigned char *p;
  p = (unsigned char *) &__stack_chk_guard;

  p[0] = 0;
  p[1] = 0;
  p[2] = '\n';
  p[3] = 255;	// XXX: Random this
}

/*
 * Arrange so that the __stack_chk_guard_setup function is called during
 * early init.
 */
void __attribute__((section(".preinit_array"))) (*preinit__stack_chk_guard_setup[])(void) = {__stack_chk_guard_setup};

/*
 * This is called if the SSP checks fail, which means that the stack has been
 * corrupted.
 */
void __attribute__((noreturn)) __stack_chk_fail(void)
{
    DEBUGGER_BREAK(BREAK_MEMORY_CORRUPTION);
    while(1);
}
#endif /* defined(__SSP__) || defined(__SSP_ALL__) */
