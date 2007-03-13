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
 * $Id: ethernode-rime.c,v 1.1 2007/03/13 13:07:48 adamdunkels Exp $
 */

#include "contiki.h"

#include "ethernode.h"

#include "net/rime.h"

PROCESS(ethernode_rime_process, "Ethernode Rime driver");

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ethernode_rime_process, ev, data)
{
  PROCESS_BEGIN();

  printf("ethernode_rime_process\n");
  
  while(1) {
    process_poll(&ethernode_rime_process);
    PROCESS_WAIT_EVENT();
    
    /* Poll Ethernet device to see if there is a frame avaliable. */
    {
      u16_t len;

      rimebuf_clear();
      
      len = ethernode_poll(rimebuf_dataptr(), RIMEBUF_SIZE);

      if(len > 0) {

	rimebuf_set_datalen(len);
	
	printf("ethernode_rime_process: received len %d\n",
	       len);
	abc_input_packet();
      }
    }
  }
  PROCESS_END();
    
}
/*---------------------------------------------------------------------------*/
void
abc_arch_send(u8_t *buf, int len)
{
  ethernode_send_buf(buf, len);
}
/*---------------------------------------------------------------------------*/
