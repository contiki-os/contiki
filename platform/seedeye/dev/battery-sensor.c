/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 
/**
 * \addtogroup SeedEye Contiki SEEDEYE Platform
 *
 * @{
 */

/**
 * \file   battery-sensor.c
 * \brief  Battery Sensor
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-07-04
 */

#include <dev/battery-sensor.h>

#include <p32xxxx.h>

/*---------------------------------------------------------------------------*/
PROCESS(battery_process, "battery sensor");
/*---------------------------------------------------------------------------*/

const struct sensors_sensor battery_sensor;

#define BATTERY_SAMPLES 10

static uint16_t battery_samples[BATTERY_SAMPLES];
static uint8_t counter = 0;

static uint8_t sensor_status = 0;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint32_t tmp = 0;
  uint8_t i;
  for(i = 0; i < BATTERY_SAMPLES; ++i) {
    tmp += battery_samples[i];
  }
  
  return tmp / BATTERY_SAMPLES;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  // all PORTB = Digital; RB10 = analog
  AD1PCFG = 0b1111110111111111;
  
  // SSRC bit = 111 implies internal counter ends sampling and starts converting
  AD1CON1 = 0b0000000011100000;
                                
  AD1CHS = 0b00000000000010100000000000000000;

  AD1CSSL = 0;

  // Manual Sample, Tad = internal 6 TPB
  AD1CON3 = 0x1F02;

  AD1CON2 = 0;

  // Turn ADC on
  AD1CON1SET = 0b1000000000000000;

  process_start(&battery_process, NULL);

  sensor_status = 1;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return sensor_status;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(battery_process, ev, data)
{
  PROCESS_BEGIN();
   
  while(1) {
    static struct etimer et;

    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    // start converting
    AD1CON1SET = 0b0000000000000010;

    while(!(AD1CON1 & 0b0000000000000001)) {
      ; // wait conversion finish
    }
    
    // read the conversion result
    battery_samples[counter] = ADC1BUF0;
    
    counter = (counter + 1) % BATTERY_SAMPLES;
  }

  PROCESS_END();
}

/** @} */
