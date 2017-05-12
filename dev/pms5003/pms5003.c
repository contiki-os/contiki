/*
 * Copyright (c) 2017, Peter Sjodin, KTH Royal Institute of Technology
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
 * Author  : Peter Sjodin, KTH Royal Institute of Technology
 * Created : 2017-04-21
 */

/*
 * \file
 *         Driver for Planttower PMSX003 dust sensors
 */

#include "contiki.h"
#include "sys/etimer.h"
#include "sys/pt.h"
#include <stdio.h>
#include "i2c.h"
#include "watchdog.h"
#include "dev/leds.h"
#include "dev/rs232.h"
#include "dev/pms5003-arch.h"
#include "pms5003.h"

#include "lib/ringbuf.h"

/*
 * Definitions for frames from PMSX003 sensors
 */

/* Two preamble bytes */
#define PRE1 0x42
#define PRE2 0x4d
/* Valid values for body length field */
#define PMSMINBODYLEN 20
#define PMSMAXBODYLEN 28
/* Buffer holds frame body plus preamble (two bytes)
 * and length field (two bytes) */
#define PMSBUFFER (PMSMAXBODYLEN + 4)

/* Frame assembly statistics */
static uint32_t invalid_frames, valid_frames;

/* Sensor configured on? */
static uint8_t configured_on = 0;
/* When sensor entered current power save mode, in clock_seconds()*/
static unsigned long when_mode;

/* Last readings of sensor data */
static uint16_t PM1, PM2_5, PM10;
static uint16_t PM1_ATM, PM2_5_ATM, PM10_ATM;
/* Time when last sensor data was read, in clock_seconds()*/
static unsigned long timestamp = 0;

#if PMS_SERIAL_UART
#if (PMS_BUFSIZE & (PMS_BUFSIZE - 1)) != 0
#error PMS5003_CONF_UART_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#endif /* PMS_BUFSIZE */

/* Ring buffer for storing input from uart */
static struct ringbuf rxbuf;
static uint8_t rxbuf_data[PMS_BUFSIZE];

static int uart_input_byte(unsigned char);
#endif /* PMS_SERIAL_UART */

/*---------------------------------------------------------------------------*/
#if PMS_SERIAL_UART
PROCESS(pms5003_uart_process, "PMS5003/UART dust sensor process");
#endif /* PMS_SERIAL_UART */
PROCESS(pms5003_timer_process, "PMS5003 periodic dust sensor process");
/*---------------------------------------------------------------------------*/
/**
 * Initialize. Create event, and start timer-driven process.
 * If UART enabled, also install UART callback function and
 * start PMS frame assembly process.
 */
void
pms5003_init()
{
  pms5003_event = process_alloc_event();
  process_start(&pms5003_timer_process, NULL);

#if PMS_SERIAL_UART
  ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));
  rs232_set_input(PMS_UART_PORT, uart_input_byte);
  process_start(&pms5003_uart_process, NULL);
#endif /* PMS_SERIAL_UART */

  configured_on = 1;

#ifdef DEBUG
  printf("PMS5003: UART %d, I2C %d, sample period %d, startup interval %d\n",
         PMS_SERIAL_UART, PMS_SERIAL_I2C, PMS_SAMPLE_PERIOD, PMS_STARTUP_INTERVAL);
#endif /* DEBUG */
}
/*---------------------------------------------------------------------------*/
/**
 * Sensor API for PMS5003
 */

