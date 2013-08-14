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
 *         This file handles the sensor readings and float conversions
 *         for sensors-example. We keep this separate in order to place it
 *         to a higher BANK.
 *
 *         Bankable
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki-conf.h"
#include "uip.h" /* for htons / htonl */
#include "dev/leds.h"
#if CONTIKI_TARGET_SENSINODE
#include "dev/sensinode-sensors.h"
#include "debug.h"
#endif

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#include <stdio.h>
#include <ctype.h>

#define SENSOR_OK       0
#define SENSOR_ADC_OFF -1
#define SENSOR_UNKNOWN -2

/* Request Bits */
#define REQUEST_BIT_P0_GET    0x0400
#define REQUEST_BIT_L2_SET    0x0200
#define REQUEST_BIT_L1_SET    0x0100
#define REQUEST_BIT_LED_GET   0x0080
#define REQUEST_BIT_ACC       0x0040
#define REQUEST_BIT_BAT       0x0020
#define REQUEST_BIT_VDD       0x0010
#define REQUEST_BIT_TEMP      0x0008
#define REQUEST_BIT_LIGHT     0x0004
#define REQUEST_BIT_UPTIME    0x0002
#define REQUEST_BIT_CHIPID    0x0001

/*---------------------------------------------------------------------------*/
int8_t
read_sensor(char *rs)
{
  /* Sensor Values */
  static int rv;
  static struct sensors_sensor *sensor;

  /* Those 3 variables are only used for debugging */
#if DEBUG
  static float sane = 0;
  static int dec;
  static float frac;
#endif
  uint16_t r;
  uint8_t len = 0;

  sensor = sensors_find(ADC_SENSOR);
  if(!sensor) {
    PRINTF("ADC not found\n");
    return (SENSOR_ADC_OFF);
  }

  /* Fetch the request bytes */
  memcpy(&r, rs, 2);
  r = uip_ntohs(r);
  PRINTF("R=%u\n", r);

  if(r & REQUEST_BIT_CHIPID) {
    uint8_t chipid = CHIPID;
    memcpy(rs + len, &chipid, sizeof(chipid));
    len += sizeof(chipid);
    PRINTF("ChipID=0x%02x\n", chipid);
  }
  if(r & REQUEST_BIT_UPTIME) {
    /* Uptime */
    unsigned long l;

    l = uip_htonl(clock_seconds());
    memcpy(rs + len, &l, sizeof(l));
    len += sizeof(l);
    PRINTF("Uptime=%lu secs\n", uip_ntohl(l));
  }
  if(r & REQUEST_BIT_LIGHT) {
    rv = sensor->value(ADC_SENSOR_TYPE_LIGHT);
    if(rv != -1) {
#if DEBUG
      sane = (float)(rv * 0.4071);
      dec = sane;
      frac = sane - dec;
      PRINTF(" Light=%d.%02ulux (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
  }
  if(r & REQUEST_BIT_TEMP) {
    rv = sensor->value(ADC_SENSOR_TYPE_TEMP);
    if(rv != -1) {
#if DEBUG
      sane = ((rv * 0.61065 - 773) / 2.45);
      dec = sane;
      frac = sane - dec;
      PRINTF("  Temp=%d.%02u C (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
  }
  if(r & (REQUEST_BIT_VDD | REQUEST_BIT_BAT)) {
    /* We want VDD for both cases */
    rv = sensor->value(ADC_SENSOR_TYPE_VDD);
    if(rv != -1) {
#if DEBUG
      sane = rv * 3.75 / 2047;
      dec = sane;
      frac = sane - dec;
      PRINTF("Supply=%d.%02uV (%d)\n", dec, (unsigned int)(frac * 100), rv);
      /* Store rv temporarily in dec so we can use it for the battery */
      dec = rv;
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
    /* And then carry on with battery if needed */
    if(r & REQUEST_BIT_BAT) {
      rv = sensor->value(ADC_SENSOR_TYPE_BATTERY);
      if(rv != -1) {
#if DEBUG
        sane = (11.25 * rv * dec) / (0x7FE002);
        dec = sane;
        frac = sane - dec;
        PRINTF(" Batt.=%d.%02uV (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
        memcpy(rs + len, &rv, sizeof(rv));
        len += sizeof(rv);
      }
    }
  }
  if(r & REQUEST_BIT_ACC) {
    rv = sensor->value(ADC_SENSOR_TYPE_ACC_X);
    if(rv != -1) {
#if DEBUG
      sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
      dec = sane;
      frac = sane - dec;
      frac = (frac < 0) ? -frac : frac;

      PRINTF("  AccX=");
      if(sane < 0 && dec == 0) {
        PRINTF('-');
      }
      PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
    rv = sensor->value(ADC_SENSOR_TYPE_ACC_Y);
    if(rv != -1) {
#if DEBUG
      sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
      dec = sane;
      frac = sane - dec;
      frac = (frac < 0) ? -frac : frac;
      PRINTF("  AccY=");
      if(sane < 0 && dec == 0) {
        PRINTF('-');
      }
      PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
    rv = sensor->value(ADC_SENSOR_TYPE_ACC_Z);
    if(rv != -1) {
#if DEBUG
      sane = ((rv * 3.75 / 2047) - 1.65) / 0.44;
      dec = sane;
      frac = sane - dec;
      frac = (frac < 0) ? -frac : frac;
      PRINTF("  AccZ=");
      if(sane < 0 && dec == 0) {
        PRINTF('-');
      }
      PRINTF("%d.%02ug (%d)\n", dec, (unsigned int)(frac * 100), rv);
#endif
      memcpy(rs + len, &rv, sizeof(rv));
      len += sizeof(rv);
    }
  }
  if(r & REQUEST_BIT_L1_SET) {
    leds_toggle(LEDS_GREEN);
  }
  if(r & REQUEST_BIT_L2_SET) {
    leds_toggle(LEDS_RED);
  }
  if(r & REQUEST_BIT_LED_GET) {
    uint8_t leds = leds_get();
    memcpy(rs + len, &leds, sizeof(leds));
    len += sizeof(leds);
    PRINTF(" LED 2=%u\n", leds);
  }
  if(r & REQUEST_BIT_P0_GET) {
    uint8_t p0 = P0_3;
    memcpy(rs + len, &p0, sizeof(p0));
    len += sizeof(p0);
  }
  return len;
}
