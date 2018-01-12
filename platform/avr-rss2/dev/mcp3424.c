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
 */

#include "contiki.h"
#include "mcp3424.h"
#include "i2c.h"
#include "i2c_utilities.h"
//#include "uart.h"
#include <stdio.h>
 #include <util/delay.h> 
 #include <util/twi.h> 
 #include <avr/io.h>
 #include <math.h>


const struct sensors_sensor mcp3424_sensor;
    
int address;      // I2C bus address
uint8_t resolution;   // ADC resolution
uint8_t gain;         // Gain
int32_t value1;                  // Last measurement result, in uV
char str[15]; //temp

void mcp3424_init(uint8_t _address, uint8_t channel, uint8_t _gain, uint8_t _resolution) 
{
	// Configure the device for one-shot mode. Resolution and gain don't matter here.
  	i2c_init(F_SCL);
	address = _address; gain=_gain; resolution=_resolution;	
	unsigned char PGAgain = 0;
	unsigned char sampleRate = 0;
	unsigned char conversionModeBit = 1; //1=Continuous, 0=One Shot
	unsigned char channelBits = 0; //0 = Channel 1
	channelBits = channel - 1; //zero based
	switch(gain) {
		
            case 8:
		PGAgain = 0x03;
	    break;
	    case 4:
		PGAgain = 0x02;
	    break;
	    case 2:
		PGAgain = 0x01;
	     break;
	     case 1:
		PGAgain = 0x00;
	     break;
	     default:
		PGAgain = 0x0;
             break;
	}
	
	switch(resolution) {
		case 18:
			sampleRate = 0x03; //3.75 sps (18 bits), 3 bytes of data
		break;
		case 16:
			sampleRate = 0x02; //2 bytes of data,
		break;
		case 14:
			sampleRate = 0x01; //2 bytes of data
		break;
		case 12:
			sampleRate = 0x00; //240 SPS (12 bits), 2 bytes of data
		break;
	default:
		sampleRate = 0x00;


}

	unsigned char config = PGAgain;
	config = config | (sampleRate << 2);
	config = config | (conversionModeBit << 4);
	config = config | (channelBits << 5);
	config = config | (1 << 7); //write a 1 here to initiate a new conversion in One-shot mode
	
	printf(str, "The address seems to be addr-%d config - %d ", address,config);
	//uart_puts(str);

	
	i2c_start(address | I2C_WRITE);
	printf("i2c started");
	i2c_write(config);        /*ATTN: WE HAVE AN ERROR HERE*/
	printf("i2c write complete");
	i2c_stop();
	
	printf("config has been set");
}

// calculate and return mV divisor from gain and resolution.
int get_mv_divisor()
{
	int mvdivisor = 1 << (gain + 2*resolution);
	return mvdivisor;
}


void mcp3424_start_measure() {
	i2c_start_wait(address | I2C_WRITE);
	i2c_write(MCP342X_START);
	i2c_stop();
}


bool is_measurement_ready() {
printf("inside measurement method\n");
	// Assume <18-bit mode. Note that this needs to handle 
	// 4-byte responses to work with 18-bit mode.
	int16_t rawvalue = 0;
	// Ask for 3 bytes from the sensor
	i2c_start(address | I2C_READ);
	// Read the first two bytes into the correct positions
	// for an int16_t

         uint8_t i;
	for (i = 0; i < 2; i++) {
printf("for loop \n");
		rawvalue = (rawvalue << 8) | i2c_readNak();
	}
printf("outside for loop \n");
	// read config/status byte
	uint8_t status = i2c_readAck();
	i2c_stop();
printf("after i2c stop \n");

	if (status & MCP342X_BUSY) {
printf("if loop 1\n");
		return false;
	}

	// If we're out of range, set the output to INT_MAX as a signal
	if(rawvalue == 0x7FFF) {
printf("if loop 2\n");
		value1 = 0x7FFFFFFF;
	}
	else if(rawvalue == -0x8000) {
printf("if loop 3\n");
		value1 = -0x80000000;
	}
	else {
printf("if loop 4\n");
		value1 = (((int32_t)rawvalue)*1000)/get_mv_divisor();
	}

	return true;
}

int32_t get_measurement_uv() {
	return value1;
}

 
/*---------------------------------------------------------------------------*/
static int
value()
{

return 0;

 
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
    char str[150];
	int len=0,chan=1;
	int32_t result=0;	

	while(1){
		len=0;
		if(chan>4) chan=1;
		while(chan <=4) 
		{
    	 mcp3424_start_measure(chan);

			while(!is_measurement_ready());

			result = get_measurement_uv();	

		    len += sprintf(&str[len],"V on CHAN %d is %ld\n",chan,result); 

			printf("chan incremented\n");

			++chan;
		}
		printf(str);
		_delay_ms(1000);
	}

  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(mcp3424_sensor, "MCP3424", value, configure, status);
