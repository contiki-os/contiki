/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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

/**
 * \file
 *         A very simple Contiki application showing how to use the Contiki
 *         Multi-threading library
 * \author
 *         Oliver Schmidt <ol.sc@web.de>
 * \author for PC-6001 version
 *         Takahide Matsutsuka <markn@markn.org>
 */

#include <stdio.h>

#include "contiki.h"
#include "ctk.h"
#include "sys/mt.h"
#include "mtarch.h"

#define WIN_XSIZE	10
#define WIN_YSIZE	10

static char log[WIN_XSIZE * WIN_YSIZE];
static struct ctk_window window;
static struct ctk_label loglabel = {CTK_LABEL(0, 0, WIN_XSIZE, WIN_YSIZE, log)};
PROCESS(mt_process, "Multi-threading test");

static char buf[20];

void
println(char *str1)
{
  unsigned int i;
  
  for(i = 1; i < WIN_YSIZE; i++) {
    memcpy(&log[(i - 1) * WIN_XSIZE], &log[i * WIN_XSIZE], WIN_XSIZE);
  }
  memset(&log[(WIN_YSIZE - 1) * WIN_XSIZE], 0, WIN_XSIZE);

  strncpy(&log[(WIN_YSIZE - 1) * WIN_XSIZE], str1, WIN_XSIZE);

  CTK_WIDGET_REDRAW(&loglabel);
}
/*---------------------------------------------------------------------------*/
static void
thread_func(char *str, int len)
{
  println((char *) (str + len));
  mt_yield();

  if(len) {
    thread_func(str, len - 1);
    mt_yield();
  }

  println((char *) (str + len));
}
/*---------------------------------------------------------------------------*/
static void
thread_main(void *data)
{
  while(1) {
    thread_func((char *)data, 9);
  }
  mt_exit();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mt_process, ev, data)
{

  static struct etimer timer;
  static int toggle = 0;
  static struct mt_thread th1;
  static struct mt_thread th2;

  PROCESS_BEGIN();

  ctk_window_new(&window, WIN_XSIZE, WIN_YSIZE, "Multithread");
  CTK_WIDGET_ADD(&window, &loglabel);
  memset(log, 0, sizeof(log));
  ctk_window_open(&window);
  mt_init();
  mt_start(&th1, thread_main, "JIHGFEDCBA");
  mt_start(&th2, thread_main, "9876543210");

  etimer_set(&timer, CLOCK_SECOND / 2);
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER) {
      if(toggle) {
	mt_exec(&th1);
	toggle--;
      } else {
	mt_exec(&th2);
	toggle++;
      }
      etimer_set(&timer, CLOCK_SECOND / 2);
    } else if(ev == ctk_signal_window_close || ev == PROCESS_EVENT_EXIT) {
      ctk_window_close(&window);
      process_exit(&mt_process);
      LOADER_UNLOAD();
    }
  }
  
  mt_stop(&th1);
  mt_stop(&th2);
  mt_remove();

  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
