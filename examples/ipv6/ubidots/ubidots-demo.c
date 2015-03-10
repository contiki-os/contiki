/*
 * Copyright (c) 2015, George Oikonomou - <george@contiki-os.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *    Example demonstrating how to use the Ubidots service
 *
 * \author
 *    George Oikonomou - <george@contiki-os.org>,
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/process.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "ubidots.h"

#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/* Sanity check */
#if !defined(UBIDOTS_DEMO_CONF_UPTIME) || !defined(UBIDOTS_DEMO_CONF_SEQUENCE)
#error "UBIDOTS_DEMO_CONF_UPTIME or UBIDOTS_DEMO_CONF_SEQUENCE undefined."
#error "Make sure you have followed the steps in the README"
#endif
/*---------------------------------------------------------------------------*/
/* Our own process' name */
PROCESS(ubidots_demo_process, "Ubidots demo process");
AUTOSTART_PROCESSES(&ubidots_demo_process);
/*---------------------------------------------------------------------------*/
/* POST period */
#define POST_PERIOD (CLOCK_SECOND * 30)
static struct etimer et;
/*---------------------------------------------------------------------------*/
#define VARIABLE_BUF_LEN 16
static unsigned int sequence;

static char variable_buffer[VARIABLE_BUF_LEN];
/*---------------------------------------------------------------------------*/
/*
 * 'List' of HTTP reply headers that we want to be notified about.
 * Terminate with NULL
 */
static const char *headers[] = {
  "Vary",
  NULL
};
/*---------------------------------------------------------------------------*/
/*
 * An example of how to POST one more more values to the same variable. This
 * primarily shows how to use the value argument depending on whether you
 * want to send a JSON string, number or boolean.
 */
static void
post_sequence_number(void)
{
  if(ubidots_prepare_post(UBIDOTS_DEMO_CONF_SEQUENCE) == UBIDOTS_ERROR) {
    printf("post_variable: ubidots_prepare_post failed\n");
  }

  memset(variable_buffer, 0, VARIABLE_BUF_LEN);

  /*
   * Write your value to the buffer. The contents of the buffer will be used
   * verbatim as the value of the variable in your JSON string. So, if you
   * store a number in the buffer this will become a JSON number. If you
   * enclose the value in double quotes, this will essentially be a JSON string
   *
   * Some examples
   * To send your value as a JSON number:
   * snprintf(variable_buffer, VARIABLE_BUF_LEN, "%u", sequence);
   *
   * To send your value as a JSON string:
   * snprintf(variable_buffer, VARIABLE_BUF_LEN, "\"%u\"", sequence);
   *
   * To send a JSON boolean:
   * ubidots_enqueue_value(NULL, "true");
   */
  snprintf(variable_buffer, VARIABLE_BUF_LEN, "%u", sequence);

  /* Append the contents of the buffer to your HTTP POST's payload */
  if(ubidots_enqueue_value(NULL, variable_buffer) == UBIDOTS_ERROR) {
    printf("post_variable (string): ubidots_enqueue_value failed\n");
  }

  /*
   * You can make a series of calls to ubidots_enqueue_value() here, as long
   * as they all have NULL as the first argument. In doing so, you can send
   * multiple values for the same variable
   */
  if(ubidots_post() == UBIDOTS_ERROR) {
    printf("post_variable: ubidots_post failed\n");
  }
}
/*---------------------------------------------------------------------------*/
/*
 * An example of how to post a collection: multiple different variables in
 * a single HTTP POST using {"variable":k,"value":v} pairs
 */
static void
post_collection(void)
{
  if(ubidots_prepare_post(NULL) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }

  /* Encode and enqueue the uptime as a JSON number */
  memset(variable_buffer, 0, VARIABLE_BUF_LEN);
  snprintf(variable_buffer, VARIABLE_BUF_LEN, "%lu",
           (unsigned long)clock_seconds());

  if(ubidots_enqueue_value(UBIDOTS_DEMO_CONF_UPTIME, variable_buffer) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }

  /* And the sequence counter, again as a JSON number */
  memset(variable_buffer, 0, VARIABLE_BUF_LEN);
  snprintf(variable_buffer, VARIABLE_BUF_LEN, "%u", sequence);

  if(ubidots_enqueue_value(UBIDOTS_DEMO_CONF_SEQUENCE, variable_buffer) == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }

  if(ubidots_post() == UBIDOTS_ERROR) {
    printf("post_collection: ubidots_prepare_post failed\n");
  }
}
/*---------------------------------------------------------------------------*/
/*
 * This is how to process the HTTP reply from the Ubidots server. In a real
 * scenario, we may wish to do something useful here, e.g. to test whether
 * the POST succeeded.
 *
 * This function here simply prints the entire thing, demonstrating how to use
 * the engine's API.
 */
static void
print_reply(ubidots_reply_part_t *r)
{
  switch(r->type) {
  case UBIDOTS_REPLY_TYPE_HTTP_STATUS:
    printf("HTTP Status: %ld\n", *((long int *)r->content));
    break;
  case UBIDOTS_REPLY_TYPE_HTTP_HEADER:
    printf("H: '%s'\n", (char *)r->content);
    break;
  case UBIDOTS_REPLY_TYPE_PAYLOAD:
    printf("P: '%s'\n", (char *)r->content);
    break;
  default:
    printf("Unknown reply type\n");
    break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ubidots_demo_process, ev, data)
{
  PROCESS_BEGIN();

  ubidots_init(&ubidots_demo_process, headers);

  sequence = 0;

  while(1) {

    PROCESS_YIELD();

    if(ev == ubidots_event_established ||
       (ev == PROCESS_EVENT_TIMER && data == &et)) {
      leds_on(LEDS_GREEN);
      sequence++;

      if(sequence & 1) {
        post_sequence_number();
      } else {
        post_collection();
      }
    } else if(ev == ubidots_event_post_sent) {
      leds_off(LEDS_GREEN);
      etimer_set(&et, POST_PERIOD);
    } else if(ev == ubidots_event_post_reply_received) {
      print_reply((ubidots_reply_part_t *)data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
