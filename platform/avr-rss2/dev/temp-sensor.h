/*
 * Copyright (c) 2015, Copyright Per Lindgren <per.o.lindgren@gmail.com>
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
 * Author   : Per Lindgren <per.o.lindgren@gmail.com>
 * Hacked by: Robert Olsson robert@radio-sensors.com
 * Created : 2015-11-22
 */

#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

#include "lib/sensors.h"
#include <sys/clock.h>
#include "contiki.h"
#include "rss2.h"

#define DS18B20_1_PIN OW_BUS_0
#define DS18B20_1_IN  PIND
#define DS18B20_1_OUT PORTD
#define DS18B20_1_DDR DDRD

#define OW_SET_PIN_LOW()   (DS18B20_1_OUT &= ~(1 << DS18B20_1_PIN))
#define OW_SET_PIN_HIGH()  (DS18B20_1_OUT |= (1 << DS18B20_1_PIN))
#define OW_SET_OUTPUT()    (DS18B20_1_DDR |= (1 << DS18B20_1_PIN))
#define OW_SET_INPUT()     (DS18B20_1_DDR &= ~(1 << DS18B20_1_PIN))
#define OW_GET_PIN_STATE() ((DS18B20_1_IN & (1 << DS18B20_1_PIN)) ? 1 : 0)

#define DS18B20_COMMAND_READ_SCRATCH_PAD 0xBE
#define DS18B20_COMMAND_START_CONVERSION 0x44
#define DS18B20_COMMAND_SKIP_ROM 0xCC

/* probe_for_ds18b20 probes for the sensor. Returns 0 on failure, 1 on success
 * Assumption: only one sensor on the "1-wire bus", on port DS18B20_1_PIN */

extern uint8_t ds18b20_probe(void);
extern uint8_t  ds18b20_get_temp(double *temp);
extern uint8_t crc8_ds18b20(uint8_t *buf, uint8_t buf_len);

extern const struct sensors_sensor temp_sensor;

#define TEMP_SENSOR "temp"

#endif /* TEMP_SENSOR_H_ */
