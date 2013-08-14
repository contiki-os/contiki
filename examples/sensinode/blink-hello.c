/* This is a very simple hello_world program.
 * It aims to demonstrate the co-existence of two processes:
 * One of them prints a hello world message and the other blinks the LEDs
 *
 * It is largely based on hello_world in $(CONTIKI)/examples/sensinode
 *
 * Author: George Oikonomou <G.Oikonomou@lboro.ac.uk>
 */

#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");

AUTOSTART_PROCESSES(&hello_world_process, &blink_process);
/*---------------------------------------------------------------------------*/
/* Implementation of the first process */
PROCESS_THREAD(hello_world_process, ev, data)
{
  static struct etimer timer;
  static int count;

  PROCESS_BEGIN();

  etimer_set(&timer, CLOCK_CONF_SECOND * 4);
  count = 0;

  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      printf("Sensor says no... #%d\r\n", count);
      count++;

      etimer_reset(&timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of the second process */
PROCESS_THREAD(blink_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  while(1) {
    etimer_set(&timer, CLOCK_CONF_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Blink... (state %0.2X).\r\n", leds_get());
    leds_toggle(LEDS_GREEN);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
