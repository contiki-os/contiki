/*
 * Copyright (c) 2011, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *	A small command-line interface for the querying remote database systems.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "dev/serial-line.h"
#include "dev/sht11/sht11.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/mesh.h"

#include "antelope.h"
/*---------------------------------------------------------------------------*/
/* Sampling interval in Hz. */
#ifndef SAMPLING_INTERVAL
#define SAMPLING_INTERVAL 60
#endif

#ifndef RESPONSE_LIMIT
#define RESPONSE_LIMIT 1000
#endif

#ifndef PREPARE_DB 
#define PREPARE_DB 1
#endif

#ifndef CARDINALITY
#define CARDINALITY 1000
#endif

#define MAX_BUFFER_SIZE 80

#define NETDB_CHANNEL 70
/*---------------------------------------------------------------------------*/
PROCESS(netdb_process, "NetDB");
AUTOSTART_PROCESSES(&netdb_process);

static struct mesh_conn mesh;
static linkaddr_t reply_addr;
static uint8_t buffer_offset;
static char buffer[MAX_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static void
send_buffered_data(void)
{
  if(buffer_offset > 0) {
    packetbuf_copyfrom(buffer, buffer_offset);
    mesh_send(&mesh, &reply_addr);
    buffer_offset = 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
buffer_db_data(const char *format, ...)
{
  va_list ap;
  size_t len;
  char tmp[MAX_BUFFER_SIZE + 1];

  va_start(ap, format);
  len = vsnprintf(tmp, sizeof(tmp), format, ap);
  va_end(ap);

  if(len < 0) {
    return -1;
  }

  if(len + buffer_offset > sizeof(buffer)) {
    send_buffered_data();
  }

  memcpy(&buffer[buffer_offset], tmp, len);
  buffer_offset += len;

  return len;
}
/*---------------------------------------------------------------------------*/
#if !PREPARE_DB
static void
take_sample(void)
{
  unsigned seconds;
  unsigned humidity;

  seconds = clock_seconds();
  humidity = /*sht11_humidity()*/ random_rand();
  if(DB_ERROR(db_query(NULL, "INSERT (%u, %u) INTO samples;",
		       seconds, humidity))) {
    printf("DB insertion failed\n");
  }
}
#endif /* !PREPARE_DB */
/*---------------------------------------------------------------------------*/
static void
stop_handler(void *ptr)
{
  printf("END\n");
}
/*---------------------------------------------------------------------------*/
PROCESS(query_process, "Query process");

PROCESS_THREAD(query_process, ev, data)
{
  static db_handle_t handle;
  db_result_t result;
  static tuple_id_t matching;
  static tuple_id_t processed;
#if !PREPARE_DB
  static struct etimer sampling_timer;
#endif
  static unsigned i, errors;

  PROCESS_BEGIN();

  printf("NetDB host\n");

  db_init();
  db_set_output_function(buffer_db_data);

  db_query(NULL, "REMOVE RELATION samples;");
  db_query(NULL, "CREATE RELATION samples;");
  db_query(NULL, "CREATE ATTRIBUTE time DOMAIN INT IN samples;");
  db_query(NULL, "CREATE ATTRIBUTE hum DOMAIN INT IN samples;");
  db_query(NULL, "CREATE INDEX samples.time TYPE INLINE;");

#if PREPARE_DB
  printf("Preparing the DB with %d tuples...\n", CARDINALITY);
  errors = 0;
  for(i = 1; i <= CARDINALITY; i++) {
    PROCESS_PAUSE();

    result = db_query(NULL, "INSERT (%u, %u) INTO samples;",
                      i, (unsigned)random_rand());
    if(DB_ERROR(result)) {
      errors++;
    }
  }
  printf("Done. Insertion errors: %d\n", errors);
  printf("Ready to process queries\n");
#else
  etimer_set(&sampling_timer, SAMPLING_INTERVAL * CLOCK_SECOND);
#endif

  for(;;) {
    PROCESS_WAIT_EVENT();

    if(ev == serial_line_event_message && data != NULL) {
      printf("START %s\n", (char *)data);
      result = db_query(&handle, data);
      if(DB_ERROR(result)) {
	buffer_db_data("Query error: %s\n", db_get_result_message(result));
        stop_handler(NULL);
	db_free(&handle);
	continue;
      }

      if(!db_processing(&handle)) {
	buffer_db_data("OK\n");
	send_buffered_data();
        stop_handler(NULL);
	continue;
      }

      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
			 PACKETBUF_ATTR_PACKET_TYPE_STREAM);

      db_print_header(&handle);

      matching = 0;
      processed = 0;

      while(db_processing(&handle)) {
	PROCESS_PAUSE();

        if(matching == RESPONSE_LIMIT) {
	    buffer_db_data("Response suppressed at %u tuples: limit reached\n",
                           RESPONSE_LIMIT);
            stop_handler(NULL);
            db_free(&handle);
            break;
        }

	result = db_process(&handle);
        if(result == DB_GOT_ROW) {
	  /* The processed tuple matched the condition in the query. */
	  matching++;
	  processed++;
	  db_print_tuple(&handle);
	} else if(result == DB_OK) {
	  /* A tuple was processed, but did not match the condition. */
	  processed++;
	  continue;
	} else {
	  if(result == DB_FINISHED) {
	    /* The processing has finished. Wait for a new command. */
	    buffer_db_data("[%ld tuples returned; %ld tuples processed]\n",
			   (long)matching, (long)processed);
	    buffer_db_data("OK\n");
	  } else if(DB_ERROR(result)) {
	    buffer_db_data("Processing error: %s\n",
			   db_get_result_message(result));      
	  }
          stop_handler(NULL);
	  db_free(&handle);
	}
      }
      send_buffered_data();
    }

#if !PREPARE_DB
    if(etimer_expired(&sampling_timer)) {
      take_sample();
      etimer_reset(&sampling_timer);
    }
#endif
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
sent(struct mesh_conn *c)
{
}

static void
timedout(struct mesh_conn *c)
{
  printf("packet time out\n");
}

static void
received(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  char *data;
  unsigned len;
  static char query[MAX_BUFFER_SIZE + 1];

  data = (char *)packetbuf_dataptr();
  len = packetbuf_datalen();

  if(len > MAX_BUFFER_SIZE) {
    buffer_db_data("Too long query: %d bytes\n", len);
    return;
  }

  memcpy(query, data, len);
  query[len] = '\0';  

  printf("Query received from %d.%d: %s (%d hops)\n",
         from->u8[0], from->u8[1], query, (int)hops);
  linkaddr_copy(&reply_addr, from);

  process_post(&query_process, serial_line_event_message, query);
}

static const struct mesh_callbacks callbacks = {received, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(netdb_process, ev, data)
{
  PROCESS_EXITHANDLER(mesh_close(&mesh));
  PROCESS_BEGIN();

  mesh_open(&mesh, NETDB_CHANNEL, &callbacks);
  process_start(&query_process, NULL);

  PROCESS_END();
}
