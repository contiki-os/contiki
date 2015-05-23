/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "contiki-net.h"
#include "sys/cc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 80

static struct tcp_socket socket;

#define INPUTBUFSIZE 400
static uint8_t inputbuf[INPUTBUFSIZE];

#define OUTPUTBUFSIZE 400
static uint8_t outputbuf[OUTPUTBUFSIZE];

PROCESS(tcp_server_process, "TCP echo process");
AUTOSTART_PROCESSES(&tcp_server_process);
static uint8_t get_received;
static int bytes_to_send;
/*---------------------------------------------------------------------------*/
static int
input(struct tcp_socket *s, void *ptr,
      const uint8_t *inputptr, int inputdatalen)
{
  printf("input %d bytes '%s'\n", inputdatalen, inputptr);
  if(!get_received) {
    /* See if we have a full GET request in the buffer. */
    if(strncmp((char *)inputptr, "GET /", 5) == 0 &&
       atoi((char *)&inputptr[5]) != 0) {
      bytes_to_send = atoi((char *)&inputptr[5]);
      printf("bytes_to_send %d\n", bytes_to_send);
      return 0;
    }
    printf("inputptr '%.*s'\n", inputdatalen, inputptr);
    /* Return the number of data bytes we received, to keep them all
       in the buffer. */
    return inputdatalen;
  } else {
    /* Discard everything */
    return 0; /* all data consumed */
  }
}
/*---------------------------------------------------------------------------*/
static void
event(struct tcp_socket *s, void *ptr,
      tcp_socket_event_t ev)
{
  printf("event %d\n", ev);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcp_server_process, ev, data)
{
  PROCESS_BEGIN();

  tcp_socket_register(&socket, NULL,
               inputbuf, sizeof(inputbuf),
               outputbuf, sizeof(outputbuf),
               input, event);
  tcp_socket_listen(&socket, SERVER_PORT);

  printf("Listening on %d\n", SERVER_PORT);
  while(1) {
    PROCESS_PAUSE();

    if(bytes_to_send > 0) {
      /* Send header */
      printf("sending header\n");
      tcp_socket_send_str(&socket, "HTTP/1.0 200 ok\r\nServer: Contiki tcp-socket example\r\n\r\n");

      /* Send data */
      printf("sending data\n");
      while(bytes_to_send > 0) {
        PROCESS_PAUSE();
        int len, tosend;
        tosend = MIN(bytes_to_send, sizeof(outputbuf));
        len = tcp_socket_send(&socket, (uint8_t *)"", tosend);
        bytes_to_send -= len;
      }

      tcp_socket_close(&socket);
    }

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
