/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
/** \addtogroup cc2538-char-io
 * @{ */
/**
 * \file
 *     Implementation of arch-specific functions required by the dbg_io API in
 *     cpu/arm/common/dbg-io
 */
#include "contiki.h"

#include "dbg.h"
#include "dev/uart.h"
#include "usb/usb-serial.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#ifndef DBG_CONF_USB
#define DBG_CONF_USB 0
#endif

#if DBG_CONF_USB
#define write_byte(b) usb_serial_writeb(b)
#define flush()       usb_serial_flush()
#else
#define write_byte(b) uart_write_byte(DBG_CONF_UART, b)
#define flush()
#endif
/*---------------------------------------------------------------------------*/
#undef putchar
#undef puts

#define SLIP_END     0300
/*---------------------------------------------------------------------------*/
int
putchar(int c)
{
#if DBG_CONF_SLIP_MUX
  static char debug_frame = 0;

  if(!debug_frame) {
    write_byte(SLIP_END);
    write_byte('\r');
    debug_frame = 1;
  }
#endif

  write_byte(c);

  if(c == '\n') {
#if DBG_CONF_SLIP_MUX
    write_byte(SLIP_END);
    debug_frame = 0;
#endif
    dbg_flush();
  }
  return c;
}
/*---------------------------------------------------------------------------*/
unsigned int
dbg_send_bytes(const unsigned char *s, unsigned int len)
{
  unsigned int i = 0;

  while(s && *s != 0) {
    if(i >= len) {
      break;
    }
    putchar(*s++);
    i++;
  }
  return i;
}
/*---------------------------------------------------------------------------*/
int
puts(const char *s)
{
  unsigned int i = 0;

  while(s && *s != 0) {
    putchar(*s++);
    i++;
  }
  putchar('\n');
  return i;
}
/*---------------------------------------------------------------------------*/

/** @} */
