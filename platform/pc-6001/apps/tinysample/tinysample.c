/*
 * Copyright (c) 2007, Takahide Matsutsuka.
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
 * $Id: tinysample.c,v 1.2 2007/11/28 09:37:11 matsutsuka Exp $
 */

/**
 * \file
 *         A very small sample Contiki application using console.
 * \author 
 *         Takahide Matsutsuka <markn@markn.org>
 */

#include "contiki.h"
#include "libconio_arch-small.h"

PROCESS(tiny_process, "Tiny Sample");

static char ch[] = "A";
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tiny_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  clrscr_arch();

  while(1) {
    etimer_set(&timer, CLOCK_SECOND / 32);
    PROCESS_WAIT_EVENT();
    if(etimer_expired(&timer)) {
      libputs_arch(ch);
      ch[0]++;
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
