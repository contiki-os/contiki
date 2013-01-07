/* This file demonstrates the interaction between two processes via events.
 *
 * - "Sensor process": Throws an event periodically. This can be for example a
 * 				       sensor value.
 * - "Print process" : Waits for events from "Sensor" and prints a message when
 *                     an event occurs (e.g. prints the sensor value)
 *
 * This example is derived from the contiki code examples for the WSN430
 * (SensTools - Inria: http://senstools.gforge.inria.fr/)
 *
 * Author: George Oikonomou <G.Oikonomou@lboro.ac.uk>
 */

#include "contiki.h"
//#include "dev/leds.h"
#include <limits.h>
#include <stdio.h>
#include "event-post.h"

/* This is our event type */
static process_event_t event_data_ready;
/*---------------------------------------------------------------------------*/
/* Declare the two processes here */
PROCESS(sensor_process, "Sensor process");
PROCESS(print_process, "Print process");

AUTOSTART_PROCESSES(&sensor_process, &print_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_process, ev, data)
{
  static struct etimer timer;
  static struct event_struct es;

  PROCESS_BEGIN();

  es.s_val = SHRT_MAX - 2;
  es.i_val = INT_MAX - 2;
  es.l_val = LONG_MAX - 2;
  es.ll_val = LONG_MAX - 2;
  es.u8_val = UCHAR_MAX - 2;
  es.u16_val = USHRT_MAX - 2;
  es.u32_val = ULONG_MAX - 2;

  event_data_ready = process_alloc_event();

  printf("Contiki allocated event ID %d.\r\n", event_data_ready);

  etimer_set(&timer, CLOCK_CONF_SECOND * 2);

  while(1) {
    printf("Sensor process: Wait for timer event...\r\n");

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Sensor Process: Incrementing values...\r\n");
    es.s_val++;
    es.i_val++;
    es.l_val++;
    es.ll_val++;
    es.u8_val++;
    es.u16_val++;
    es.u32_val++;

    printf("Sensor Process: Generating 'Data Ready' event.\r\n");
    process_post(&print_process, event_data_ready, &es);

    etimer_reset(&timer);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of "Print Process" */
PROCESS_THREAD(print_process, ev, data)
{

  struct event_struct *sd;

  PROCESS_BEGIN();

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

    sd = data;
    printf("Print Process - Data Ready:\r\n");
    printf("    s: %d\r\n", sd->s_val);
    printf("    i: %d\r\n", sd->i_val);
    printf("    l: %ld\r\n", sd->l_val);
    printf("   ll: %lld\r\n", sd->ll_val);
    printf("   u8: %u\r\n", sd->u8_val);
    printf("  u16: %u\r\n", sd->u16_val);
    printf("  u32: %lu\r\n", sd->u32_val);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
