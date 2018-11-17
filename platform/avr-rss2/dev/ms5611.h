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
 * Author  : Robert Olsson
 * Created : 2015-10-27
 * Updated : $Date: 2010/01/14 20:23:02 $
 *           $Revision: 1.2 $
 */

#ifndef MS5611_SENSOR_H_
#define MS5611_SENSOR_H_

#include "lib/sensors.h"
#define TRUE 1
#define FALSE 0

#define SCL_CLOCK  100000L   
 
 #define ADDR_W      0xEC     // Module address write mode
 #define ADDR_R 0xED     // Module address read mode  
#define CMD_RESET   0x1E  // ADC reset command
 #define CMD_ADC_READ 0x00  // ADC read command
 #define CMD_ADC_CONV 0x40  // ADC conversion command
 #define CMD_ADC_D1   0x00    // ADC D1 conversion
 #define CMD_ADC_D2   0x10    // ADC D2 conversion
 #define CMD_ADC_256  0x00    // ADC OSR=256
 #define CMD_ADC_512  0x02    // ADC OSR=512 
 #define CMD_ADC_1024 0x04    // ADC OSR=1024 
 #define CMD_ADC_2048 0x06    // ADC OSR=2048
 #define CMD_ADC_4096 0x08    // ADC OSR=4096 
 #define CMD_PROM_RD  0xA0  // Prom read command
extern const struct sensors_sensor ms5611_sensor;

  void cmd_reset(void);
  unsigned long cmd_adc(char cmd);
  unsigned int cmd_prom(char coef_num); 
  unsigned char crc4(unsigned int n_prom[]);
  uint8_t missing_p_value();

#endif /* LIGHT-SENSOR_H_ */
