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
 * $Id: main.c,v 1.13 2008/11/09 12:30:32 adamdunkels Exp $
 */

/**
 * \file
 * This file implements the main function of the Contiki distributed
 * sensor network simulation environment.
 * \author Adam Dunkels <adam@sics.se>
 *
 * When starting, each sensor node is created as its own process. The
 * sensor node processes communicates with the starting process using
 * named pipes. These pipes carry messages such as data packets and
 * configuration and statistics information requests.
 */
#include "contiki-net.h"
#include "display.h"
#include "contiki-main.h"
#include "nodes.h"
#include "ether.h"
#include "node.h"

#include "net/ethernode.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <arpa/inet.h>

in_addr_t gwaddr, winifaddr;

void netsim_init(void);

static int main_process = 0;

/*---------------------------------------------------------------------------*/
static void
sigchld_handler(int sig)
{
  int status;
  pid_t pid;
  struct nodes_node *node;
  if(!main_process) {
    return;
  }
  
  pid = waitpid(-1, &status, WNOHANG);
  
  if(WIFSIGNALED(status) &&
     WTERMSIG(status) == SIGSEGV) {
    node = nodes_find_pid(pid);
    if(node == NULL) {
      printf("A Contiki node crashed, but it wasn't even started by the system. Something weird is going on!\n");
    } else {
      printf("Contiki node at (%d, %d) crashed - Segmentation fault\n",
	     node->x, node->y);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
idle(void)
{
  int events;

  do {
    ether_server_poll();
    display_tick();
    display_redraw();
    ether_tick();
    events = process_run();
    if(events > 0) {
      printf("events %d\n", events);
    }
  } while(events > 0);

}
/*---------------------------------------------------------------------------*/
static int
start_node(int x, int y, int b)
{
  pid_t pid;
  struct timeval tv;
  static unsigned short port = NODES_PORTBASE;
  
  pid = fork();
  
  if(pid == 0) {

    /* This is the sensor process. */
    main_process = 0;

    /* Make standard output unbuffered. */
    setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
    
    srand(getpid());

    tv.tv_sec = 0;
    tv.tv_usec = 1000 * (rand() % 1000);
    select(0, NULL, NULL, NULL, &tv);
    
    node_init(port - NODES_PORTBASE + 2, x, y, b);
    ethernode_init(port);

    contiki_main(b);
    
    /* NOTREACHED */
  }

  if(b) {
    nodes_base_node_port = port;
  }
  
  /*  printf("Adding sensor %d at (%d,%d)\n", pid, x, y);*/
  main_process = 1;
  nodes_add(pid, x, y, port, port - NODES_PORTBASE + 2);
  

  ++port;
  return port - NODES_PORTBASE + 1;
}
/*---------------------------------------------------------------------------*/
int
main_add_node(int x, int y)
{
  return start_node(x, y, 0);
}
/*---------------------------------------------------------------------------*/
void
main_add_base(int x, int y)
{
  start_node(x, y, 1);
}
/*---------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
#ifdef __CYGWIN__
  if(argc == 3 &&
     inet_addr(argv[1]) == INADDR_NONE &&
     inet_addr(argv[2]) == INADDR_NONE) {
    printf("usage: %s <ip addr of ethernet card to share> "
	   "<ip addr of netsim gateway>\n", argv[0]);
    exit(1);
  } else if(argc >= 2) {
    gwaddr = inet_addr(argv[2]);
    winifaddr = inet_addr(argv[1]);
  }
#endif /* __CYGWIN__ */

  /*  system("ifconfig tap0 inet 192.168.250.1");*/
  /*  system("route delete 172.16.0.0/16");
      system("route add 172.16.0.0/16 192.168.250.2");*/

  nodes_init();
  
  atexit(nodes_kill);
  atexit(ether_print_stats);

  netsim_init();
  
  ether_server_init();

#if 0
  while(1) {
    ether_server_poll();
    ether_tick();
    process_run();
    usleep(100);
  }
#endif /* 0 */

#ifdef __CYGWIN__
  if(argc > 1 && (strcmp(argv[1], "-q") ||
		  strcmp(argv[2], "-q") ||
		  strcmp(argv[3], "-q")) == 0) {
#else /* __CYGWIN__ */
  if(argc > 1 && strcmp(argv[1], "-q") == 0) {
#endif /* __CYGWIN__ */
    display_init(idle, 50, 0);
  } else {
    display_init(idle, 50, 1);
  }
  display_redraw();

  signal(SIGCHLD, sigchld_handler);
  
  display_run();
  
  return 0;

  argv = argv;
  argc = argc;
}
/*-----------------------------------------------------------------------------------*/
char *arg_alloc(char size) {return NULL;}
void  arg_init(void) {}
void  arg_free(char *arg) {}
/*-----------------------------------------------------------------------------------*/

char *shell_prompt_text = "sensor-router> ";

/*-----------------------------------------------------------------------------------*/
#include <sys/time.h>

static signed long drift = 0;

void
clock_delay(unsigned int num)
{
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100 * num;
  select(0, NULL, NULL, NULL, &tv);
}

void
clock_set_time(clock_time_t time, clock_time_t ftime)
{
  drift = time - node_time();
}

clock_time_t
clock_time(void)
{
  return drift + node_time();
}
/*-----------------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return node_seconds();
}
/*-----------------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  uip_ipaddr_t addr;

  uip_gethostaddr(&addr);

  printf("uIP log at %d.%d.%d.%d: %s\n", uip_ipaddr_to_quad(&addr), m);
  fflush(NULL);
}
void
configurator_send_config(uip_ipaddr_t *addr, unsigned long seconds)
{
  printf("Configurator: address %d.%d.%d.%d, seconds %lu\n",
	 uip_ipaddr_to_quad(addr), seconds);
}


void
system_log(char *m)
{
  printf("%s", m);
}
/*void tr1001_drv_set_slip_dump(int c)
{

}*/
