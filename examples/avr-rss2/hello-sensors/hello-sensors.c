/*
 * Copyright (c) 2015, Copyright Robert Olsson / Radio Sensors AB  
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
 *
 * Author  : Robert Olsson robert@radio-sensors.com
 * Created : 2015-11-22
 */

/**
 * \file
 *         A simple application showing sensor reading on RSS2 mote
 */

#include "contiki.h"
#include "sys/etimer.h"
#include <stdio.h>
#include "adc.h"
#include "i2c.h"
#include "dev/leds.h"
#include "dev/battery-sensor.h"
#include "dev/temp-sensor.h"
#include "dev/temp_mcu-sensor.h"
#include "dev/light-sensor.h"
#include "dev/pulse-sensor.h"
#include "dev/bme280/bme280-sensor.h"
#include "dev/co2_sa_kxx-sensor.h"
/*---------------------------------------------------------------------------*/
PROCESS(hello_sensors_process, "Hello sensor process");
AUTOSTART_PROCESSES(&hello_sensors_process);

static struct etimer et;

static void
read_values(void)
{
  char serial[16];

  int i;
 
 /* Read out mote unique 128 bit ID */
  i2c_at24mac_read((char *) &serial, 0);
  printf("128_bit_ID=");
  for(i=0; i < 15; i++)
    printf("%02x", serial[i]);
  printf("%02x\n", serial[15]);
  printf("T=%-5.2f", ((double) temp_sensor.value(0)/100.));
  printf(" V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
  printf(" V_IN=%-4.2f", adc_read_v_in());
  printf(" V_AD1=%-4.2f", adc_read_a1());
  printf(" V_AD2=%-4.2f", adc_read_a2());
  printf(" T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
  printf(" LIGHT=%-d", light_sensor.value(0));
  printf(" PULSE_0=%-d", pulse_sensor.value(0));
  printf(" PULSE_1=%-d", pulse_sensor.value(1));

  if( i2c_probed & I2C_CO2SA ) {
    printf(" CO2=%-d", co2_sa_kxx_sensor.value( CO2_SA_KXX_CO2));
  }

  if( i2c_probed & I2C_BME280 ) {
#if STD_API
    printf(" BME280_TEMP=%-d", bme280_sensor.value(BME280_SENSOR_TEMP));
    printf(" BME280_RH=%-d", bme280_sensor.value(BME280_SENSOR_HUMIDITY));
    printf(" BME280_P=%-d", bme280_sensor.value(BME280_SENSOR_PRESSURE));
#else 
    /* Trigger burst read */
    bme280_sensor.value(BME280_SENSOR_TEMP);
    printf(" T_BME280=%5.2f", (double)bme280_mea.t_overscale100 / 100.);
    printf(" RH_BME280=%5.2f", (double)bme280_mea.h_overscale1024 / 1024.);
#ifdef BME280_64BIT
    printf(" P_BME280=%5.2f", (double)bme280_mea.p_overscale256 / 256.);
#else
    printf(" P_BME280=%5.2f", (double)bme280_mea.p);
#endif
#endif
  }

  printf("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_sensors_process, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(temp_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(temp_mcu_sensor);
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(pulse_sensor);

  if( i2c_probed & I2C_BME280 ) {
    SENSORS_ACTIVATE(bme280_sensor);
  }

  if( i2c_probed & I2C_CO2SA ) {
    SENSORS_ACTIVATE(co2_sa_kxx_sensor);
  }
  leds_init(); 
  leds_on(LEDS_RED);
  leds_on(LEDS_YELLOW);

  /* 
   * Delay 5 sec 
   * Gives a chance to trigger some pulses
   */

    etimer_set(&et, CLOCK_SECOND * 5);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    read_values();
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
