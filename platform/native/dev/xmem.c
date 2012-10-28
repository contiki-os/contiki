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
 */

#include "contiki-conf.h"
#include "dev/xmem.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define XMEM_SIZE 1024 * 1024

static unsigned char xmem[XMEM_SIZE];
/*---------------------------------------------------------------------------*/
int
xmem_pwrite(const void *buf, int size, unsigned long offset)
{
  /*  int f;
  char name[400];

  snprintf(name, sizeof(name), "xmem.%d.%d", node_x(), node_y());
  f = open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);
  lseek(f, addr, SEEK_SET);
  write(f, buf, size);
  close(f);*/
  
  /*  printf("xmem_write(offset 0x%02x, buf %p, size %l);\n", offset, buf, size);*/
  
  memcpy(&xmem[offset], buf, size);
  return size;
}
/*---------------------------------------------------------------------------*/
int
xmem_pread(void *buf, int size, unsigned long offset)
{
  /*  printf("xmem_read(addr 0x%02x, buf %p, size %d);\n", addr, buf, size);*/
  memcpy(buf, &xmem[offset], size);
  return size;
}
/*---------------------------------------------------------------------------*/
int
xmem_erase(long nbytes, unsigned long offset)
{
  /*  printf("xmem_read(addr 0x%02x, buf %p, size %d);\n", addr, buf, size);*/
  memset(&xmem[offset], 0, nbytes);
  return nbytes;
}
/*---------------------------------------------------------------------------*/
void
xmem_init(void)
{

}
/*---------------------------------------------------------------------------*/
