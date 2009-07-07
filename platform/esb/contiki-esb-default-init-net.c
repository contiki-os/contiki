/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: contiki-esb-default-init-net.c,v 1.13 2009/07/07 13:06:56 nifi Exp $
 */

#include "contiki-conf.h"
#include "dev/tr1001.h"
#include "dev/rs232.h"
#include "dev/serial-line.h"
#include "net/rime.h"
#include "node-id.h"
#include <stdio.h>

#ifndef MAC_DRIVER
#ifdef MAC_CONF_DRIVER
#define MAC_DRIVER MAC_CONF_DRIVER
#else
#define MAC_DRIVER nullmac_driver
#endif /* MAC_CONF_DRIVER */
#endif /* MAC_DRIVER */

extern const struct mac_driver MAC_DRIVER;

void
init_net(void)
{
  rimeaddr_t rimeaddr;
  int i;

  tr1001_init();
  rime_init(MAC_DRIVER.init(&tr1001_driver));
  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);

  printf("Rime started with address ");
  for(i = 0; i < sizeof(rimeaddr.u8) - 1; i++) {
    printf("%u.", rimeaddr.u8[i]);
  }
  printf("%u (%s)\n", rimeaddr.u8[i], MAC_DRIVER.name);

  rs232_set_input(serial_line_input_byte);
  serial_line_init();
}
