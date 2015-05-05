/* Copyright (c) 2009  ARAGO SYSTEMS
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
#include "dev/sht15.h"

#define DATA_OUT() GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(4))
#define DATA_IN()  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(4))
#define DATA_SET() GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(4)); clock_delay_usec(10)
#define DATA_CLR() GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(4)); clock_delay_usec(10)
#define DATA_VAL() GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(4))

#define SCK_OUT() GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3))
#define SCK_SET() GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3)); clock_delay_usec(10)
#define SCK_CLR() GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(3)); clock_delay_usec(10)


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Driver file for SHT75 Temperature & Humidity Sensor                       //
//                                                                           //
// ***** To initialise SHT75 sensor upon power up *****                      //
//                                                                           //
// Function : sht_init()                                                     //
// Return   : none                                                           //
//                                                                           //
//                                                                           //
// ***** To measure and caluculate SHT75 temp & real RH *****                //
//                                                                           //
// Function : sht_rd (temp, truehumid)                                       //
// Return   : temperature & true humidity in float values                    //
//                                                                           //
/////////////////////////////////////////////////////////////////////////////// 

#include "SHT15.h"
//***** Function to alert SHT75 *****

void comstart (void)
{
    Set_DataOutToSTH75_Hi();  //data high
    SCK_CLR();  //clk low
    clock_delay_usec(1);
    SCK_SET();  //clk high
    clock_delay_usec(1);
    Set_DataOutToSTH75_Lo(); //data low
    clock_delay_usec(1);
    SCK_CLR();   //clk low
    clock_delay_usec(2);
    SCK_SET();  //clk high
    clock_delay_usec(1);
    Set_DataOutToSTH75_Hi();  //data high
    clock_delay_usec(1);
    SCK_CLR();   //clk low
}


//***** Function to write data to SHT75 *****

int comwrite (char iobyte)
{
    char i, mask = 0x80;
    int ack;

    //Shift out command
    clock_delay_usec(4);
    for(i=0; i<8; i++)
    {
        SCK_CLR(); //clk low
        if((iobyte & mask) > 0) Set_DataOutToSTH75_Hi();  //data high if MSB high
        else Set_DataOutToSTH75_Lo();                    //data low if MSB low
        clock_delay_usec(1);
        SCK_SET();                          //clk high
        clock_delay_usec(1);
        mask = mask >> 1;                                    //shift to next bit
    }

    //Shift in ack
    SCK_CLR();  //clk low
    clock_delay_usec(1);
    ack = DATA_VAL();   //get ack bit
    SCK_SET();  //clk high
    clock_delay_usec(1);
	SCK_CLR();  //clk low
    return(ack);
}


//***** Function to read data from SHT75 *****

int comread (void)
{
    char i;
    int iobyte = 0;
    const int mask0 = 0x0000;
    const int mask1 = 0x0001;
	
	DATA_IN();
    //shift in MSB data
    for(i=0; i<8; i++)
    {
        iobyte = iobyte << 1;
        SCK_SET();                //clk high
        clock_delay_usec(1);
        if (DATA_VAL()) iobyte |= mask1;  //shift in data bit
        else iobyte |= mask0;
        SCK_CLR();                //clk low
        clock_delay_usec(1);
    }

    //send ack 0 bit
    Set_DataOutToSTH75_Lo(); //data low
    clock_delay_usec(1);
    SCK_SET();  //clk high
    clock_delay_usec(2);
    SCK_CLR();  //clk low
    clock_delay_usec(1);
    Set_DataOutToSTH75_Hi();  //data high

    //shift in LSB data
    for(i=0; i<8; i++)
    {
        iobyte = iobyte << 1;
        SCK_SET();                //clk high
        clock_delay_usec(1);
        if (DATA_VAL()) iobyte |= mask1;  //shift in data bit
        else iobyte |= mask0;
        SCK_CLR();                //clk low
        clock_delay_usec(1);
    }

    //send ack 1 bit
    Set_DataOutToSTH75_Hi();  //data high
    clock_delay_usec(1);
    SCK_SET();  //clk high
    clock_delay_usec(2);
    SCK_CLR();  //clk low

    return(iobyte);
}


//***** Function to wait for SHT75 reading *****

void comwait (void)
{
    int sht_delay;

    Set_DataOutToSTH75_Hi();                     //data high
    SCK_CLR();                     //clk low
    clock_delay_usec(1);
    for(sht_delay=0; sht_delay<300000; sht_delay++)  // wait for max 300ms
    {
        if (DATA_VAL() == 0) 
            break;              //if sht_data_pin low, SHT75 ready
        clock_delay_usec(1);
    }
}


//***** Function to reset SHT75 communication *****

void comreset (void)
{
    char i;

    Set_DataOutToSTH75_Hi();    //data high
    SCK_CLR();    //clk low
    clock_delay_usec(2);
    for(i=0; i<9; i++)
    {
        SCK_SET();  //toggle clk 9 times
        //clock_delay_usec(2);
        SCK_CLR();
        //clock_delay_usec(2);
    }
    comstart();
}


//***** Function to soft reset SHT75 *****

void sht_soft_reset (void)
{
    comreset();           //SHT75 communication reset
    comwrite(0x1e);       //send SHT75 reset command
    clock_delay_usec(15);         //pause 15 ms
}


//***** Function to measure SHT75 temperature *****

int measuretemp (void)
{
    int ack;
    int iobyte;

    comstart();             //alert SHT75
    ack = comwrite(0x03);   //send measure temp command and read ack status
    if(ack == 1) return 100;
    comwait();              //wait for SHT75 measurement to complete
    iobyte = comread();     //read SHT75 temp data
    return(iobyte);
}


//***** Function to measure SHT75 RH *****

int measurehumid (void)
{
    int ack;
    int iobyte;

    comstart();            //alert SHT75
    ack = comwrite(0x05);  //send measure RH command and read ack status
    if(ack == 1) return 100;
    comwait();             //wait for SHT75 measurement to complete
    iobyte = comread();    //read SHT75 temp data
    return(iobyte);
}


//***** Function to calculate SHT75 temp & RH *****

void calculate_data(int temp, int humid, float *tc, float *rhlin, float *rhtrue)
{
    float rh;

    //calculate temperature reading
    *tc = ((float) temp * 0.01) - 39.65;

    //calculate Real RH reading
    rh = (float) humid;
    
    //*rhlin = (rh * 0.0405) - (rh * rh * 0.0000028) - 4.0;
    *rhlin = -2.0468 + (0.0367 * rh) + (-0.0000015955 * rh * rh);

    //calculate True RH reading
    *rhtrue = (*tc - 25) * (0.01 + 0.00008 * rh) + *rhlin;
}


//***** Function to measure & calculate SHT75 temp & RH *****

void sht_rd (float *temp, float *truehumid)
{
    int restemp, reshumid;
    float realhumid;
    restemp = 0; *truehumid = 0;

    restemp = measuretemp();    //measure temp
    reshumid = measurehumid();  //measure RH
    calculate_data (restemp, reshumid, temp, &realhumid, truehumid);  //calculate temp & RH
}


//***** Function to initialise SHT75 on power-up *****

void sht_init (void)
{
	  /* DATA and SCK lines are I/O */
  GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, 3);
  GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, 4);
  /* Set SCK and DATA as output */
  DATA_OUT();
  SCK_OUT();
  DATA_SET();
  SCK_SET();
  
    comreset();    //reset SHT75
    clock_delay_usec(20);  //delay for power-up
}

//.............................................................................

void Set_DataOutToSTH75_Lo(void)
{
	DATA_OUT();
	DATA_CLR();
    clock_delay_usec(5);
}

//.............................................................................

void Set_DataOutToSTH75_Hi(void)
{
	DATA_OUT();
	DATA_SET();
    clock_delay_usec(5);
}
