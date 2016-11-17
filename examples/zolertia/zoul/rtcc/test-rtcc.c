/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 * \addtogroup remote-rtcc-test
 * @{
 *
 *  Example project to show the on-board RTCC configuration and operation
 *  Retrieves the current time and date from the system, then sets an alarm to
 *  trigger every TEST_ALARM_SECOND match, generating an interrupt event and
 *  printing the current time/date, toggling also the LEDs
 *
 * @{
 *
 * \file
 *         RE-Mote on-board RTCC test application
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Aitor Mejias <amejias@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rtcc.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD             60L
#define LOOP_INTERVAL           (CLOCK_SECOND * LOOP_PERIOD)
#define TEST_ALARM_SECOND       15
/*---------------------------------------------------------------------------*/
/* Enable to match a given second number every minute, else it will trigger an
 * interrupt every TEST_ALARM_SECOND
 */
#define TEST_ALARM_MATCH_MIN    0
/*---------------------------------------------------------------------------*/
PROCESS(test_remote_rtcc_process, "Test RTC driver process");
AUTOSTART_PROCESSES(&test_remote_rtcc_process);
/*---------------------------------------------------------------------------*/
static uint8_t rtc_buffer[sizeof(simple_td_map)];
static simple_td_map *simple_td = (simple_td_map *)rtc_buffer;
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
void
configure_new_alarm(void)
{
  if(rtcc_date_increment_seconds(simple_td, TEST_ALARM_SECOND) == AB08_ERROR) {
    printf("Fail: could not increment the next alarm date\n");
    return;
  }

  /* We use the RTCC_REPEAT_DAY as we want the RTCC to match the given date */
  if(rtcc_set_alarm_time_date(simple_td, RTCC_ALARM_ON, RTCC_REPEAT_DAY,
                              RTCC_TRIGGER_INT1) == AB08_ERROR) {
    printf("Fail: couldn't set the alarm\n");
    return;
  }

  printf("Alarm set to match: ");
  rtcc_print(RTCC_PRINT_ALARM_DEC);
}
/*---------------------------------------------------------------------------*/
void
rtcc_interrupt_callback(uint8_t value)
{
  printf("A RTCC interrupt just happened! time/date: ");
  rtcc_print(RTCC_PRINT_DATE_DEC);
  leds_toggle(LEDS_ALL);

#if !TEST_ALARM_MATCH_MIN
  configure_new_alarm();
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_rtcc_process, ev, data)
{
  PROCESS_BEGIN();

  printf("RE-Mote RTC test\n");

  /* Map interrupt callback handler */
  RTCC_REGISTER_INT1(rtcc_interrupt_callback);

  /* Wait a bit */
  etimer_set(&et, (CLOCK_SECOND * 2));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Retrieve the configured time and date, this doesn't overwrites the
   * mode and century values
   */
  if(rtcc_get_time_date(simple_td) == AB08_ERROR) {
    printf("Fail: Couldn't read time and date\n");
    PROCESS_EXIT();
  }

  /* ...or for visualization only, just print the date directly from the RTCC */
  printf("Configured time: ");
  rtcc_print(RTCC_PRINT_DATE_DEC);

#if TEST_ALARM_MATCH_MIN
  /* Configure the RTCC to trigger an alarm every TEST_ALARM_SECOND match */
  printf("Setting an alarm to tick every %u seconds match\n", TEST_ALARM_SECOND);

  simple_td->seconds = TEST_ALARM_SECOND;

  /* Notice the arguments, we want to trigger the alarm every time the clock
   * matches the seconds values, so the alarm would have to be repeated every
   * minute.  In case we would want to trigger the alarm on a specific time,
   * then we would want to set a daily repeat interval
   */
  if(rtcc_set_alarm_time_date(simple_td, RTCC_ALARM_ON, RTCC_REPEAT_MINUTE,
                              RTCC_TRIGGER_INT1) == AB08_ERROR) {
    printf("Fail: couldn't set the alarm\n");
    PROCESS_EXIT();
  }

#else
  /* Configure the RTCC to trigger an alarm every TEST_ALARM_SECOND tick */
  printf("Setting an alarm to tick every %u seconds\n", TEST_ALARM_SECOND);

  configure_new_alarm();
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
