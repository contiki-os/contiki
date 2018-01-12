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
 * MS5611 Pressure sensor
 * \Authors: Ian Tukesiga, Mary Nsabagwa  <mnsabagwa@cit.ac.ug>
 *
 */

#include "contiki.h"
#include "dev/i2c.h"
#include "dev/ms5611.h"
#include <stdio.h>
#include <util/delay.h> 
#include <util/twi.h> 
#include <avr/io.h>
#include <math.h>

/*  */
void cmd_reset(void) {   
   i2c_send(CMD_RESET);   
   // send reset sequence 
   _delay_ms(3);      } 
   unsigned long cmd_adc(char cmd) {
   unsigned int ret; 
   unsigned long temp=0;         
   i2c_send(CMD_ADC_CONV+cmd);
   switch (cmd & 0x0f)     // wait necessary conversion time 
   { 
	case CMD_ADC_256 : 
		_delay_us(900); 
        break;   
	case CMD_ADC_512 : 
		_delay_ms(3); 
 	break;   
	case CMD_ADC_1024:
		 _delay_ms(4);  
 	break;   
	case CMD_ADC_2048: 
		_delay_ms(6);  
	break;  
        case CMD_ADC_4096: 
		_delay_ms(10); 
 	break; 
 } 
  i2c_send(CMD_ADC_READ); 
  ret = i2c_start(ADDR_R); 
  if ( ret )   {//failed to issue start condition, possibly no device found     
  	i2c_stop();     
  }  else   {//issuing start condition ok, device accessible    
 	ret = i2c_readAck();    // read MSB and acknowledge  
  	temp=65536*ret; 
  	ret = i2c_readAck();    // read byte and acknowledge       
        temp=temp+256*ret;  
  	ret = i2c_readNak();   // read LSB and not acknowledge   
  	temp=temp+ret;    
  	i2c_stop();     // send stop condition 
  }  
	return temp; 
} 

/* prom  */
unsigned int cmd_prom(char coef_num) {  
  unsigned int ret;  
  unsigned int rC=0; 
  i2c_send(CMD_PROM_RD+coef_num*2);  // send PROM READ command
  ret = i2c_start(ADDR_R);         // set device address and read mode 
  if ( ret )   {//failed to issue start condition, possibly no device found     
 	 i2c_stop();     
  }  else   {//issuing start condition ok, device accessible   
 	 ret = i2c_readAck();             // read MSB and acknowledge               
 	 rC=256*ret;  
 	 ret = i2c_readNak();   // read LSB and not acknowledge  
 	 rC=rC+ret;  
 	 i2c_stop();   
 }  
return rC; 
}

/* Calculating the CRC */ 
unsigned char crc4(unsigned int n_prom[]) {    
 int cnt;     // simple counter    
 unsigned int n_rem;    // crc reminder 
 unsigned int crc_read;   // original value of the crc 
 unsigned char  n_bit; 
 n_rem = 0x00;   
 crc_read=n_prom[7];    //save read CRC  
 n_prom[7]=(0xFF00 & (n_prom[7]));  //CRC byte is replaced by 0    
  for (cnt = 0; cnt < 16; cnt++)      // operation is performed on bytes   
  {// choose LSB or MSB 
 	 if (cnt%2==1) 
		n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);   
  	 else 
		n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);       
  for (n_bit = 8; n_bit > 0; n_bit--)         {   
  	if (n_rem & (0x8000))                  {           
  	n_rem = (n_rem << 1) ^ 0x3000; 
   }                  else                  {               
   n_rem = (n_rem << 1);                  }         }      }      n_rem=  (0x000F & (n_rem >> 12));  // final 4-bit reminder is CRC code     
   n_prom[7]=crc_read;   // restore the crc_read to its original place    
   return (n_rem ^ 0x0); 
}  

/*Calculate Pressure*/ 
float value(int type) { 
   unsigned long d1;    // ADC value of the pressure conversion 
   unsigned long d2;    // ADC value of the temperature conversion  
   unsigned int c[8],i;   // calibration coefficients 
   double p;   // compensated pressure value 
   double dt;   // difference between actual and measured temperature 
   double off;   // offset at actual temperature  
   double sens;   // sensitivity at actual temperature  
   for (i=0;i<8;i++)
	c[i]=cmd_prom(i);// read coefficients 

   	d2=cmd_adc(CMD_ADC_D2+CMD_ADC_4096);   // read D2  
   	d1=cmd_adc(CMD_ADC_D1+CMD_ADC_4096);   // read D1  
  	// calcualte 1st order pressure and temperature (MS5607 1st order algorithm)
  	dt=d2-c[5]*pow(2,8);  
  	off=c[2]*pow(2,17)+dt*c[4]/pow(2,6); //OFF  = OFFT1 + TCO * dT = C2  * 216
                                             // + (C4 * dT ) / 27  
  	sens=c[1]*pow(2,16)+dt*c[3]/pow(2,7); 
       p = (((d1 * sens/2097152 - off)/32768)/100)-1000.0;   
  return p; 
}  

/*---------------------------------------------------------------------------*/
static int status(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ms5611_sensor, "MS5611", value, configure, status);
