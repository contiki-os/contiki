/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: eeprom.c,v 1.1 2006/06/17 22:41:31 adamdunkels Exp $
 */
#include "dev/eeprom.h"
#include "node.h"
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static unsigned char eeprom[65536];

void
eeprom_write(eeprom_addr_t addr, unsigned char *buf, int size)
{
  int f;
  char name[400];

  snprintf(name, sizeof(name), "eeprom.%d.%d", node_x(), node_y());
  f = open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);
  lseek(f, addr, SEEK_SET);
  write(f, buf, size);
  close(f);
  
  printf("eeprom_write(addr 0x%02x, buf %p, size %d);\n", addr, buf, size);
  
  memcpy(&eeprom[addr], buf, size);
}
void
eeprom_read(eeprom_addr_t addr, unsigned char *buf, int size)
{
  /*  printf("eeprom_read(addr 0x%02x, buf %p, size %d);\n", addr, buf, size);*/
  memcpy(buf, &eeprom[addr], size);
}
