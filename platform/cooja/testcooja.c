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
 * $Id: testcooja.c,v 1.1 2007/03/22 10:04:47 fros4943 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "net/uip.h"
#include "lib/sensors.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "sys/log.h"
#include "node-id.h"

#define COOJA_PORT 1234

PROCESS(cooja_test_process, "Example process for COOJA");
AUTOSTART_PROCESSES(&cooja_test_process);

static struct uip_udp_conn *broadcast_conn;
/*---------------------------------------------------------------------*/
PROCESS_THREAD(cooja_test_process, ev, data)
{
	PROCESS_BEGIN();

	log_message("Example process started", "");
  
	broadcast_conn = udp_broadcast_new(COOJA_PORT , NULL);

	button_sensor.activate();

	while(1) {
		PROCESS_WAIT_EVENT();
		log_message("An event occured: ", "");

		if(ev == PROCESS_EVENT_EXIT) {
			log_message("shutting down\n", "");
			break;
		}

		if(ev == sensors_event && data == &button_sensor && button_sensor.value(0)) {
			log_message("the button is pressed, sending packet\n", "");

			tcpip_poll_udp(broadcast_conn);
			PROCESS_WAIT_UNTIL(ev == tcpip_event && uip_poll());
			uip_send("cooyah COOJA", 12);
		}
		
		if(ev == sensors_event && data == &button_sensor && !button_sensor.value(0)) {
			log_message("the button was released again, doing nothing\n", "");
		}

		if(ev == tcpip_event && uip_newdata()) {
			log_message("a packet was received, turning on leds\n", "");
			log_message("PACKET DATA: ", uip_appdata);
			leds_on(LEDS_ALL);
		}
	
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------*/
