/**
 * \addtogroup stm32w-cpu
 *
 * @{
 */

/*
 * Copyright (c) 2010, STMicroelectronics.
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

#include <stdio.h>
#include "dev/uart1.h"


#include PLATFORM_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
 /* #include "uart.h" */

#ifdef __GNUC__
# define _LLIO_STDIN ((int) stdin)
# define _LLIO_STDOUT ((int) stdout)
# define _LLIO_STDERR ((int) stderr)
# define _LLIO_ERROR  (-1)
#else
# ifdef __ICCARM__
# include <yfuns.h>
# endif
#endif

#undef putchar
/*--------------------------------------------------------------------------*/
int
  __attribute__ ((weak)) putchar(int c)
{
  uart1_writeb(c);
  return c;
}
/*--------------------------------------------------------------------------*/
void
__io_putchar(char c)
{
  putchar(c);
}
/*--------------------------------------------------------------------------*/
size_t
_write(int handle, const unsigned char *buffer, size_t size)
{
  size_t nChars = 0;

  if(handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
    return _LLIO_ERROR;
  }
  if(buffer == 0) {
    /* This means that we should flush internal buffers. */
    /* spin until TX complete (TX is idle) */
    while((SC1_UARTSTAT & SC_UARTTXIDLE) != SC_UARTTXIDLE) {
    }
    return 0;
  }

  /* ensure port is configured for UART */
  if(SC1_MODE != SC1_MODE_UART) {
    return _LLIO_ERROR;
  }
  while(size--) {
    __io_putchar(*buffer++);
    ++nChars;
  }

  return nChars;
}

/*--------------------------------------------------------------------------*/
size_t
_read(int handle, unsigned char *buffer, size_t size)
{
  return 0;
}
/** @} */
