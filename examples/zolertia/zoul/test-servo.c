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
 *
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
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-servo-test Test the EMAX ES08A II servo motor
 *
 * Demonstrates the use of the EMAX ES08A servo motor.  This servo requires a
 * +5V voltage, it can be powered from D+5.1 pin (of the ADC3 connector), but
 * it requires either an external power supply other than the USB for programing
 * or it can be powered by the USB 2.0 connector, which allows a higher current
 * draw.
 *
 * This test uses the default servo values (freq 50Hz, traveling time 1.5-1.9ms)
 * for 0-180º movement, tested with the EMAX ES08A.  Depending on the servo used
 * you might need to adjust these parameters in the servo.h header file.
 *
 * @{
 *
 * \file
 *         A quick program for testing a servo motor
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/leds.h"
#include "dev/servo.h"
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(servo_test_process, "Zolertia servo test process");
AUTOSTART_PROCESSES(&servo_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(servo_test_process, ev, data)
{
  PROCESS_BEGIN();

  static uint8_t deg = 0;

  PRINTF("\nStarting the test\n");

  while(1) {

    servo_position(SERVO_CHANNEL_5, GPIO_A_NUM, 5, deg);
    PRINTF("Current position --> %03uº\n", deg);

    etimer_set(&et, CLOCK_SECOND / 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Increase the position by 10º each iteration */
    deg += 10;
    if(deg > SERVO_MAX_DEGREES) {
      deg = 0;
      servo_stop(SERVO_CHANNEL_5, GPIO_A_NUM, 5);

      /* Stop the servo and wait 2 seconds before resuming from start */
      etimer_set(&et, CLOCK_SECOND * 2);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

