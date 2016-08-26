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
 * \addtogroup zoul-dht22
 * @{
 *
 * \file
 *  Driver for the DHT22 temperature and humidity sensor
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dht22.h"
#include "dev/gpio.h"
#include "lib/sensors.h"
#include "dev/ioc.h"
#include "dev/watchdog.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) { \
      watchdog_periodic(); \
    } \
  } while(0)
/*---------------------------------------------------------------------------*/
#define DHT22_PORT_BASE          GPIO_PORT_TO_BASE(DHT22_PORT)
#define DHT22_PIN_MASK           GPIO_PIN_MASK(DHT22_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t busy;
static uint8_t dht22_data[DHT22_BUFFER];
/*---------------------------------------------------------------------------*/
static int
dht22_read(void)
{
  uint8_t i;
  uint8_t j = 0;
  uint8_t last_state;
  uint8_t counter = 0;
  uint8_t checksum = 0;

  if(enabled) {
    /* Exit low power mode and initialize variables */
    GPIO_SET_OUTPUT(DHT22_PORT_BASE, DHT22_PIN_MASK);
    GPIO_SET_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);
    BUSYWAIT_UNTIL(DHT22_AWAKE_TIME);
    memset(dht22_data, 0, DHT22_BUFFER);

    /* Initialization sequence */
    GPIO_CLR_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);
    BUSYWAIT_UNTIL(DHT22_START_TIME);
    GPIO_SET_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);
    clock_delay_usec(DHT22_READY_TIME);

    /* Prepare to read, DHT22 should keep line low 80us, then 80us high.
     * The ready-to-send-bit condition is the line kept low for 50us, then if
     * the line is high between 24-25us the bit sent will be "0" (zero), else
     * if the line is high between 70-74us the bit sent will be "1" (one).
     */
    GPIO_SET_INPUT(DHT22_PORT_BASE, DHT22_PIN_MASK);
    last_state = GPIO_READ_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);

    for(i = 0; i < DHT22_MAX_TIMMING; i++) {
      counter = 0;
      while(GPIO_READ_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK) == last_state) {
        counter++;
        clock_delay_usec(DHT22_READING_DELAY);

        /* Exit if not responsive */
        if(counter == 0xFF) {
          break;
        }
      }

      last_state = GPIO_READ_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);

      /* Double check for stray sensor */
      if(counter == 0xFF) {
        break;
      }

      /* Ignore the first 3 transitions (the 80us x 2 start condition plus the
       * first ready-to-send-bit state), and discard ready-to-send-bit counts
       */
      if((i >= 4) && ((i % 2) == 0)) {
        dht22_data[j / 8] <<= 1;
        if(counter > DHT22_COUNT) {
          dht22_data[j / 8] |= 1;
        }
        j++;
      }
    }

    for(i = 0; i < DHT22_BUFFER; i++) {
      PRINTF("DHT22: (%u) %u\n", i, dht22_data[i]);
    }

    /* If we have 5 bytes (40 bits), wrap-up and end */
    if(j >= 40) {
      /* The first 2 bytes are humidity values, the next 2 are temperature, the
       * final byte is the checksum
       */
      checksum = dht22_data[0] + dht22_data[1] + dht22_data[2] + dht22_data[3];
      checksum &= 0xFF;
      if(dht22_data[4] == checksum) {
        GPIO_SET_INPUT(DHT22_PORT_BASE, DHT22_PIN_MASK);
        GPIO_SET_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);
        return DHT22_SUCCESS;
      }
      PRINTF("DHT22: bad checksum\n");
    }
  }
  return DHT22_ERROR;
}
/*---------------------------------------------------------------------------*/
static uint16_t
dht22_humidity(void)
{
  uint16_t res;
  res = dht22_data[0];
  res *= 256;
  res += dht22_data[1];
  busy = 0;
  return res;
}
/*---------------------------------------------------------------------------*/
static uint16_t
dht22_temperature(void)
{
  uint16_t res;
  res = dht22_data[2] & 0x7F;
  res *= 256;
  res += dht22_data[3];
  busy = 0;
  return res;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if((type != DHT22_READ_HUM) && (type != DHT22_READ_TEMP) &&
     (type != DHT22_READ_ALL)) {
    PRINTF("DHT22: Invalid type %u\n", type);
    return DHT22_ERROR;
  }

  if(busy) {
    PRINTF("DHT22: ongoing operation, wait\n");
    return DHT22_BUSY;
  }

  busy = 1;

  if(dht22_read() != DHT22_SUCCESS) {
    PRINTF("DHT22: Fail to read sensor\n");
    GPIO_SET_INPUT(DHT22_PORT_BASE, DHT22_PIN_MASK);
    GPIO_SET_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);
    busy = 0;
    return DHT22_ERROR;
  }

  switch(type) {
  case DHT22_READ_HUM:
    return dht22_humidity();
  case DHT22_READ_TEMP:
    return dht22_temperature();
  case DHT22_READ_ALL:
    return DHT22_SUCCESS;
  default:
    return DHT22_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
int
dht22_read_all(int *temperature, int *humidity)
{
  if((temperature == NULL) || (humidity == NULL)) {
    PRINTF("DHT22: Invalid arguments\n");
    return DHT22_ERROR;
  }

  if(value(DHT22_READ_ALL) != DHT22_ERROR) {
    *temperature = dht22_temperature();
    *humidity = dht22_humidity();
    return DHT22_SUCCESS;
  }

  /* Already cleaned-up in the value() function */
  return DHT22_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return DHT22_ERROR;
  }

  GPIO_SOFTWARE_CONTROL(DHT22_PORT_BASE, DHT22_PIN_MASK);
  GPIO_SET_INPUT(DHT22_PORT_BASE, DHT22_PIN_MASK);
  ioc_set_over(DHT22_PORT, DHT22_PIN, IOC_OVERRIDE_OE);
  GPIO_SET_PIN(DHT22_PORT_BASE, DHT22_PIN_MASK);

  /* Restart flag */
  busy = 0;

  if(value) {
    enabled = 1;
    return DHT22_SUCCESS;
  }

  enabled = 0;
  return DHT22_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(dht22, DHT22_SENSOR, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