void
pms5003_off()
{
  pms5003_set_standby_mode(STANDBY_MODE_ON);
  configured_on = 0;
}
uint16_t
pms5003_pm1()
{
  return PM1;
}
uint16_t
pms5003_pm2_5()
{
  return PM2_5;
}
uint16_t
pms5003_pm10()
{
  return PM10;
}
uint16_t
pms5003_pm1_atm()
{
  return PM1_ATM;
}
uint16_t
pms5003_pm2_5_atm()
{
  return PM2_5_ATM;
}
uint16_t
pms5003_pm10_atm()
{
  return PM10_ATM;
}
uint32_t
pms5003_timestamp()
{
  return timestamp;
}
uint32_t
pms5003_valid_frames()
{
  return valid_frames;
}
uint32_t
pms5003_invalid_frames()
{
  return invalid_frames;
}
/*---------------------------------------------------------------------------*/
/**
 * Validate frame by checking preamble, length field and checksum.
 * Return 0 if invalid frame, otherwise 1.
 */
static int
check_pmsframe(uint8_t *buf)
{
  int sum, pmssum;
  int i;
  int len;

  if(buf[0] != PRE1 || buf[1] != PRE2) {
    return 0;
  }
  /* len is length of frame not including preamble and checksum */
  len = (buf[2] << 8) + buf[3];
  if(len < PMSMINBODYLEN || len > PMSMAXBODYLEN) {
    return 0;
  }
  /* Sum data bytewise, including preamble but excluding checksum */
  sum = 0;
  for(i = 0; i < len + 2; i++) {
    sum += buf[i];
  }
  /* Compare with received checksum last in frame*/
  pmssum = (buf[len + 2] << 8) + buf[len + 3];
  return pmssum == sum;
}
/*---------------------------------------------------------------------------*/
static void
printpm()
{
  printf("PMS frames: valid %lu, invalid %lu\n",
         valid_frames, invalid_frames);
  printf("PM1 = %04d, PM2.5 = %04d, PM10 = %04d\n", PM1, PM2_5, PM10);
  printf("PM1_ATM = %04d, PM2.5_ATM = %04d, PM10_ATM = %04d\n",
         PM1_ATM, PM2_5_ATM, PM10_ATM);
}
/*---------------------------------------------------------------------------*/
/**
 * Frame received from PMS sensor. Validate and update sensor data.
 * Return 1 if valid frame, otherwise 0
 */
