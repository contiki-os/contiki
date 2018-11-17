
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
 *
 * Some code adopted from Robert Olsson <robert@herjulf.se> and Manee
 * @Author: 
 * Nsabagwa Mary <mnsabagwa@cit.ac.ug>
 * Okello Joel <okellojoelocaye@gmail.com>,
 * Alinitwe Sandra Kamugisha <sandraalinitwe@gmail.com>
 * Byamukama Maximus <maximus.byamukama@gmail.com>
 */

#ifndef MCP3424_SENSOR_H_
#define MCP3424_SENSOR_H_

#include "lib/sensors.h"
#define TRUE 1
#define FALSE 0

#define SCL_CLOCK  100000L  //stardard mode  100khz

extern const struct sensors_sensor mcp3424_sensor;
#define MCP3424_ADDR 0x68   //(0x68<< 1) 0 1101 000 the last 3 bits represent adr0 and adr1 set low
#define MCP342X_START 0X80 // write: start a conversion
#define MCP342X_BUSY 0X80 // read: output not ready
typedef enum { false, true } bool;
uint8_t missing_adc_value();

//address (input) I2C bus address of the device
//gain (input) Measurement gain multiplier (1,2,4,8)
//resolution (input) ADC resolution (0,1,2,4)
void mcp3424_init(uint8_t address, uint8_t channel, uint8_t gain, uint8_t resolution);


// Start a one-shot channel measurement
// @param channel (input) Analog input to read (1-4)
void mcp3424_start_measure();

// Test if a measurement is ready. Use getMeasurement() to return
// the value of the result.
// @return True if measurement was ready, false otherwise
bool is_measurement_ready();

// Get the result of the last channel measurement
// @param value (output) Measured value of channel, in mV
int32_t get_measurement_uv();

// Get the divisor for converting a measurement into mV
int get_mv_divisor();
#endif /* MCP3424-SENSOR_H_ */



