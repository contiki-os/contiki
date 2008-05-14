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
 * $Id: nodes.c,v 1.8 2008/05/14 19:22:58 adamdunkels Exp $
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodes.h"


static int numnodes;

static struct nodes_node nodes[2000];

int nodes_base_node_port = 0;
/*---------------------------------------------------------------------------*/
void
nodes_init(void)
{
  numnodes = 0;
}
/*---------------------------------------------------------------------------*/
void
nodes_add(int pid, int x, int y, int port, int id)
{
  nodes[numnodes].pid = pid;
  nodes[numnodes].x = x;
  nodes[numnodes].y = y;
  nodes[numnodes].port = port;
  nodes[numnodes].leds = 0;
  nodes[numnodes].done = 0;
  nodes[numnodes].id = id;
  ++numnodes;
}
/*---------------------------------------------------------------------------*/
void
nodes_kill(void)
{
  int i;
  for(i = 0; i < numnodes; ++i) {
    kill(nodes[i].pid, SIGTERM);
  }
}
/*---------------------------------------------------------------------------*/
int
nodes_num(void)
{
  return numnodes;
}
/*---------------------------------------------------------------------------*/
struct nodes_node *
nodes_node(int num)
{
  if(num > numnodes) {
    fprintf(stderr, "nodes_node: request for %d > %d\n", num, numnodes);
    abort();
  }
  return &nodes[num];
}
/*---------------------------------------------------------------------------*/
static struct nodes_node *
find_node(int x, int y)
{
  int i;

  for(i = numnodes; i >= 0; --i) {
    if(nodes[i].x == x && nodes[i].y == y) {
      return &nodes[i];
    }
  }
  return &nodes[0];
}
/*---------------------------------------------------------------------------*/
void
nodes_set_leds(int x, int y, int leds)
{
  find_node(x, y)->leds = leds;
}
/*---------------------------------------------------------------------------*/
void
nodes_set_text(int x, int y, char *text)
{
  strncpy(find_node(x, y)->text, text, NODES_TEXTLEN);
}
/*---------------------------------------------------------------------------*/
void
nodes_set_radio_status(int x, int y, int radio_status)
{
  find_node(x, y)->radio_status = radio_status;
}
/*---------------------------------------------------------------------------*/
void
nodes_set_line(int x, int y, int linex, int liney)
{
  struct nodes_node *n;

  n = find_node(x, y);
  n->linex = linex;
  n->liney = liney;
}
/*---------------------------------------------------------------------------*/
struct nodes_node *
nodes_find_pid(pid_t pid)
{
  int i;
  printf("Nofodes %d\n", numnodes);
  for(i = 0; i < numnodes; ++i) {
    printf("%d == %d\n", pid, nodes[i].pid); fflush(NULL);
    if(nodes[i].pid == pid) {
      return &nodes[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
nodes_done(int id)
{
  int i;
  int num_done = 0;

  for(i = numnodes; i >= 0; --i) {
    if(nodes[i].id == id) {
      nodes[i].done = 1;
    }
    if(nodes[i].done != 0) {
      num_done++;
    }
  }

  if(num_done == numnodes) {
    exit(0);
  }
}
/*---------------------------------------------------------------------------*/
