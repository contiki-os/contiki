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
 * \addtogroup zoul-sht25-sensor
 * @{
 *
 * \file
 *         SHT25 temperature and humidity sensor driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         SHT25 temperature and humidity sensor driver
 * Authors : Joel Okello <okellojoelacaye@gmail.com>, Mary Nsabagwa <mnsabagwa@cit.ac.ug
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "i2c.h"
#include "sht25.h"
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
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
	return enabled;
}


static uint16_t value(int type){
 uint16_t returned;
 uint16_t data_low;
 uint16_t data_high;
 uint16_t command;
 unsigned int ret;
 float data;

 if(type==0)
      command = SHT25_TEMP_HOLD;
 else 
      command = SHT25_HUM_HOLD;
  
 ret = i2c_start(SHT25_ADDR<<1);         // set device address and read mode 
 if ( ret )  {
//failed to issue start condition, possibly no device found     
	i2c_stop();    
} 
else{

	i2c_start_wait(SHT25_ADDR<<1);
	i2c_write(SHT25_TEMP_HOLD);
	i2c_start_wait((SHT25_ADDR<<1)|1);
	data_high = i2c_readAck();
	data_low = i2c_readNak();
	if(type==0){
		float temp = (data_high<<8) +(data_low);	
		data =    ((temp/65536)* 175.72)-46.85;
		//returned = data;
		return data;
         }

	if(type==1){
		float temp = (data_high<<8) +(data_low);
		data = ((temp/(65536)) * 125)-6;
		//returned = data1;
		return data;
 	}
  }//else 

}


/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{

return SHT25_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sht25_sensor,SHT25_SENSOR, value, configure, status);
/*--------------------------------------------------------------------------*/
/** @} */

