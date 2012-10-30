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
#include "net/ethernet-drv.h"

#include "net/ethernet.h"

struct {
  char                signature[4];
  struct uip_eth_addr ethernet_address;
  uint8_t             *buffer;
  uint16_t            buffer_size;
  void __fastcall__   (* init)(uint16_t reg);
  uint16_t            (* poll)(void);
  void __fastcall__   (* send)(uint16_t len);
  void                (* exit)(void);
} *module;

/*---------------------------------------------------------------------------*/
void CC_FASTCALL
ethernet_init(struct ethernet_config *config)
{
  static const char signature[4] = {0x65, 0x74, 0x68, 0x01};

#ifndef ETHERNET

  struct mod_ctrl module_control = {cfs_read};
  uint8_t byte;

  module_control.callerdata = cfs_open(config->name, CFS_READ);
  if(module_control.callerdata < 0) {
    log_message(config->name, ": File not found");
    error_exit();
  }

  byte = mod_load(&module_control);
  if(byte != MLOAD_OK) {
    log_message(config->name, byte == MLOAD_ERR_MEM? ": Out of memory":
						     ": No module");
    error_exit();
  }

  cfs_close(module_control.callerdata);
  module = module_control.module;

  for(byte = 0; byte < 4; ++byte) {
    if(module->signature[byte] != signature[byte]) {
      log_message(config->name, ": No ETH driver");
      error_exit();
    }
  }

#else /* !ETHERNET */

  extern void ETHERNET;

  module = &ETHERNET;

#endif /* !ETHERNET */

  module->buffer = uip_buf;
  module->buffer_size = UIP_BUFSIZE;
  module->init(config->addr);

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

#ifndef ETHERNET
  mod_free(module);
#endif /* !ETHERNET */
}
/*---------------------------------------------------------------------------*/
