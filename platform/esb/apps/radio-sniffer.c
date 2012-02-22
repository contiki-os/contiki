/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: radio-sniffer.c,v 1.3 2007/08/07 11:20:11 nifi Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "radio-sniffer.h"
#include "contiki-net.h"
#include "contiki-esb.h"
#include "net/hc.h"
#include "net/tcpdump.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(radio_sniffer_process, "Radio sniffer");
AUTOSTART_PROCESSES(&radio_sniffer_process);
/*---------------------------------------------------------------------------*/
static void
sniffer_callback(const struct radio_driver *driver)
{
  static char buf[40];
  static uint8_t packet[UIP_BUFSIZE];
  static int len;
  len = driver->read(packet, sizeof(packet));
  if(len > 0) {
    leds_blink();
    len = hc_inflate(packet, len);
    tcpdump_format(packet, len, buf, sizeof(buf));
    printf("radio-sniffer %d: packet length %d, %s\n", node_id, len, buf);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(radio_sniffer_process, ev, data)
{
  PROCESS_BEGIN();

  tr1001_init();
  tr1001_driver.set_receive_function(sniffer_callback);

  printf("Radio sniffer started.\n");

  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
init_net(void)
{
}
/*---------------------------------------------------------------------------*/
