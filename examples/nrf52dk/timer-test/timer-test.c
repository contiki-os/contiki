/**
 * \file
 *         Tests related to clocks and timers
 *         This is based on clock_test.c from the original sensinode port
 *
 * \author
 *         Zach Shelby <zach@sensinode.com> (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net> (rtimer code)
 *         Wojciech Bober <wojciech.bober@nordicsemi.no> (nRF52 DK adaptation)
 *
 */

/**
 * \addtogroup nrf52dk-examples Demo projects for nRF52 DK
 * @{
 */
#include "contiki.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/leds.h"

#include "nrf_delay.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define TEST_CLOCK_DELAY_USEC 1
#define TEST_RTIMER           1
#define TEST_ETIMER           1
/*---------------------------------------------------------------------------*/
static struct etimer et;

#if TEST_CLOCK_DELAY_USEC
static rtimer_clock_t start_count, end_count, diff;
#endif

#if TEST_ETIMER
static clock_time_t count;
#endif

#if TEST_RTIMER
static struct rtimer rt;
static clock_time_t ct_now;
rtimer_clock_t rt_now, rt_until;

static volatile rtimer_clock_t rt_now_cb;
static volatile clock_time_t ct_cb;
#endif

static uint8_t i;
/*---------------------------------------------------------------------------*/
PROCESS(clock_test_process, "Clock test process");
AUTOSTART_PROCESSES(&clock_test_process);
/*---------------------------------------------------------------------------*/
#if TEST_RTIMER
void
rt_callback(struct rtimer *t, void *ptr)
{
  rt_now_cb = RTIMER_NOW();
  ct_cb = clock_time();
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(clock_test_process, ev, data)
{

  PROCESS_BEGIN();
  etimer_set(&et, 2 * CLOCK_SECOND);
  PROCESS_YIELD();

  printf("RTIMER_SECOND=%d CLOCK_SECOND=%d\n", RTIMER_SECOND, CLOCK_SECOND);

#if TEST_CLOCK_DELAY_USEC
  printf("=======================\n");
  printf("TEST clock_delay_usec()\n");
  printf("=======================\n");
  i = 1;
  while(i < 7) {
    start_count = RTIMER_NOW();
    clock_delay_usec(10000 * i);
    end_count = RTIMER_NOW();
    diff = end_count - start_count;
    printf("difference [usec]: %ld\n", 10000 * i - (diff*(1000000/RTIMER_SECOND)));
    i++;
  }
#endif

#if TEST_RTIMER
  printf("=======================\n");
  printf("TEST rtimer\n");
  printf("=======================\n");
  i = 0;
  while(i < 5) {
    etimer_set(&et, 2 * CLOCK_SECOND);
    rt_now = RTIMER_NOW();
    ct_now = clock_time();
    rt_until = rt_now + RTIMER_SECOND;
    printf("now [ticks]: %lu until[ticks]: %lu\n", rt_now, rt_until);
    if (rtimer_set(&rt, rt_until, 1, rt_callback, NULL) != RTIMER_OK) {
      printf("Error setting\n");
    }
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("rtimer difference [ticks]: %ld\n", RTIMER_SECOND - (rt_now_cb - rt_now));
    printf("clock difference [ticks]: %ld\n", CLOCK_SECOND - (ct_cb - ct_now));
    i++;
  }
#endif

#if TEST_ETIMER
  printf("=======================\n");
  printf("TEST etimer\n");
  printf("=======================\n");
  i = 0;
  while(i < 5) {
    etimer_set(&et, i*CLOCK_SECOND);
    count = clock_time();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
    printf("difference [ticks]: %lu\n", i*CLOCK_SECOND - (clock_time() - count));
    leds_toggle(LEDS_RED);
    i++;
  }
#endif

  printf("Done!\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
