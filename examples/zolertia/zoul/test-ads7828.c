/*
 * Copyright (c) 2017, Indian Institute of Science
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
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/*
 * \addtogroup zoul-examples
 * @{
 * \defgroup zoul-ads7828-test Test TI ADS7828
 *
 * The example application shows how to extend 8 SAR ADC channels using TI ADS7828
 * on Remote and read sensor data using Remote I2C interface.
 * Datasheet: http://www.ti.com/lit/ds/symlink/ads7828.pdf
 *
 * @{
 *
 * \file
 *  Example demonstrating TI ADS7828 12-bit, 8-channel sampling ADC with
 *  Remote i2c interface
 *
 * \author
 *         Sumankumar Panchal <suman@ece.iisc.ernet.in>
 *         Akshay P M <akshaypm@ece.iisc.ernet.in>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "dev/i2c.h"
#include "dev/ads7828.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD         1
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define LEDS_OFF_HYSTERISIS ((RTIMER_SECOND * LOOP_PERIOD) >> 1)

#define ADS7828_CMD_0 (ADS7828_CH0 | ADS7828_MODE_SE | ADS7828_MODE_INT_REF_ON_ADC_ON)
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(zoul_ads7828_demo_process, "Zoul External ADC process");
AUTOSTART_PROCESSES(&zoul_ads7828_demo_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(zoul_ads7828_demo_process, ev, data)
{

  PROCESS_BEGIN();

  uint16_t data;

  printf("TI ADS7828 test application\n");

  ads7828_init();
  etimer_set(&et, LOOP_INTERVAL);

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {

      if(ads7828_read_channel(ADS7828_CMD_0, &data) == ADS7828_SUCCESS) {
        printf("ADC=%d\n", data);
      }
      etimer_set(&et, LOOP_INTERVAL);
    }
  } /* while end */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
