/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
/**
 * \addtogroup zoul-iaq-sensor
 * @{
 * Driver for the RE-Mote IAQ iAQ-Core (Indoor Air Quality Sensor)
 * \file
 * Driver for the RE-Mote RF IAQ iAQ-Core sensor (IAQ)
 * \author
 * Aitor Mejias <amejias@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/gpio.h"
#include "dev/i2c.h"
#include "iaq.h"
#include "sys/timer.h"
#include "sys/etimer.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Callback pointers when interrupt occurs */
void (*iaq_enable_callback)(uint16_t value);
/*---------------------------------------------------------------------------*/
static int16_t enabled;
/*---------------------------------------------------------------------------*/
static struct etimer et;
static simple_iaq_data iaq_data;
static uint8_t iaq_buffer[IAQ_FRAME_SIZE+1];
/*---------------------------------------------------------------------------*/
PROCESS(iaq_process, "IAQ process handler");
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
/* Return the status of the iAQ-Core or the status of the driver */
  if (type == IAQ_STATUS) {
    return (uint16_t)iaq_data.status;
  } else if (type == IAQ_DRIVER_STATUS) {
    return enabled;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(iaq_process, ev, data)
{
  #if DEBUG
  uint8_t i = 0;
  #endif

  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
      I2C_SCL_NORMAL_BUS_SPEED);

  while(1) {
    etimer_set(&et, (IAQ_POLLING_TIME));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    i2c_master_enable();
    if(i2c_burst_receive(IAQ_ADDR, &iaq_buffer[0], IAQ_FRAME_SIZE) != 
      I2C_MASTER_ERR_NONE) {
      PRINTF("IAQ: Failed to retrieve data from IAQ\n");
      enabled = IAQ_ERROR;
      iaq_data.status = IAQ_INTERNAL_ERROR;
    } else {
      #if DEBUG
      PRINTF("IAQ: Buffer ");
      for (i=1;i<10;i++) {
        PRINTF("[%d] %x,  ", i-1, iaq_buffer[i-1]);
      }
      PRINTF("\n");
      #endif
      /* Update the status of the sensor. This value readed represents the 
         internal status of the external driver. */
      switch (iaq_buffer[2]) {
        case IAQ_INTERNAL_SUCCESS:
          enabled = IAQ_ACTIVE;
        break;
        case IAQ_INTERNAL_RUNIN:
          enabled = IAQ_INIT_STATE;
        break;
        case IAQ_INTERNAL_BUSY:
        case IAQ_INTERNAL_ERROR:
          enabled = IAQ_ERROR;
        break;
        default:
          enabled = IAQ_ERROR;
        break;
      }
  
      iaq_data.tvoc = ((uint16_t)iaq_buffer[0] << 8) + iaq_buffer[1];
      iaq_data.co2 = ((uint16_t)iaq_buffer[7] << 8) + iaq_buffer[8];
      iaq_data.status = iaq_buffer[2];
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if (!enabled)  {
    PRINTF("IAQ: Sensor not enabled\n");
    return IAQ_ERROR;
  }
  if (enabled == IAQ_INIT_STATE)  {
    PRINTF("IAQ: Sensor initializing\n");
    return IAQ_INIT_STATE;
  }
  if (type == IAQ_CO2_VALUE) {
    return iaq_data.co2;
  }
  if (type == IAQ_VOC_VALUE) {
    return iaq_data.tvoc;
  }
  if (type == IAQ_STATUS) {
    #if DEBUG
    switch (iaq_data.status) {
      case IAQ_INTERNAL_SUCCESS:
        PRINTF("IAQ Status: SUCCESS\n");
      break;
      case IAQ_INTERNAL_RUNIN:
        PRINTF("IAQ Status: WARM UP\n");
      break;
      case IAQ_INTERNAL_BUSY:
      case IAQ_INTERNAL_ERROR:
        PRINTF("IAQ Status: ERROR\n");
      break;
      default:
        PRINTF("IAQ Status: UNKNOWN STATUS %d\n", iaq_data.status);
      break;
    }
    #endif
    return iaq_data.status;
  }

  return IAQ_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  /* Check the current status. If is initialized or is active, return the same
     state */
  if ((enabled == IAQ_INIT_STATE) || (enabled == IAQ_ACTIVE)) {
    return IAQ_ERROR;
  }

  /* Fix the status in initial wait status */
  enabled = IAQ_INIT_STATE;

  /* Start Internal process to measure the iAQ Sensor */
  process_start(&iaq_process, NULL);

  return enabled;
}
/*---------------------------------------------------------------------------*/
/* name, type, value, configure, status */
SENSORS_SENSOR(iaq, IAQ_SENSOR, value, configure, status);  
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

