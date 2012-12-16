/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Sources for the BATtery MONitor app. It dumps a log entry to the
 *         external flash periodically as well as upon external trigger.
 *
 *         It started off as a VDD and battery logger but now it also stores
 *         energest values and other goodies.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "sys/etimer.h"
#include "sys/energest.h"
#include "dev/sensinode-sensors.h"
#include "dev/n740.h"
#include "dev/m25p16.h"

#define BATMON_LOG_PERIOD 60 /* in seconds */
/*---------------------------------------------------------------------------*/
static const uint8_t magic[3] = { 0x0B, 0xEE, 0xF0 };
/*---------------------------------------------------------------------------*/
struct record {
  uint8_t magic[3];
  uint8_t trigger;
  unsigned long c; /* uptime */
  int v; /* VDD (reference) */
  int b; /* Voltage ADC */
#if ENERGEST_CONF_ON
  unsigned long mcu;
  unsigned long lpm;
  unsigned long irq;
  unsigned long tx;
  unsigned long rx;
  unsigned long f_write;
  unsigned long f_read;
#endif
};

#define RECORD_SIZE 64
#define LAST_WRITE (0xFFFF - RECORD_SIZE)

#define LOG_TRIGGER_PERIODIC 0xFF
/*---------------------------------------------------------------------------*/
struct flash_address {
  uint8_t s; /* sector */
  uint8_t p; /* page */
  uint8_t a; /* address */
};
static struct flash_address f;

static struct record r;
static struct sensors_sensor *s;
static struct etimer et;
#define FLASH_START_ADDR 0x1E0000
#define FLASH_END_ADDR   0x1FFFFF
/*---------------------------------------------------------------------------*/
PROCESS(batmon_process, "Logger Process");
/*---------------------------------------------------------------------------*/
static int
find_gap() CC_NON_BANKED
{
  uint8_t seq[3];
  uint32_t address = FLASH_START_ADDR;
  memset(&f, 0, sizeof(f));

  for(address = FLASH_START_ADDR; address <= FLASH_END_ADDR; address +=
      RECORD_SIZE) {
    n740_analog_deactivate();
    f.s = ((address & 0xFF0000) >> 16);
    f.p = ((address & 0xFF00) >> 8);
    f.a = address & 0xFF;
    m25p16_read_fast((uint8_t *)&f, seq, sizeof(magic));
    n740_analog_activate();
    if(memcmp(seq, magic, sizeof(magic)) != 0) {
      PRINTF("BatMon: Resume write @ 0x%02x%02x%02x\n", f.s, f.p, f.a);
      return 1;
    }
  }

  /* If we reach here, we ran out of flash */
  return -1;
}
/*---------------------------------------------------------------------------*/
static void
abort() CC_NON_BANKED
{
  PRINTF("BatMon: Abort\n");
  etimer_stop(&et);
  process_exit(&batmon_process);
}
/*---------------------------------------------------------------------------*/
void
batmon_log(uint8_t trigger)
{
  uint32_t next;

  /* Only continue if the process (us) is running */
  if(!process_is_running(&batmon_process)) {
    return;
  }

  next = f.a;
  next |= (((uint32_t) f.p) << 8);
  next |= (((uint32_t) f.s) << 16);

  memcpy(r.magic, magic, sizeof(magic));
  r.trigger = trigger;
  r.c = clock_seconds();

  /* Read VDD and use as ADC reference */
  r.v = s->value(ADC_SENSOR_TYPE_VDD);

  /* And then carry on with battery */
  r.b = s->value(ADC_SENSOR_TYPE_BATTERY);

#if ENERGEST_CONF_ON
  /* ENERGEST values */
  r.mcu = energest_type_time(ENERGEST_TYPE_CPU);
  r.lpm = energest_type_time(ENERGEST_TYPE_LPM);
  r.irq = energest_type_time(ENERGEST_TYPE_IRQ);
  r.tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  r.rx = energest_type_time(ENERGEST_TYPE_LISTEN);
  r.f_write = energest_type_time(ENERGEST_TYPE_FLASH_WRITE);
  r.f_read = energest_type_time(ENERGEST_TYPE_FLASH_READ);
#endif

  n740_analog_deactivate();
  /* Make sure we're on */
  if(M25P16_WIP()) {
    m25p16_res();
  }
  m25p16_pp((uint8_t *)&f, (uint8_t *)&r, sizeof(r));
  n740_analog_activate();

  PRINTF("BatMon: @%lu [%u] ", r.c, r.trigger);
  PRINTF("BatMon: 0x%02x%02x%02x\n", f.s, f.p, f.a);

  next += RECORD_SIZE;

  if(next >= FLASH_END_ADDR) {
    abort();
    return;
  }

  f.s = ((next & 0xFF0000) >> 16);
  f.p = ((next & 0xFF00) >> 8);
  f.a = next & 0xFF;

  if(trigger == LOG_TRIGGER_PERIODIC) {
    etimer_reset(&et);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(batmon_process, ev, data)
{

  PROCESS_BEGIN();

  PRINTF("BatMon\n", sizeof(r));

  s = sensors_find(ADC_SENSOR);
  if(!s) {
    PRINTF("BatMon: ADC not found\n");
    PROCESS_EXIT();
  }

  n740_analog_deactivate();
  m25p16_res();
  n740_analog_activate();

  /* Find last written location */
  if(find_gap() == -1) {
    PRINTF("BatMon: Flash storage full\n");
    PROCESS_EXIT();
  }

  etimer_set(&et, BATMON_LOG_PERIOD * CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&et)) {
      batmon_log(LOG_TRIGGER_PERIODIC);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
