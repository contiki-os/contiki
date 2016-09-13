/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 * \addtogroup remote-power-mgmt-revb-test
 * @{
 *
 * Test the RE-Mote's power management features, shutdown mode and battery
 * management.
 *
 * @{
 *
 * \author
 *         Aitor Mejias <amejias@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/process.h"
#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "lib/list.h"
#include "power-mgmt.h"
#include "rtcc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
/* RE-Mote revision B, low-power PIC version */
#define PM_EXPECTED_VERSION               0x20
/*---------------------------------------------------------------------------*/
#ifndef DATE
#define DATE "Unknown"
#endif
/*---------------------------------------------------------------------------*/
#define TEST_LEDS_FAIL                    leds_off(LEDS_ALL); \
                                          leds_on(LEDS_RED);  \
                                          PROCESS_EXIT();
/*---------------------------------------------------------------------------*/
#define TEST_ALARM_SECOND                 15
/*---------------------------------------------------------------------------*/
PROCESS(test_remote_pm, "RE-Mote rev.B Power Management Test");
AUTOSTART_PROCESSES(&test_remote_pm);
/*---------------------------------------------------------------------------*/
static uint8_t rtc_buffer[sizeof(simple_td_map)];
static simple_td_map *simple_td = (simple_td_map *)rtc_buffer;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_pm, ev, data)
{
  static uint8_t aux;
  static uint16_t voltage;
  static uint32_t cycles;
  static char *next;

  PROCESS_BEGIN();

  aux = 0;
  cycles = 0;

  /* Initialize the power management block and signal the low-power PIC */
  if(pm_enable() != PM_SUCCESS) {
    printf("PM: Failed to initialize\n");
    TEST_LEDS_FAIL;
  }

  printf("PM: enabled!\n");

  /* Retrieve the firmware version and check expected */
  if((pm_get_fw_ver(&aux) == PM_ERROR) ||
     (aux != PM_EXPECTED_VERSION)) {
    printf("PM: unexpected version 0x%02X\n", aux);
    TEST_LEDS_FAIL;
  }

  printf("PM: firmware version 0x%02X OK\n", aux);

  /* Read the battery voltage level */
  if(pm_get_voltage(&voltage) != PM_SUCCESS) {
    printf("PM: error retrieving voltage\n");
    TEST_LEDS_FAIL;
  }

  printf("PM: Voltage (raw) = %u\n", voltage);

  /* Note: if running the following test while the RE-Mote is powered over USB
   * will show the command execution, but it will not put the board in shutdown
   * mode.  If powering the RE-Mote with an external battery the shutdown mode
   * will operate as intended, and the RE-Mote will restart and run the tests
   * from the start after waking-up off the shutdown mode.
   *
   * The first test shows how to use the "soft" shutdown mode, being the low
   * power PIC the one counting cycles and restarting the system off shutdown
   * mode.
   *
   * Each restart cycle is tracked by the low-power PIC, we can use this value
   * to determine how many times we have entered shutdown mode, thus choosing
   * a specific configuration or behaviour.  For the next examples we are going
   * to trigger a "soft" mode each even number, and "hard" if odd.
   */

  cycles = pm_get_num_cycles();
  printf("PM: cycle number %lu\n", cycles);

  if((cycles % 2) == 0) {
    /* Set the timeout */
    if(pm_set_timeout(PM_SOFT_SHTDN_5_7_SEC) != PM_SUCCESS) {
      printf("PM: error setting timeout for soft shutdown mode\n");
      TEST_LEDS_FAIL;
    }

    printf("PM: Soft shutdown, timeout set to %lu\n", pm_get_timeout());

    leds_off(LEDS_ALL);
    leds_on(LEDS_PURPLE);

    /* Wait just enough to be able to check the LED result */
    etimer_set(&et, CLOCK_SECOND * 3);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Enter soft shut*/
    if(pm_shutdown_now(PM_SOFT_SLEEP_CONFIG) == PM_SUCCESS) {
      printf("PM: Good night!\n");
    } else {
      printf("PM: error shutting down the system!\n");
      TEST_LEDS_FAIL;
    }

    /* Exit and wait the next cycle */
    PROCESS_EXIT();
  }

  /* Configure the RTCC to schedule a "hard" restart of the shutdown mode,
   * waking up from a RTCC interrupt to the low-power PIC
   */
  printf("PM: System date: %s\n", DATE);
  if(strcmp("Unknown", DATE) == 0) {
    printf("PM: could not retrieve date from system\n");
    TEST_LEDS_FAIL;
  }

  /* Configure RTC and return structure with all parameters */
  rtcc_init();

  /* Configure the RTC with the current values */
  simple_td->weekdays = (uint8_t)strtol(DATE, &next, 10);
  simple_td->day = (uint8_t)strtol(next, &next, 10);
  simple_td->months = (uint8_t)strtol(next, &next, 10);
  simple_td->years = (uint8_t)strtol(next, &next, 10);
  simple_td->hours = (uint8_t)strtol(next, &next, 10);
  simple_td->minutes = (uint8_t)strtol(next, &next, 10);
  simple_td->seconds = (uint8_t)strtol(next, NULL, 10);

  simple_td->miliseconds = 0;
  simple_td->mode = RTCC_24H_MODE;
  simple_td->century = RTCC_CENTURY_20XX;

  if(rtcc_set_time_date(simple_td) == AB08_ERROR) {
    printf("PM: Time and date configuration failed\n");
    TEST_LEDS_FAIL;
  } else {
    if(rtcc_get_time_date(simple_td) == AB08_ERROR) {
      printf("PM: Couldn't read time and date\n");
      TEST_LEDS_FAIL;
    }
  }

  printf("PM: Configured time: ");
  rtcc_print(RTCC_PRINT_DATE_DEC);

  /* Configure the RTCC to trigger an alarm  tick */
  printf("\nPM: Setting an alarm to tick in %u seconds\n", TEST_ALARM_SECOND);
  if(rtcc_date_increment_seconds(simple_td, TEST_ALARM_SECOND) == AB08_ERROR) {
    printf("PM: could not increment the next alarm date\n");
    TEST_LEDS_FAIL;
  }

  /* Set the timeout to zero to avoid the PIC being awake while waiting for the
   * RTCC system to kick-in
   */
  if(pm_set_timeout(0x00) != PM_SUCCESS) {
    printf("PM: couldn't clear the shutdown period\n");
    TEST_LEDS_FAIL;
  }

  /* We use the RTCC_REPEAT_DAY as we want the RTCC to match the given date */
  if(rtcc_set_alarm_time_date(simple_td, RTCC_ALARM_ON, RTCC_REPEAT_DAY,
                              RTCC_TRIGGER_INT2) == AB08_ERROR) {
    printf("PM: couldn't set the alarm\n");
    TEST_LEDS_FAIL;
  }

  printf("PM: Alarm set to match: ");
  rtcc_print(RTCC_PRINT_ALARM_DEC);

  leds_off(LEDS_ALL);
  leds_on(LEDS_BLUE);

  etimer_set(&et, CLOCK_SECOND * 3);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  if(pm_shutdown_now(PM_HARD_SLEEP_CONFIG) == PM_SUCCESS) {
    printf("PM: good night!\n");
  } else {
    printf("PM: error shutting down the system!\n");
    TEST_LEDS_FAIL;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
* @}
* @}
*/
