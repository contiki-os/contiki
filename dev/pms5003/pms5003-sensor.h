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
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Peter Sjodin, KTH Royal Institute of Technology
 * Created : 2017-04-21
 */

#ifndef PMS5003_SENSOR_H_
#define PMS5003_SENSOR_H_

#include "lib/sensors.h"

extern const struct sensors_sensor pms5003_sensor;

#define PMS5003_SENSOR_PM1           0
#define PMS5003_SENSOR_PM2_5         1
#define PMS5003_SENSOR_PM10          2
#define PMS5003_SENSOR_PM1_ATM       3
#define PMS5003_SENSOR_PM2_5_ATM     4
#define PMS5003_SENSOR_PM10_ATM      5
#define PMS5003_SENSOR_DB0_3         6
#define PMS5003_SENSOR_DB0_5         7
#define PMS5003_SENSOR_DB1            8
#define PMS5003_SENSOR_DB2_5         9
#define PMS5003_SENSOR_DB5          10
#define PMS5003_SENSOR_DB10          11
#define PMS5003_SENSOR_TIMESTAMP    12

#endif /* PMS5003_SENSOR_H_ */
