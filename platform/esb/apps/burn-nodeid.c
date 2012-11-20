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
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/node-id.h"
#include "contiki.h"
#include "dev/esb-sensors.h"
#include "dev/rs232.h"

#include <stdio.h>

PROCESS(burn_process, "Burn node id");
AUTOSTART_PROCESSES(&burn_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(burn_process, ev, data)
{
  PROCESS_BEGIN();
#if NODEID
  printf("Burning node id %d\n", NODEID);
  node_id_burn(NODEID);
  node_id_restore();
  printf("Restored node id %d\n", node_id);
#else
#error "burn-nodeid must be compiled with nodeid=<the ID of the node>"
  node_id_restore();
  printf("Restored node id %d\n", node_id);
#endif
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void init_net(void) {}
void init_lowlevel(void) {esb_sensors_init(); esb_sensors_on(); rs232_init();}