static int
pmsframe(uint8_t *buf)
{

  if(check_pmsframe(buf)) {
    timestamp = clock_seconds();
    valid_frames++;
    /* Update sensor readings */
    PM1 = (buf[4] << 8) | buf[5];
    PM2_5 = (buf[6] << 8) | buf[7];
    PM10 = (buf[8] << 8) | buf[9];
    PM1_ATM = (buf[10] << 8) | buf[11];
    PM2_5_ATM = (buf[12] << 8) | buf[13];
    PM10_ATM = (buf[14] << 8) | buf[15];
#ifdef DEBUG
    printpm();
#endif /* DEBUG */
    return 1;
  } else {
    invalid_frames++;
#ifdef DEBUG
    printpm();
#endif /* DEBUG */
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
#if PMS_SERIAL_UART
/**
 * State machine for assembling PMS5003 frames
 * from uart. Use protothread for state machine.
 */
static
PT_THREAD(pms5003_uart_fsm_pt(struct pt *pt, uint8_t data)) {
  static uint8_t buf[PMSBUFFER], *bufp;
  static int remain;
  static unsigned long mode_secs;

  PT_BEGIN(pt);
  bufp = buf;
  if(data != PRE1) {
    PT_RESTART(pt);
  }
  *bufp++ = data;
  PT_YIELD(pt);
  if(data != PRE2) {
    PT_RESTART(pt);
  }
  *bufp++ = data;
  /* Found preamble. Then get length (two bytes) */
  PT_YIELD(pt);
  *bufp++ = data;
  PT_YIELD(pt);
  *bufp++ = data;

  /* Get body length -- no of bytes that remain */
  remain = (buf[2] << 8) + buf[3];
  if(remain < PMSMINBODYLEN || remain > PMSMAXBODYLEN) {
    invalid_frames++;
  } else {
    while(remain--) {
      PT_YIELD(pt);
      *bufp++ = data;
    }
    /* We have a frame! */
    mode_secs = clock_seconds() - when_mode;
    /* Frames received while sensor is starting up are ignored */
    if((pms5003_get_standby_mode() == STANDBY_MODE_OFF) &&
       (mode_secs >= PMS_STARTUP_INTERVAL)) {
      /* Check frame and update sensor readings */
      if(pmsframe(buf)) {
        /* Tell other processes there is new data */
        (void)process_post(PROCESS_BROADCAST, pms5003_event, NULL);
        /* Enter standby mode */
        pms5003_set_standby_mode(STANDBY_MODE_ON);
        when_mode = clock_seconds();
      }
    }
  }
  PT_RESTART(pt);

  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
/**
 * UART callback function.
 */
static int
uart_input_byte(unsigned char c)
{

  /* Add char to buffer. Unlike serial line input, ignore buffer overflow */
  (void)ringbuf_put(&rxbuf, c);
  /* Wake up consumer process */
  process_poll(&pms5003_uart_process);
  return 1;
}
/*---------------------------------------------------------------------------*/
static struct pt uart_pt;
/**
 * Consumer thread for UART process. Pick up data from input buffer and
 * dispatch to FSM for frame assembly.
 */
PROCESS_THREAD(pms5003_uart_process, ev, data)
{

  PROCESS_BEGIN();
  PT_INIT(&uart_pt);
  while(1) {
    int c = ringbuf_get(&rxbuf);
    if(c == -1) {
      PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    } else if(configured_on) {
      pms5003_uart_fsm_pt(&uart_pt, c);
    }
  }
  PROCESS_END();
}
#endif /* PMS_SERIAL_UART */
/*---------------------------------------------------------------------------*/
/**
 * Timer thread: duty-cycle sensor. Toggle between idle and active mode.
 * For I2C, also read data when it is due.
 */
PROCESS_THREAD(pms5003_timer_process, ev, data)
{
  static struct etimer pmstimer;
  static unsigned long mode_secs;
  static uint8_t standbymode;

  PROCESS_BEGIN();
  etimer_set(&pmstimer, CLOCK_SECOND * PMS_PROCESS_PERIOD);
  pms5003_set_standby_mode(STANDBY_MODE_ON);
  when_mode = clock_seconds();
  pms5003_event = process_alloc_event();

  /* Main loop */
  while(1) {
    PROCESS_YIELD();
    if(!configured_on) {
      continue;
    }

    if((ev == PROCESS_EVENT_TIMER) && (data == &pmstimer)) {
      mode_secs = clock_seconds() - when_mode;
      standbymode = pms5003_get_standby_mode();
      if(standbymode == STANDBY_MODE_OFF) {
#if PMS_SERIAL_I2C
        static uint8_t buf[PMSBUFFER];
        /* Read data over I2C if it is time */
        if(mode_secs >= PMS_STARTUP_INTERVAL) {
          if(pms5003_i2c_probe()) {
            leds_on(LEDS_RED);
            i2c_read_mem(I2C_PMS5003_ADDR, 0, buf, PMSBUFFER);
            /* Check frame and update sensor readings */
            if(pmsframe(buf)) {
              /* Tell other processes there is new data */
              if(process_post(PROCESS_BROADCAST, pms5003_event, NULL) == PROCESS_ERR_OK) {
                PROCESS_WAIT_EVENT_UNTIL(ev == pms5003_event);
              }
              pms5003_set_standby_mode(STANDBY_MODE_ON);
              when_mode = clock_seconds();
            }
          }
        }
#else
        /* Do nothing -- UART process puts sensor in standby */
        ;
#endif /* PMS_SERIAL_I2C */
      } else if(standbymode == STANDBY_MODE_ON) {
        if(mode_secs >= (PMS_SAMPLE_PERIOD - PMS_STARTUP_INTERVAL)) {
          pms5003_set_standby_mode(STANDBY_MODE_OFF);
          when_mode = clock_seconds();
        }
      }
      etimer_reset(&pmstimer);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
