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
 * $Id: uaodv-example.c,v 1.2 2006/08/21 15:32:29 fros4943 Exp $
 */

#include <stdio.h>
#include "contiki-net.h"
#include "net/uaodv.h"
#include "net/uaodv-rt.h"

#include "lib/sensors.h"
#include "sys/log.h"

#include "dev/button-sensor.h"
#include "dev/serial.h"

/*---------------------------------------------------------------------------*/
PROCESS(uaodv_example_process, "uAODV example");

AUTOSTART_PROCESSES(&uaodv_process, &uaodv_example_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(uaodv_example_process, ev, data)
{
  static uip_ipaddr_t addr;
  
  PROCESS_BEGIN();

  int ipA, ipB, ipC, ipD;
  char buf[200];
    
  button_sensor.activate();
  serial_init();

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor && button_sensor.value(0)) {
      uip_ipaddr(&addr, 10,10,0,1);
      log_message("Sending RREQ to (static) 10.10.0.1\n", "");
      uaodv_request_route_to(&addr);
    } else if(ev == serial_event_message) {
      sscanf(data, "SENDTO>%d.%d.%d.%d", &ipA, &ipB, &ipC, &ipD);
      sprintf(buf, "Sending RREQ to %d.%d.%d.%d .. \n", ipA, ipB, ipC, ipD);
      log_message(buf, "");
      uip_ipaddr(&addr, ipA, ipB, ipC, ipD);
      uaodv_request_route_to(&addr);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
