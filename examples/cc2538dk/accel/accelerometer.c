/*
 * Copyright (c) 2013, Loughborough University.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 * \addtogroup cc2538-examples
 * @{
 *
 *
 * \defgroup cc2538-spi cc2538dk accelerometer Project
 *
 *    This example tests the correct functionality of RF06 accelerometer
 *
 * @{
 *
 * \file
 *         Tests related accelerometer
 *
 *
 * \author
 *         Vasilis Michopoulos <basilismicho@gmail.com>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/accel-sensor.h"
#include "dev/leds.h"

/*---------------------------------------------------------------------------*/

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"

/*---------------------------------------------------------------------------*/
PROCESS(accelerometer_process, "accelerometer process");
AUTOSTART_PROCESSES(&accelerometer_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(accelerometer_process, ev, data) {

  static struct etimer et;
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND);
  accel_sensor.configure(SENSORS_HW_INIT, 1);

  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      etimer_restart(&et);

      PRINTF("(X,Y,Z): (%d,%d,%d)\n", accel_sensor.value(ACC_X_AXIS),
             accel_sensor.value(ACC_Y_AXIS),
             accel_sensor.value(ACC_Z_AXIS));

      PRINTF("tmp: (%x)\n", accel_sensor.value(ACC_TMP));

      leds_toggle(LEDS_RED);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
