/*
 * Copyright (c) 2015, Copyright Robert Olsson
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
 * Author  : Robert Olsson rolss@kth.se/robert@radio-sensors.com
 * Created : 2016-09-14
 */

/**
 * \file
 *        Basic functions for Bosch BME280 based on datasheet Rev 1.1
 */

#include "contiki.h"
#include <string.h>
#include "bme280.h"
#include "bme280-arch.h"
#include "lib/sensors.h"

static struct {
  unsigned short dig_t1;
  signed short dig_t2;
  signed short dig_t3;
  unsigned short dig_p1;
  signed short dig_p2;
  signed short dig_p3;
  signed short dig_p4;
  signed short dig_p5;
  signed short dig_p6;
  signed short dig_p7;
  signed short dig_p8;
  signed short dig_p9;
  unsigned char dig_h1;
  signed short dig_h2;
  unsigned char dig_h3;
  signed short dig_h4;
  signed short dig_h5;
  signed char dig_h6;
  int32_t t_fine;
  uint8_t mode;
} bm;

int32_t
bme280_t_overscale100(int32_t ut)
{
  int32_t v1, v2, t;

  v1 = ((((ut >> 3) - ((int32_t)bm.dig_t1 << 1))) *
        ((int32_t)bm.dig_t2)) >> 11;

  v2 = (((((ut >> 4) - ((int32_t)bm.dig_t1)) * ((ut >> 4) -
                                                ((int32_t)bm.dig_t1))) >> 12) * ((int32_t)bm.dig_t3)) >> 14;

  bm.t_fine = v1 + v2;
  t = (bm.t_fine * 5 + 128) >> 8;
  return t;
}
#ifdef BME280_32BIT
static uint32_t
bme280_p(int32_t up)
{
  int32_t v1, v2;
  uint32_t p;

  v1 = (((int32_t)bm.t_fine) >> 1) - (int32_t)64000;
  v2 = (((v1 >> 2) * (v1 >> 2)) >> 11) * ((int32_t)bm.dig_p6);
  v2 = v2 + ((v1 * ((int32_t)bm.dig_p5)) << 1);
  v2 = (v2 >> 2) + (((int32_t)bm.dig_p4) << 16);

  v1 = (((bm.dig_p3 * (((v1 >> 2) * (v1 >> 2)) >> 13)) >> 3) +
        ((((int32_t)bm.dig_p2) * v1) >> 1)) >> 18;

  v1 = ((((32768 + v1)) * ((int32_t)bm.dig_p1)) >> 15);

  if(v1 == 0) {
    return 0;
  }

  p = (((uint32_t)(((int32_t)1048576) - up) - (v2 >> 12))) * 3125;

  if(p < 0x80000000) {
    p = (p << 1) / ((uint32_t)v1);
  } else {
    p = (p / (uint32_t)v1) * 2;
  }

  v1 = (((int32_t)bm.dig_p9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
  v2 = (((int32_t)(p >> 2)) * ((int32_t)bm.dig_p8)) >> 13;
  p = (uint32_t)((int32_t)p + ((v1 + v2 + bm.dig_p7) >> 4));
  return p;
}
#else

static uint32_t
bme280_p_overscale256(int32_t up)
{
  int64_t v1, v2, p;

  v1 = ((int64_t)bm.t_fine) - 128000;
  v2 = v1 * v1 * (int64_t)bm.dig_p6;
  v2 = v2 + ((v1 * (int64_t)bm.dig_p5) << 17);
  v2 = v2 + (((int64_t)bm.dig_p4) << 35);
  v1 = ((v1 * v1 * (int64_t)bm.dig_p3) >> 8) + ((v1 * (int64_t)bm.dig_p2) << 12);
  v1 = (((((int64_t)1) << 47) + v1)) * ((int64_t)bm.dig_p1) >> 33;

  if(v1 == 0) {
    return 0;
  }

  p = 1048576 - up;
  p = (((p << 31) - v2) * 3125) / v1;
  v1 = (((int64_t)bm.dig_p9) * (p >> 13) * (p >> 13)) >> 25;
  v2 = (((int64_t)bm.dig_p8) * p) >> 19;
  p = (((p + v1 + v2) >> 8) + (((int64_t)bm.dig_p7) << 4));
  return (uint32_t)p;
}
#endif

static uint32_t
bme280_h_overscale1024(int32_t uh)
{
  int32_t v1;
  v1 = (bm.t_fine - ((int32_t)76800));
  v1 = (((((uh << 14) - (((int32_t)bm.dig_h4) << 20) - (((int32_t)bm.dig_h5) * v1)) + ((int32_t)16384)) >> 15)
        * (((((((v1 * ((int32_t)bm.dig_h6)) >> 10) * (((v1 * ((int32_t)bm.dig_h3)) >> 11) + ((int32_t)32768)))
              >> 10) + ((int32_t)2097152)) * ((int32_t)bm.dig_h2) + 8192) >> 14));
  v1 = (v1 - (((((v1 >> 15) * (v1 >> 15)) >> 7) * ((int32_t)bm.dig_h1)) >> 4));
  v1 = (v1 < 0 ? 0 : v1);
  v1 = (v1 > 419430400 ? 419430400 : v1);
  return (uint32_t)(v1 >> 12);
}
uint8_t
bme280_init(uint8_t mode)
{
  uint16_t i;
  uint8_t buf[26];

  bme280_arch_i2c_init();

  /* Do not mess with other chips */
  bme280_arch_i2c_read_mem(BME280_ADDR, 0xD0, buf, 1);
  if(buf[0] != BME280_CHIP_ID) {
    return 0;
  }

  bme280_arch_i2c_write_mem(BME280_ADDR, BME280_CNTL_RESET, 0xB6);

  for(i = 0; i < BME280_MAX_WAIT; i++) {
    clock_delay_usec(1000);
  }

  memset(buf, 0, sizeof(buf));

  /* Burst read of all calibration part 1 */
  bme280_arch_i2c_read_mem(BME280_ADDR, BME280_DIG_T1_ADDR, buf, sizeof(buf));
  bm.dig_t1 = ((uint16_t)buf[1] << 8) | (uint16_t)buf[0];
  bm.dig_t2 = ((int16_t)buf[3] << 8) | (uint16_t)buf[2];
  bm.dig_t3 = ((int16_t)buf[5] << 8) | (uint16_t)buf[4];
  bm.dig_p1 = ((uint16_t)buf[7] << 8) | (uint16_t)buf[6];
  bm.dig_p2 = ((int16_t)buf[9] << 8) | (uint16_t)buf[8];
  bm.dig_p3 = ((int16_t)buf[11] << 8) | (uint16_t)buf[10];
  bm.dig_p4 = ((int16_t)buf[13] << 8) | (uint16_t)buf[12];
  bm.dig_p5 = ((int16_t)buf[15] << 8) | (uint16_t)buf[14];
  bm.dig_p6 = ((int16_t)buf[17] << 8) | (uint16_t)buf[16];
  bm.dig_p7 = ((int16_t)buf[19] << 8) | (uint16_t)buf[18];
  bm.dig_p8 = ((int16_t)buf[21] << 8) | (uint16_t)buf[20];
  bm.dig_p9 = ((int16_t)buf[23] << 8) | (uint16_t)buf[22];
  /* A0 not used */
  bm.dig_h1 = (unsigned char)buf[25];

  /* Burst read of all calibration part 2 */
  bme280_arch_i2c_read_mem(BME280_ADDR, BME280_DIG_H2_ADDR, buf, 8);
  bm.dig_h2 = ((int16_t)buf[1] << 8) | (uint16_t)buf[0];
  bm.dig_h3 = (unsigned char)buf[2];
  bm.dig_h4 = ((int16_t)buf[3] << 4) | (((uint16_t)buf[4]) & 0xF);
  bm.dig_h5 = ((int16_t)buf[6] << 4) | (((uint16_t)buf[5]) & 0xF);
  bm.dig_h6 = (unsigned char)buf[7];

  bm.mode = mode;
  return 1;
}
void
bme280_read(uint8_t mode)
{
  int32_t ut, uh, up;
  uint8_t buf[8], sleep;
  uint16_t i;
  memset(buf, 0, sizeof(buf));

  /* Are we initilized and in the right mode? */
  if(mode == BME280_MODE_NONE || mode != bm.mode) {
    return;
  }

  ut = uh = up = 0;

  /* Weather mode. See sectiom 3.5 Datasheet */
  if(mode == BME280_MODE_WEATHER) {
    /* Humidity oversampling *1 */
    bme280_arch_i2c_write_mem(BME280_ADDR, BME280_CNTL_HUM, 0x01);

    /*  00100111  0x27 oversampling *1 for t and p plus normal mode */
    /* 0.5 ms -- no filter -- no SPI */
    bme280_arch_i2c_write_mem(BME280_ADDR, BME280_CONTROL, 0x00);

    /*  00100110  0x26 oversampling *1 for t and p  plus forced mode */
    /* Trigger measurement needed for every time in forced mode */
    bme280_arch_i2c_write_mem(BME280_ADDR, BME280_CNTL_MEAS, 0x26);
    /* Wait to get into sleep mode == measurement done */
    for(i = 0; i < BME280_MAX_WAIT; i++) {
      bme280_arch_i2c_read_mem(BME280_ADDR, BME280_CNTL_MEAS, &sleep, 1);
      sleep = sleep& 0x03;
      if(sleep== 0) {
        break;
      } else {
        clock_delay_usec(1000);
      }
    }
    if(i == BME280_MAX_WAIT) {
      return; /* error  wait*/
    }
  } else {  /* if(mode == BME280_MODE_WEATHER) */
    return; /* error mode*/
  }

  /* Burst read of all measurements */
  bme280_arch_i2c_read_mem(BME280_ADDR, BME280_PRESS, buf, 8);
  ut = (uint32_t)(buf[3]) << 12 | (uint32_t)(buf[4]) << 4 | (uint32_t)buf[5] >> 4;
  up = (uint32_t)(buf[0]) << 12 | (uint32_t)(buf[1]) << 4 | (uint32_t)buf[2] >> 4;
  uh = (uint32_t)(buf[6]) << 8 | (uint32_t)buf[7];

  bme280_mea.t_overscale100 = bme280_t_overscale100(ut);
  bme280_mea.h_overscale1024 = bme280_h_overscale1024(uh);
#ifdef BME280_64BIT
  bme280_mea.p_overscale256 = bme280_p_overscale256(up);
#else
  bme280_mea.p = bme280_p(up);
#endif

#if TEST
  printf("T_BME280=%5.2f", (double)bme280_mea.t_overscale100 / 100.);
  printf(" RH_BME280=%5.2f ", (double)bme280_mea.h_overscale1024 / 1024.);
#ifdef BME280_64BIT
  printf(" P_BME280=%5.2f\n", (double)bme280_mea.p_overscale256 / 256.);
#else
  printf(" P_BME280=%5.2f\n", (double)bme280_mea.p);
#endif
#endif
}
