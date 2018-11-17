/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 *
 * Some code adopted from Robert Olsson <robert@herjulf.se> and Manee
 * @Author: 
 * Nsabagwa Mary <mnsabagwa@cit.ac.ug>
 * Okello Joel <okellojoelocaye@gmail.com>,
 * Alinitwe Sandra Kamugisha <sandraalinitwe@gmail.com>
 * Byamukama Maximus <maximus.byamukama@gmail.com>
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "i2c.h"
#include "mcp3424.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t user_reg;
uint8_t missing_adc_value_flag = 1;
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
/*
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
  }
  return 0;*/
return enabled;
}





static uint16_t value(int type)
{
 uint16_t data;
uint16_t returned;
uint8_t data_low;
uint8_t data_high;
uint16_t command;
uint16_t value;
//default configuration
command = 0b00010000;
switch(type){
//type for channel number 
case 0:
command = command;
break;
case 1:
command |= (0b01<<5);
break;
case 2:
command |= (0b10<<5);
break;
case 3:
command |= (0b11<<5);
break;
}



 unsigned int ret;  
 ret = i2c_start(MCP3424_ADDR<<1); 
// set device address and read mode 
  if ( ret )   
{//failed to issue start condition, possibly no device found  
i2c_stop();      
} 
else
{

i2c_start_wait(MCP3424_ADDR<<1);
i2c_write(command);
i2c_start_wait((MCP3424_ADDR<<1)|1);
data_high = i2c_readAck();
data_low = i2c_readNak();
data = (data_high<<8) +(data_low);
		missing_adc_value_flag = 0;
		return data;

}
missing_adc_value_flag = 1;
return 0;
}


/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{

return 0;
}
uint8_t missing_adc_value(){
 return missing_adc_value_flag;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(mcp3424_sensor,"MCP3424_SENSOR", value, configure, status);
/*--------------------------------------------------------------------------*/
