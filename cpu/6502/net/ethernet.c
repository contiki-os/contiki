/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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

#include <modload.h>

#include "contiki-net.h"
#include "cfs/cfs.h"
#include "sys/log.h"
#include "lib/error.h"
#include "lib/config.h"

#include "net/ethernet.h"

struct {
  char                signature[4];
  struct uip_eth_addr ethernet_address;
  uint8_t             *buffer;
  uint16_t            buffer_size;
  char                jmp_init;
  char __fastcall__   (* init)(uint16_t reg);
  char                jmp_poll;
  uint16_t            (* poll)(void);
  char                jmp_send;
  void __fastcall__   (* send)(uint16_t len);
  char                jmp_exit;
  void                (* exit)(void);
} *module;

/*---------------------------------------------------------------------------*/
void
ethernet_init(void)
{
  static const char signature[4] = {0x65, 0x74, 0x68, 0x01};

#ifdef STATIC_DRIVER

  extern void STATIC_DRIVER;

  module = &STATIC_DRIVER;

  module->buffer = uip_buf;
  module->buffer_size = UIP_BUFSIZE;
  if(module->init(config.ethernet.addr)) {
    #define _stringize(arg) #arg
    #define  stringize(arg) _stringize(arg)
    log_message(stringize(STATIC_DRIVER), ": No hardware");
    #undef  _stringize
    #undef   stringize
    error_exit();
  }

#else /* STATIC_DRIVER */

  struct mod_ctrl module_control = {cfs_read};
  uint8_t byte;

  module_control.callerdata = cfs_open(config.ethernet.name, CFS_READ);
  if(module_control.callerdata < 0) {
    log_message(config.ethernet.name, ": File not found");
    error_exit();
  }

  byte = mod_load(&module_control);
  if(byte != MLOAD_OK) {
    log_message(config.ethernet.name, byte == MLOAD_ERR_MEM? ": Out of memory":
                                                             ": No module");
    error_exit();
  }

  cfs_close(module_control.callerdata);
  module = module_control.module;

  for(byte = 0; byte < 4; ++byte) {
    if(module->signature[byte] != signature[byte]) {
      log_message(config.ethernet.name, ": No ETH driver");
      error_exit();
    }
  }

  module->buffer = uip_buf;
  module->buffer_size = UIP_BUFSIZE;
  if(module->init(config.ethernet.addr)) {
    log_message(config.ethernet.name, ": No hardware");
    error_exit();
  }

#endif /* STATIC_DRIVER */

  uip_setethaddr(module->ethernet_address);
}
/*---------------------------------------------------------------------------*/
uint16_t
ethernet_poll(void)
{
  return module->poll();
}
/*---------------------------------------------------------------------------*/
void
ethernet_send(void)
{
  module->send(uip_len);
}
/*---------------------------------------------------------------------------*/
void
ethernet_exit(void)
{
  module->exit();

#ifndef STATIC_DRIVER
  mod_free(module);
#endif /* !STATIC_DRIVER */
}
/*---------------------------------------------------------------------------*/
