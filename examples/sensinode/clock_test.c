/**
 * \file
 *         Tests related to clocks and timers
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#include "contiki.h"
#include "sys/clock.h"
#include "dev/bus.h"
#include "dev/leds.h"
#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(clock_test_process, "Clock test process");
AUTOSTART_PROCESSES(&clock_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(clock_test_process, ev, data)
{
  static struct etimer et;
  static clock_time_t count, start_count, end_count, diff;
  static unsigned long sec;
  static u8_t i;

  PROCESS_BEGIN();

  printf("Clock delay test (10 x (10,000xi) cycles):\n");
  i = 1;
  while(i < 6) {
	start_count = clock_time();
	clock_delay(10000*i);
	end_count = clock_time();
	diff = end_count-start_count;
	printf("Delayed %u = %u ticks = ~%u ms\n", 10000*i, diff, diff*8 );
    i++;
  }

  printf("Clock tick and etimer test (10 x 1s):\n");
  i = 0;
  while(i < 10) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

	count = clock_time();
	printf("%u ticks\n", count);

    leds_toggle(LEDS_RED);
    i++;
  }

  printf("Clock seconds test (10 x 5s):\n");
  i = 0;
  while(i < 10) {
    etimer_set(&et, 5 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

	sec = clock_seconds();
	printf("%u seconds\n", (u16_t) sec);

    leds_toggle(LEDS_GREEN);
    i++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
