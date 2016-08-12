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

#include <sys/stat.h>
#include <errno.h>

#include "uart.h"
#include "helpers.h"

#define CONSOLE_OUTPUT_DEV QUARK_X1000_UART_1

#define HEAP_MAX_SIZE 2048

static char _heap[HEAP_MAX_SIZE];
static char *prog_break = _heap;

int
_close_r(struct _reent *ptr, int file)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return -1;
}
/*---------------------------------------------------------------------------*/
void
_exit(int status)
{
  halt();
}
/*---------------------------------------------------------------------------*/
int
_getpid_r(struct _reent *ptr)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return -1;
}
/*---------------------------------------------------------------------------*/
int
_isatty_r(struct _reent *ptr, int file)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return 0;
}
/*---------------------------------------------------------------------------*/
int
_kill_r(struct _reent *ptr, int pid, int signal)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return -1;
}
/*---------------------------------------------------------------------------*/
int
_read_r(struct _reent *ptr, int file, char *buf, int len)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return -1;
}
/*---------------------------------------------------------------------------*/
int
_write_r(struct _reent *ptr, int file, const char *buf, int len)
{
  int ret;
  int i;

  switch(file) {
  case 0:
    ptr->_errno = EBADF;
    ret = -1;
    break;

  case 1:
  case 2:
    for(i = 0; i < len; i++) {
      /* Since file descriptors 1 and 2 (stdout and stderr) are mapped to a
       * serial console, we should translate the 'newline' escape sequence
       * to 'carriage return' (CR) followed by 'line feed' (LF) ASCII
       * characters.
       */
      if(buf[i] == '\n') {
        quarkX1000_uart_tx(CONSOLE_OUTPUT_DEV, '\r');
      }
      quarkX1000_uart_tx(CONSOLE_OUTPUT_DEV, buf[i]);
    }

    ret = len;
    break;

  default:
    /* We don't support any filesystem yet. */
    ptr->_errno = ENOTSUP;
    ret = -1;
    break;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
int
_lseek_r(struct _reent *ptr, int file, int p, int dir)
{
  /* Stubbed function */
  ptr->_errno = ENOTSUP;
  return -1;
}
/*---------------------------------------------------------------------------*/
int
_fstat_r(struct _reent *ptr, int file, struct stat *st)
{
  /* We don't support the standard input yet so file descriptor 0 is not
   * supported by this function. Additionally, we don't have support for
   * any filesystem thus file descriptors greater than 2 are not supported
   * as well.
   *
   * We support standard ouput and error (file descriptors 1 and 2) only.
   */
  if(file == 0 || file > 2) {
    ptr->_errno = ENOTSUP;
    return -1;
  }

  st->st_mode = S_IFCHR;
  return 0;
}
/*---------------------------------------------------------------------------*/
caddr_t
_sbrk_r(struct _reent *ptr, int incr)
{
  char *prev_prog_break;

  /* If the new program break overruns the maximum heap address, we return
   * "Out of Memory" error to the user.
   */
  if(prog_break + incr > _heap + HEAP_MAX_SIZE) {
    ptr->_errno = ENOMEM;
    return NULL;
  }

  prev_prog_break = prog_break;

  prog_break += incr;

  return prev_prog_break;
}
