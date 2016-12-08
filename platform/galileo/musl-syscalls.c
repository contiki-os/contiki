/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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

#include <assert.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include "helpers.h"
#include "uart.h"

#define CONSOLE_OUTPUT_DEV QUARK_X1000_UART_1

/*---------------------------------------------------------------------------*/
long
galileo_writev(int fd, struct iovec *iovs, int iovcnt)
{
  int i;
  long res = 0;

  assert((fd == STDOUT_FILENO) || (fd == STDERR_FILENO));

  for(i = 0; i < iovcnt; i++) {
    int j;
    for(j = 0; j < iovs[i].iov_len; j++) {
      char c = ((char *)iovs[i].iov_base)[j];
      if(c == '\n') {
        quarkX1000_uart_tx(CONSOLE_OUTPUT_DEV, '\r');
      }
      quarkX1000_uart_tx(CONSOLE_OUTPUT_DEV, c);
      res++;
    }
  }

  return res;
}
/*---------------------------------------------------------------------------*/
long
galileo_vsyscall(long syscall_id,
                 long arg0, long arg1, long arg2,
                 long arg3, long arg4, long arg5)
{
  long res = 0;

  switch(syscall_id) {
  case SYS_ioctl:
    if(arg1 == TIOCGWINSZ) {
      assert((arg0 == STDOUT_FILENO) || (arg0 != STDERR_FILENO));

      struct winsize *w_sz = (struct winsize *)arg2;
      w_sz->ws_row = 25;
      w_sz->ws_col = 80;
      res = 0;
    } else {
      fprintf(stderr, "Unimplemented IOCTL: %ld.\n", arg1);
      halt();
    }
    break;

  case SYS_writev:
    res = galileo_writev((int)arg0, (struct iovec *)arg1, (int)arg2);
    break;

  default:
    fprintf(stderr, "Unimplemented syscall: %ld.\n", syscall_id);
    halt();
  }

  return res;
}
/*---------------------------------------------------------------------------*/
void
libc_init(void)
{
  typedef long (vsyscall_t)(void);

  extern vsyscall_t galileo_vsyscall_asm;

  extern vsyscall_t *__sysinfo;

  __sysinfo = galileo_vsyscall_asm;
}
