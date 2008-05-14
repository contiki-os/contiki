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
 * $Id: nodes.h,v 1.6 2008/05/14 19:22:58 adamdunkels Exp $
 */
#ifndef __NODES_H__
#define __NODES_H__

#include <sys/types.h>

#define NODES_TEXTLEN 10

void nodes_init(void);
void nodes_add(int pid, int x, int y, int port, int id);
void nodes_kill(void);
void nodes_set_leds(int x, int y, int leds);
void nodes_set_text(int x, int y, char *text);
void nodes_set_line(int x, int y, int linex, int liney);
void nodes_set_radio_status(int x, int y, int radio_status);

void nodes_done(int id);

int nodes_num(void);
struct nodes_node *nodes_node(int num);
struct nodes_node *nodes_find_pid(pid_t pid);

struct nodes_node {
  int pid;
  int id;
  int x, y;
  int port;
  int leds;
  int done;
  int linex, liney;
  int radio_status;
  char text[NODES_TEXTLEN];
};

#define NODES_PORTBASE 5000

extern int nodes_base_node_port;

#endif /* __NODES_H__ */
