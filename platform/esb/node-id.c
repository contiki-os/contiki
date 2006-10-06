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
 * $Id: node-id.c,v 1.1 2006/10/06 07:47:02 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "node-id.h"
#include "contiki-conf.h"
#include "dev/eeprom.h"

unsigned short node_id = 0;
/*---------------------------------------------------------------------------*/
void
node_id_restore(void)
{
  unsigned char buf[2];
  eeprom_read(NODE_ID_EEPROM_OFFSET, buf, 2);
  if(buf[0] == 0xad &&
     buf[1] == 0xde) {
    eeprom_read(NODE_ID_EEPROM_OFFSET + 2, buf, 2);
    node_id = (buf[0] << 8) | buf[1];
  } else {
    node_id = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
node_id_burn(unsigned short id)
{
  unsigned char buf[2];
  buf[0] = 0xad;
  buf[1] = 0xde;
  eeprom_write(NODE_ID_EEPROM_OFFSET, buf, 2);
  buf[0] = id >> 8;
  buf[1] = id & 0xff;
  eeprom_write(NODE_ID_EEPROM_OFFSET + 2, buf, 2);
}
/*---------------------------------------------------------------------------*/
