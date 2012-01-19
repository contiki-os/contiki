/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: node-id.c,v 1.2 2010/08/26 22:08:11 nifi Exp $
 */

/**
 * \file
 *         Utility to store a node id in the external flash
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "node-id.h"
#include "contiki-conf.h"
#include "dev/xmem.h"
#include <string.h>


unsigned short node_id = 0;
unsigned char node_mac[8];

uint8_t id[] = {0xab, 0xcd};

/*---------------------------------------------------------------------------*/
void
node_id_restore(void)
{
  uint8_t *infomem;
  infomem = (uint8_t *) 0x1800;
  uint8_t i;
  if(infomem[0] == id[0] && infomem[1] == id[1]) {
    for(i = 0; i < 8; i++) {
      node_mac[i] = infomem[2 + i];
    }
  } else {
    /* default address */
    node_mac[0] = 0x02;
    node_mac[1] = 0x012;
    node_mac[3] = 0x074;
    node_mac[4] = 0x000;
    node_mac[5] = 0x01;
    node_mac[6] = 0x02;
    node_mac[7] = 0x03;
  }
  node_id = node_mac[7] | (node_mac[6] << 2);
}
/*---------------------------------------------------------------------------*/
void
node_id_burn(unsigned short id)
{
  /* add this later */
}
/*---------------------------------------------------------------------------*/
