/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: websense-remote.c,v 1.2 2010/06/14 14:12:43 nifi Exp $
 */

/**
 * \file
 *         A simple example using HTTP to control and be controlled
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "wget.h"
#include "httpd-simple.h"
#include <stdio.h>

/* The address of the server to register the services for this node */
#define SERVER       "[aaaa::1]"

/* This command registers two services (/0 and /1) to turn the leds on or off */
#define REGISTER_COMMAND "/r?p=0&d=Turn%20off%20leds&p=1&d=Turn%20on%20leds"

/* The address of the other node to control */
#define OTHER_NODE   "[aaaa::1]"

/* The commands to send to the other node */
#define SET_LEDS_ON  "/1"
#define SET_LEDS_OFF "/0"

PROCESS(websense_remote_process, "Websense Remote");
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
AUTOSTART_PROCESSES(&websense_remote_process,&webserver_nogui_process);

/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Contiki Websense Remote</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_command(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  if(s->filename[1] == '0') {
    /* Turn off leds */
    leds_off(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned off leds!");
  } else if(s->filename[1] == '1') {
    /* Turn on leds */
    leds_on(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned on leds!");
  } else {
    SEND_STRING(&s->sout, "Unknown command");
  }

  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return handle_command;
}
/*---------------------------------------------------------------------------*/
static void
wget_done(int status)
{
  switch(status) {
  case WGET_OK:
    printf("Command sent.\n");
    break;
  case WGET_TIMEDOUT:
    printf("Command timedout.\n");
    break;
  case WGET_ABORTED:
    printf("Connection aborted.\n");
    break;
  case WGET_CONNECT_FAILED:
    printf("Failed to connect.\n");
    break;
  }
}
static const struct wget_callbacks callbacks = {
  NULL, wget_done
};
static void
send_command(const char *server, const char *command)
{
  int ret;
  printf("Sending to [%s]: %s\n", server, command);
  ret = wget_get(server, 80, command, &callbacks);
  if(ret != WGET_OK) {
    if(ret == WGET_ALREADY_RUNNING) {
      printf("Waiting for previous command to finish.\n");
    } else {
      printf("Failed to send command: %u\n", ret);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(websense_remote_process, ev, data)
{
  static int mode;
  static struct etimer timer;

  PROCESS_BEGIN();

  mode = 0;
  wget_init();

  SENSORS_ACTIVATE(button_sensor);

  /* Give the node some time to join the network before registering
     the available services. */
  etimer_set(&timer, CLOCK_SECOND * 30);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == sensors_event && data == &button_sensor) {
      /* The button has been pressed. Send command to the other node. */
      if(mode) {
        /* Turn on leds */
        send_command(OTHER_NODE, SET_LEDS_OFF);
      } else {
        send_command(OTHER_NODE, SET_LEDS_ON);
      }
      /* Alternate between the two commands */
      mode = !mode;

    } else if(ev == PROCESS_EVENT_TIMER && etimer_expired(&timer)) {
      printf("Registering services\n");
      send_command(SERVER, REGISTER_COMMAND);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
