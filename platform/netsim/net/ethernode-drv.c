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
 * $Id: ethernode-drv.c,v 1.4 2006/10/10 15:58:31 adamdunkels Exp $
 */

#include "contiki.h"

#include "ethernode.h"

#include "net/uip-fw.h"
#include "net/hc.h"
#include "net/tapdev.h"

#include "node-id.h"

PROCESS(ethernode_drv_process, "Ethernode driver");

enum { NULLEVENT };
/*---------------------------------------------------------------------------*/
u8_t
ethernode_drv_send(void)
{
  uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);

  return ethernode_send();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ethernode_drv_process, ev, data)
{
  static int drop = 3;
  PROCESS_BEGIN();

  while(1) {
    process_poll(&ethernode_drv_process);
    PROCESS_WAIT_EVENT();
    
    /* Poll Ethernet device to see if there is a frame avaliable. */
    uip_len = ethernode_poll();

    if(uip_len > 0) {
      /*      printf("%d: new packet len %d\n", node_id, uip_len);*/

      /*      if((random_rand() % drop) <= drop / 2) {
	printf("Bropp\n");
	} else*/ {

	uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);

	tapdev_send_raw();
	/*    if(uip_fw_forward() == UIP_FW_LOCAL)*/ {
	  /* A frame was avaliable (and is now read into the uip_buf), so
	     we process it. */
	  tcpip_input();
	}
      }
    }
  }
  PROCESS_END();
    
}
/*---------------------------------------------------------------------------*/
