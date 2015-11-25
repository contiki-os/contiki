/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
/*
 * Function implementation taken and adapted from:
 * cpu/stm32w108/e_stdio/src/syscalls.c
 */
/*---------------------------------------------------------------------------*/
#include <errno.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
extern int errno;
/*---------------------------------------------------------------------------*/
/* Register name faking - works in collusion with the linker.  */
register char *stack_ptr asm ("sp");
/*---------------------------------------------------------------------------*/
caddr_t
_sbrk(int incr)
{
  extern char end;  /* Defined by the linker */
  static char *heap_end;
  char *prev_heap_end;

  if(heap_end == 0) {
    heap_end = &end;
  }
  prev_heap_end = heap_end;
  if(heap_end + incr > stack_ptr) {
    _write(1, "Heap and stack collision\n", 25);
    /*abort ();*/
    errno = ENOMEM;
    return (caddr_t)-1;
  }

  heap_end += incr;
  return (caddr_t)prev_heap_end;
}
/*---------------------------------------------------------------------------*/
