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
 * \addtogroup remote-zonik
 * @{
 * Driver for the RE-Mote Zonik sonometer board
 * @{
 * \file
 * Driver for the RE-Mote Zonik sound sensor (ZONIK)
 * \author
 * Aitor Mejias <amejias@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/gpio.h"
#include "dev/i2c.h"
#include "zonik.h"
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
#define ZONIK_INT1_PORT_BASE  GPIO_PORT_TO_BASE(ZONIK_INT_PORT)
#define ZONIK_INT1_PIN_MASK   GPIO_PIN_MASK(ZONIK_INT_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t zonik_buffer[ZONIK_FRAME_SIZE+1];
static uint16_t zonik_status = ZONIK_DISABLED;
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(zonik_stm_process, "Zonik process process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(zonik_stm_process, ev, data)
{
  #if DEBUG
  static int i;
  #endif
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  while(1) {
    /*  Wait a process  */
    etimer_set(&et, ZONIK_SECOND_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Control the interrupt for activate the sensor */
    GPIO_SET_OUTPUT(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    GPIO_CLR_PIN(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    clock_delay_usec(ZONIK_INITIAL_WAIT_DELAY);
    i2c_master_enable();
    if(i2c_single_send(ZONIK_ADDR, ZONIK_CMD_READ) != I2C_MASTER_ERR_NONE) {
      zonik_status = ZONIK_ERROR;
      PRINTF("Zonik: Error in I2C Communication\n");
    }
    GPIO_SET_PIN(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    GPIO_SET_INPUT(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    if(zonik_status != ZONIK_ERROR) {
      etimer_set(&et, ZONIK_WAIT_ACQ);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      clock_delay_usec(ZONIK_FINAL_WAIT_DELAY);
      i2c_master_enable();
      if(i2c_burst_receive(ZONIK_ADDR, &zonik_buffer[0], ZONIK_FRAME_SIZE) != 
                           I2C_MASTER_ERR_NONE) {
        zonik_status = ZONIK_ERROR;
        PRINTF("Zonik: Error in I2C Burst Mode Receive");
      }
      #if DEBUG
      PRINTF("\nZonik: ");
      for(i=0; i<ZONIK_FRAME_SIZE; i++) {
        PRINTF(" 0x%02x ", zonik_buffer[i]);	  
      }
      PRINTF("\n");
      #endif
      if(zonik_status != ZONIK_ERROR) {
        zonik_status = ZONIK_ACTIVE;
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return zonik_status;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* If the system is enabling the sensor internally. Return the same state */
  if(zonik_status == ZONIK_HW_INIT) {
    return ZONIK_HW_INIT;
  }
  /* If no valid parameter, return error */
  if(zonik_status != ZONIK_ACTIVE) {
    return ZONIK_DISABLED;
  }
  /* Return the dBA received data */
  if(type == ZONIK_DBA_LEQ_VALUE) {
    return ((uint16_t)zonik_buffer[2] << 8) + zonik_buffer[3];
  }
  /* Return the internal counter loop received data */
  if(type == ZONIK_COUNT_VALUE) {
    return ((uint16_t)zonik_buffer[0] << 8) + zonik_buffer[1];
  }
  return ZONIK_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == ZONIK_ACTIVE) {
    if(value == ZONIK_VALUE_DEACTIVATE) {
      /* Disable the Zonik Sensor reading process */
      process_exit(&zonik_stm_process);
      zonik_status = ZONIK_DISABLED;
      return zonik_status;
    }
    /* Enable the Zonik Sensor reading process */
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
             I2C_SCL_NORMAL_BUS_SPEED);

    /* configuration of I2C interrupt control */
    GPIO_SOFTWARE_CONTROL(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    GPIO_SET_INPUT(ZONIK_INT1_PORT_BASE, ZONIK_INT1_PIN_MASK);
    /* Launch the main process */
    process_start(&zonik_stm_process, NULL);
    zonik_status = ZONIK_HW_INIT;
    return zonik_status;
  }
  /* Bad configuration */
  return ZONIK_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(zonik, ZONIK_SENSOR, value, configure, status);  
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

