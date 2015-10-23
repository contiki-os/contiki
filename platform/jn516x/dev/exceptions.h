/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Thomas Haydon
 * Integrated into Contiki by Beshr Al Nahas
 *
 */

#ifndef  EXCEPTIONS_H
#define  EXCEPTIONS_H

#include <jendefs.h>

/** Enumerated type of CPU exception numbers */
typedef enum {
  E_EXC_BUS_ERROR = 0x02,
  E_EXC_TICK_TIMER = 0x05,
  E_EXC_UNALIGNED_ACCESS = 0x06,
  E_EXC_ILLEGAL_INSTRUCTION = 0x07,
  E_EXC_EXTERNAL_INTERRUPT = 0x08,
  E_EXC_SYSCALL = 0x0C,
  E_EXC_TRAP = 0x0E,
  E_EXC_GENERIC = 0x0F,
  E_EXC_STACK_OVERFLOW = 0x10
} eExceptionType;

/* Exceptions set up function */
PUBLIC void vEXC_Register(void);
/* For debugging */
void debug_file_line(const char *file, int line);

#endif /* EXCEPTIONS_H */
