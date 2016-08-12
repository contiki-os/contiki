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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         A simple wget implementation
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */
/*---------------------------------------------------------------------------*/
#include "webclient.h"
#include "wget.h"
#include "dev/leds.h"
/*---------------------------------------------------------------------------*/
#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"
/*---------------------------------------------------------------------------*/
#define DEBUG_LEDS 0
#undef LEDS_ON
#undef LEDS_OFF

#if DEBUG_LEDS
#define LEDS_ON(led) leds_on(led)
#define LEDS_OFF(led) leds_off(led)
#else
#define LEDS_ON(led)
#define LEDS_OFF(led)
#endif /* DEBUG_LEDS */
/*---------------------------------------------------------------------------*/
static int fetch_running;

#define STATS ((DEBUG)&DEBUG_PRINT) && 1
#if STATS
static clock_time_t fetch_started;
static unsigned long fetch_counter;
#endif /* STATUS */

static const char *server;
static const char *file;
static uint16_t port;
static const struct wget_callbacks *callbacks;
/*---------------------------------------------------------------------------*/
PROCESS(wget_process, "wget");
/*---------------------------------------------------------------------------*/
static void
call_done(int status)
{
  if(callbacks != NULL && callbacks->done != NULL) {
    callbacks->done(status);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wget_process, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("wget: fetching %s\n", file);
#if STATS
  fetch_counter = 0;
  fetch_started = clock_time();
#endif /* STATS */
  LEDS_ON(LEDS_YELLOW);
  if(webclient_get(server, port, file) == 0) {
    PRINTF("wget: failed to connect\n");
    LEDS_OFF(LEDS_YELLOW);
    fetch_running = 0;
    call_done(WGET_CONNECT_FAILED);
  } else {
    while(fetch_running) {
      PROCESS_WAIT_EVENT();
      if(ev == tcpip_event) {
        webclient_appcall(data);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
webclient_datahandler(char *data, uint16_t len)
{
  if(len == 0) {
#if STATS
    clock_time_t elapsed;
    elapsed = clock_time() - fetch_started;
    PRINTF("wget: recv %lu bytes during %lu sec (",
           fetch_counter, (elapsed / CLOCK_SECOND));
#if CLOCK_SECOND == 128
    PRINTF("%lu.%02lus, ", (unsigned long)elapsed >> 7,
           (unsigned long)((elapsed & 127) * 100) / 128);
    PRINTF("%lu byte/sec ", (fetch_counter * 128L) / elapsed);
#endif
    PRINTF("%lu tick): ", (unsigned long)elapsed);
    if(elapsed > CLOCK_SECOND) {
      PRINTF("%lu", fetch_counter / (elapsed / CLOCK_SECOND));
    } else {
      PRINTF("-");
    }
    PRINTF(" byte/sec\n");
#else /* STATS */
    PRINTF("wget: complete\n");
#endif /* STATS */

    fetch_running = 0;
    call_done(WGET_OK);
    LEDS_OFF(LEDS_RED | LEDS_YELLOW);
  } else {
#if STATS
    fetch_counter += len;
#endif /* STATS */
    if(callbacks != NULL && callbacks->data != NULL) {
      /* Assume the buffer is large enough for ending zero */
      data[len] = '\0';
      callbacks->data(data, len);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
webclient_connected(void)
{
  LEDS_OFF(LEDS_YELLOW);
  LEDS_ON(LEDS_RED);
}
/*---------------------------------------------------------------------------*/
void
webclient_timedout(void)
{
  PRINTF("wget: timedout\n");
  LEDS_OFF(LEDS_RED | LEDS_YELLOW);
  fetch_running = 0;
  call_done(WGET_TIMEDOUT);
}
/*---------------------------------------------------------------------------*/
void
webclient_aborted(void)
{
  PRINTF("wget: aborted\n");
  LEDS_OFF(LEDS_RED | LEDS_YELLOW);
  fetch_running = 0;
  call_done(WGET_ABORTED);
}
/*---------------------------------------------------------------------------*/
void
webclient_closed(void)
{
  PRINTF("wget: closed\n");
  fetch_running = 0;
  LEDS_OFF(LEDS_RED | LEDS_YELLOW);
  call_done(WGET_CLOSED);
}
/*---------------------------------------------------------------------------*/
void
wget_init(void)
{
  webclient_init();
}
/*---------------------------------------------------------------------------*/
int
wget_get(const char *s, uint16_t p, const char *f,
         const struct wget_callbacks *c)
{
  if(fetch_running) {
    return WGET_ALREADY_RUNNING;
  }
  fetch_running = 1;
  server = s;
  port = p;
  file = f;
  callbacks = c;
  process_start(&wget_process, NULL);
  return WGET_OK;
}
/*---------------------------------------------------------------------------*/
