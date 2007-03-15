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
 * $Id: simple-cc2420-rime.c,v 1.2 2007/03/15 22:11:06 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "simple-cc2420.h"
#include "contiki.h"
#include "net/rime.h"

/*---------------------------------------------------------------------------*/
static void
receiver(void)
{
  u8_t len;

  rimebuf_clear();
  
  len = simple_cc2420_read(rimebuf_dataptr(), RIMEBUF_SIZE);
  
  if(len > 0) {
    rimebuf_set_datalen(len);
    /*    printf("simple_cc2420_rime_process: received len %d\n",
	  len);*/
    rime_input();
  }
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_rime_init(void)
{
  simple_cc2420_set_receiver(receiver);
}
/*---------------------------------------------------------------------------*/
void
rime_driver_send(void)
{
  /*  printf("simple_cc2420_rime: sending %d bytes\n", rimebuf_totlen());*/
  simple_cc2420_send(rimebuf_hdrptr(), rimebuf_totlen());
}
/*---------------------------------------------------------------------------*/
