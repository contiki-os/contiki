/*
 * Copyright (c) 2017, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#include <serial.h>
#include <stdlib.h>

#include "contiki-net.h"
#include "sys/log.h"
#include "lib/error.h"
#include "lib/config.h"

#include "dev/slip.h"

#if WITH_SLIP
/*---------------------------------------------------------------------------*/
void
slip_arch_init(unsigned long ubr)
{
  unsigned err;

  err = ser_install(STATIC_DRIVER);
  if(err == SER_ERR_OK) {
    err = ser_open((struct ser_params *)config.slip);
    if(err == SER_ERR_OK)
      atexit((void (*)(void))ser_close);
  }
  if(err != SER_ERR_OK) {
    err += '0';
    /* High byte of err serves as string termination. */
    log_message("Serial init error code: ", (char *)&err);
    error_exit();
  }

  tcpip_set_outputfunc(slip_send);
}
/*---------------------------------------------------------------------------*/
void
slip_arch_writeb(unsigned char c)
{
  while(ser_put(c) == SER_ERR_OVERFLOW)
    ;
}
/*---------------------------------------------------------------------------*/
void
slip_arch_poll(void)
{
  static unsigned char c;

  while(ser_get(&c) != SER_ERR_NO_DATA)
    slip_input_byte(c);
}
/*---------------------------------------------------------------------------*/
#endif /* WITH_SLIP */
