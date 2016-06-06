/*
 * Copyright (c) 2015, ICT/COS/NSLab, KTH Royal Institute of Technology
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
 *      dcdc/hwcfg for DC-DC converter hardware configuration
 * \author
 *      Voravit Tanyingyong <voravit@kth.se>
 */

#include "contiki.h"
/* #include "lib/sensors.h" */
#include "dev/dc-hw-sensor.h"

const struct sensors_sensor dc_hw_sensor;

/*
 * hw contains 2 parameters
 * hw[0] VMAX
 * hw[1] IMAX
 */

uint32_t volatile hw[2] = { 25, 6 };

static int
value(int type)
{
  switch(type) {
  case 0:
    return hw[0];

  case 1:
    return hw[1];
  }
  return -1;
}
static int
configure(int type, int c)
{
  switch(type) {
  case 0:
    if((c >= 0) && (c <= 25)) {
      hw[0] = c;
      return 0;
    } else {
      return 1;
    }

  case 1:
    if((c >= 0) && (c <= 6)) {
      hw[1] = c;
      return 0;
    } else {
      return 0;
    }
  }
  return 1;
}
static int
status(int type)
{
  return 1;
}
SENSORS_SENSOR(dc_hw_sensor, "hw sensors", value, configure, status);

