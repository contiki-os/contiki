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
 *  Adopted from  https://www.parallax.com/sites/default/files/downloads/29124-APPNote_520_C_code.pdf
 * by Mary Nsabagwa <mnsabagwa@cit.ac.ug> and Ian Tukesiga <tukian04@gmail.com >
 *
 */

#include "contiki.h"
#include "ms5611.h"
#include "i2c.h"
#include <stdio.h>
#include <util/delay.h> 
#include <util/twi.h> 
#include <avr/io.h>
#include <math.h>

 static uint8_t missing_p_value_flag = 1;
 uint8_t ac_value_flag = 0;
 uint8_t nac_value_flag =0;
/*Send I2C Reset command */
void cmd_reset(void) {   
  i2c_send(CMD_RESET);   
  _delay_ms(3);     

 } 

/* command for adc conversion*/
unsigned long cmd_adc(char cmd) {
  unsigned int ret; 
  unsigned long temp=0;         
  i2c_send(CMD_ADC_CONV+cmd);
  // wait necessary conversion time 
  switch (cmd & 0x0f) { 
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
 	 i2c_stop();      }  
  else   {//issuing start condition ok, device accessible    
  	ret = i2c_readAck(); 
        ac_value_flag= ret; // read MSB and acknowledge  
  	temp=65536*ret; 
  	ret = i2c_readAck();    // read byte and acknowledge       
  	temp=temp+256*ret;  
 	ret = i2c_readNak();  
        nac_value_flag= ret; // read LSB and not acknowledge   
  	temp=temp+ret;    
  	i2c_stop();     // send stop condition
        missing_p_value_flag = 0;
       return temp; 
  }
 missing_p_value_flag = 1;  
 return temp; 

}

/* Read PROM for the coefficients*/ 
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

/*Getting the CRC*/
  unsigned char crc4(unsigned int n_prom[]) {   
  int cnt;     // simple counter    
  unsigned int n_rem;    // crc reminder 
  unsigned int crc_read;   // original value of the crc 
  unsigned char  n_bit; 
  n_rem = 0x00;   
  crc_read=n_prom[6];    //save read CRC  
  n_prom[6]=(0xFF00 & (n_prom[6]));  //CRC byte is replaced by 0    
  for (cnt = 0; cnt < 16; cnt++)      // operation is performed on bytes   
  {// choose LSB or MSB 
  if (cnt%2==1) 
	n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);   
  else 
	n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);       
  for (n_bit = 8; n_bit > 0; n_bit--)         {   
  	if (n_rem & (0x8000))                  {           
 		 n_rem = (n_rem << 1) ^ 0x3000; 
   	}                 
 	else {               
  	 n_rem = (n_rem << 1);     
             }        
 	}     
 }    
  n_rem=  (0x000F & (n_rem >> 12));  // final 4-bit reminder is CRC code     
  n_prom[6]=crc_read;   // restore the crc_read to its original place    
  return (n_rem ^ 0x0); 
}  
 
/**/
static int value (int type) { 
 //static uint16_t value(int type){
int pressure;
   unsigned long D1;    // ADC value of the pressure conversion 
   unsigned long D2;    // ADC value of the temperature conversion  
   unsigned int C[8];   // calibration coefficients 
   float P;   // compensated pressure value 
//   double T;   // compensated temperature value 
   double dT;   // difference between actual and measured temperature 
   double OFF;   // offset at actual temperature  
   double SENS;   // sensitivity at actual temperature 
   int i; 
  // unsigned char n_crc; // crc value of the prom    // setup the ports
   D1=0; 
   D2=0;  
   cmd_reset();     // reset IC  
   for (i=0;i<8;i++){ 
  	C[i]=cmd_prom(i);
   } // read coefficients 
   crc4(C);    // calculate the CRC  
   D2=cmd_adc(CMD_ADC_D2+CMD_ADC_4096);   // read D2  
   D1=cmd_adc(CMD_ADC_D1+CMD_ADC_4096);   // read D1  
  // calculate 1st order pressure and temperature (MS5607 1st order algorithm)  
  dT=D2-C[5]*pow(2,8);  
  OFF=C[2]*pow(2,17)+dT*C[4]/pow(2,6);     
  SENS=C[1]*pow(2,16)+dT*C[3]/pow(2,7);    
  //T=(2000+(dT*C[6])/pow(2,23))/100;  
   P = (((D1 * SENS/2097152 - OFF)/32768)/100)-1000.0;
   // if(type==0){
         pressure = P;
        return pressure;  


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

  return 0;
}
uint8_t missing_p_value()
{
 return missing_p_value_flag;
}
uint8_t nac_value()
{
 return nac_value_flag;
}
uint8_t ac_value()
{
 return ac_value_flag;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ms5611_sensor, "Pressure", value, configure, status);
