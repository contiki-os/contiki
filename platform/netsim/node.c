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
 * $Id: node.c,v 1.10 2008/01/04 23:23:29 oliverschmidt Exp $
 */
#include "node.h"
#include "contiki.h"
#include "net/uip.h"
#include "net/rime.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

extern in_addr_t gwaddr;

static clock_time_t drift, timer;

struct node node;

static int fd;

static void init_node_log(void);

/*------------------------------------------------------------------------------*/
void
node_init(int id, int posx, int posy, int b)
{
  uip_ipaddr_t addr;
  
  node.id = id;
  node.x = posx;
  node.y = posy;
  /*  node.type = NODE_TYPE_NORMAL;*/

  if(b) {
#ifdef __CYGWIN__
    addr = *(uip_ipaddr_t *)&gwaddr;
#else /* __CYGWIN__ */
    uip_ipaddr(&addr, 192,168,1,2);
#endif /* __CYGWIN__ */
  } else {
    uip_ipaddr(&addr, 172,16,posx,posy);
  }
  uip_sethostaddr(&addr);
  
  {
    rimeaddr_t nodeaddr;
    
    nodeaddr.u8[0] = posx;
    nodeaddr.u8[1] = posy;
    rimeaddr_set_node_addr(&nodeaddr);
  }

  drift = rand() % 95726272;

  init_node_log();
}
/*------------------------------------------------------------------------------*/
#include <sys/time.h>

clock_time_t
node_time(void)
{
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
 
  return tv.tv_sec * 1000 + tv.tv_usec / 1000/* + drift*/;
}
/*------------------------------------------------------------------------------*/
unsigned long
node_seconds(void)
{
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
 
  return tv.tv_sec;
}
/*------------------------------------------------------------------------------*/
void
node_set_time(clock_time_t time)
{
  timer = time;
}
/*------------------------------------------------------------------------------*/
int
node_x(void)
{
  return node.x;
}
/*------------------------------------------------------------------------------*/
int
node_y(void)
{
  return node.y;
}
/*------------------------------------------------------------------------------*/
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

static void
init_node_log(void)
{
  fd = open("log", O_CREAT | O_WRONLY | O_APPEND, 0666);
}

void
node_log(const char *fmt, ...)
{
  va_list ap;
  char buf[4096];
  int len;

  len = sprintf(buf, "Node %d (%d, %d): ", node.id, node.x, node.y);
  va_start(ap, fmt);
  vsprintf(&buf[len], fmt, ap);
  write(fd, buf, strlen(buf));
  va_end(ap);
}
/*------------------------------------------------------------------------------*/
