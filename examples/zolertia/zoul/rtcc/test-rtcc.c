/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup remote-examples
 * @{

 * \defgroup remote-rtcc-test RE-Mote on-board RTCC test application
 *
 *  Example project to show the on-board RTCC configuration and operation
 *  Retrieves the current time and date from the system, then sets an alarm to
 *  trigger every TEST_ALARM_SECOND match, generating an interrupt event and
 *  printing the current time/date, toggling also the LEDs
 *
 * @{
 *
 * \file
 *     RE-Mote on-board RTCC test application
 *
 * \author
 *
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
#ifndef DATE
#define DATE "Unknown"
#endif
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD         60L
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define TEST_ALARM_SECOND   30
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
rtcc_interrupt_callback(uint8_t value)
{
  printf("A RTCC interrupt just happened! time/date: ");
  rtcc_print(RTCC_PRINT_DATE_DEC);
  leds_toggle(LEDS_PURPLE);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_rtcc_process, ev, data)
{
  static char *next;

  PROCESS_BEGIN();

  /* Alternatively for test only, undefine DATE and define on your own as
   * #define DATE "07 06 12 15 16 00 00"
   * Also note that if you restart the node at a given time, it will use the
   * already defined DATE, so if you want to update the device date/time you
   * need to reflash the node
   */

  /* Get the system date in the following format: wd dd mm yy hh mm ss */
  printf("RE-Mote RTC test, system date: %s\n", DATE);

  /* Sanity check */
  if(strcmp("Unknown", DATE) == 0) {
    printf("Fail: could not retrieve date from system\n");
    PROCESS_EXIT();
  }

  /* Configure RTC and return structure with all parameters */
  rtcc_init();

  /* Map interrupt callback handler */
  RTCC_REGISTER_INT1(rtcc_interrupt_callback);

  /* Configure the RTC with the current values */
  simple_td->weekdays    = (uint8_t)strtol(DATE, &next, 10);
  simple_td->day         = (uint8_t)strtol(next, &next, 10);
  simple_td->months      = (uint8_t)strtol(next, &next, 10);
  simple_td->years       = (uint8_t)strtol(next, &next, 10);
  simple_td->hours       = (uint8_t)strtol(next, &next, 10);
  simple_td->minutes     = (uint8_t)strtol(next, &next, 10);
  simple_td->seconds     = (uint8_t)strtol(next, NULL, 10);

  /* Don't care about the milliseconds... */
  simple_td->miliseconds = 0;

  /* This example relies on 24h mode */
  simple_td->mode = RTCC_24H_MODE;

  /* And to simplify the configuration, it relies it will be executed in the
   * present century
   */
  simple_td->century = RTCC_CENTURY_20XX;

  /* Set the time and date */
  if(rtcc_set_time_date(simple_td) == AB08_ERROR) {
    printf("Fail: Time and date not configured\n");
    PROCESS_EXIT();
  }

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

  /* Configure the RTCC to trigger an alarm every TEST_ALARM_SECOND tick */
  printf("Setting an alarm to tick every minute matching %u seconds\n",
         TEST_ALARM_SECOND);
  simple_td->seconds = TEST_ALARM_SECOND;

  /* Notice the arguments, we want to trigger the alarm every time the clock
   * matches the seconds values, so the alarm would have to be repeated every
   * minute.  In case we would want to trigger the alarm on a specific time,
   * then we would want to set a daily repeat interval
   */
  if(rtcc_set_alarm_time_date(simple_td, RTCC_ALARM_ON,
                              RTCC_REPEAT_MINUTE) == AB08_ERROR) {
    printf("Fail: couldn't set the alarm\n");
    PROCESS_EXIT();
  }

  printf("Alarm set to match: ");
  rtcc_print(RTCC_PRINT_ALARM_DEC);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

