/**
 * \file
 *         Tests related to clocks and timers
 *
 *         This is clock_test.c plus a small addition by George Oikonomou
 *         (Loughborough University)in order to test the rtimer
 *
 * \author
 *         Zach Shelby <zach@sensinode.com> (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net> (rtimer code)
 *
 */

#include "contiki.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define TEST_CLOCK_DELAY     1
#define TEST_RTIMER          1
#define TEST_ETIMER          1
#define TEST_CLOCK_SECONDS   1
/*---------------------------------------------------------------------------*/
PROCESS(clock_test_process, "Clock test process");
AUTOSTART_PROCESSES(&clock_test_process);
/*---------------------------------------------------------------------------*/
#if TEST_RTIMER
void
rt_callback(struct rtimer *t, void *ptr) {
  printf("Task called at %u\n", RTIMER_NOW());
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(clock_test_process, ev, data)
{
  static struct etimer et;

#if TEST_CLOCK_DELAY
  static clock_time_t start_count, end_count, diff;
#endif
#if TEST_CLOCK_SECONDS
  static unsigned long sec;
#endif
#if TEST_ETIMER
  static clock_time_t count;
#endif
#if TEST_RTIMER
  uint16_t rt_now, rt_for;
  static struct rtimer rt;
#endif
  static uint8_t i;

  PROCESS_BEGIN();

#if TEST_CLOCK_DELAY
  printf("Clock delay test (10 x (10,000xi) cycles):\n");
  i = 1;
  while(i < 6) {
    start_count = clock_time();
    clock_delay(10000 * i);
    end_count = clock_time();
    diff = end_count - start_count;
    printf("Delayed %u = %u ticks = ~%u ms\n", 10000 * i, diff, diff * 8);
    i++;
  }
#endif

#if TEST_RTIMER
  printf("Rtimer Test (10 x 1s):\n");
  i = 0;
  while(i < 10) {
    etimer_set(&et, 2*CLOCK_SECOND);
    puts("=======================");
    rt_now = RTIMER_NOW();
    rt_for = rt_now + RTIMER_SECOND;
    printf("%Now=%u - For=%u\n", rt_now, rt_for);
    if (rtimer_set(&rt, rt_for, 1,
              (void (*)(struct rtimer *, void *))rt_callback, NULL) != RTIMER_OK) {
      printf("Error setting\n");
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    i++;
  }
#endif

#if TEST_ETIMER
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
#endif

#if TEST_CLOCK_SECONDS
  printf("Clock seconds test (10 x 5s):\n");
  i = 0;
  while(i < 10) {
    etimer_set(&et, 5 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    sec = clock_seconds();
    printf("%u seconds\n", (uint16_t) sec);

    leds_toggle(LEDS_GREEN);
    i++;
  }
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
