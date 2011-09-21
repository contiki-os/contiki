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
/*
  $Id: v 0.1 2011/02/15 tchapelle Exp $
*/
#include "dev/sht15.h"

/***********************************************************************************
* @fn          halMcuWaitUs
*
* @brief       Busy wait function. Waits the specified number of microseconds. Use
*              assumptions about number of clock cycles needed for the various
*              instructions. The duration of one cycle depends on MCLK. In this HAL
*              , it is set to 8 MHz, thus 8 cycles per usec.
*
*              NB! This function is highly dependent on architecture and compiler!
*
* @param       uint16 usec - number of microseconds delay
*
* @return      none
*/

#pragma optimize=none
void halMcuWaitUs(uint16_t usec) // 5 cycles for calling
{
    // The least we can wait is 3 usec:
    // ~1 usec for call, 1 for first compare and 1 for return
    while(usec > 3)       // 2 cycles for compare
    {                // 2 cycles for jump
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        nop();       // 1 cycles for nop
        usec -= 2;        // 1 cycles for optimized decrement
    }
}                         // 4 cycles for returning


/**
    SHT15/75 Driver

  !!! be advise that the SHT15 and SHT75 are not i2C compliant sensors
      they are just designed to not disturb i2C devices on a 2 wire bus
      this driver allow to drive the sensor with GPIO and delay
*/

/***********************************************************************************
* @fn          sht15_send_start
*
* @brief       This function perform the start sequence asked by SHT15 and SHT75
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_send_start()
{
  // Sequence is to set data line to 1 then clock line to 1 
  // then set data line to 0, clock line to 0 
  // then set clock line to 1 and data line to 1
//                     ___________              ________
// data line :  _____/            \___________/
//                         ___________      ____________
// clock line : _________/            \___/
  
  DATA_OUT();
  DATA_SET();
  SCK_SET();
  DATA_CLR();
  SCK_CLR();
  SCK_SET();
  DATA_SET();
  SCK_CLR();
}
/***********************************************************************************
* @fn       sht15_read16()   
*
* @brief       
*
*              
*
* @param       none
*
* @return      uint16_t
*/
uint16_t sht15_read16()
{
uint16_t i;
DATA_IN();

SCK_CLR();
uint16_t val = 0;

   for(i = 0; i < 18; i++)
   {
           if((i != 8) && (i != 17))
           {
                   SCK_SET();
                   if(DATA_VAL())
                      val |= 1;

                   val <<= 1;
                   SCK_CLR();
           }
           else if(i == 8) // Wait for first ACK from SHT15
           {
                   DATA_OUT();

                   DATA_CLR();

                   SCK_SET();
                   SCK_CLR();

                   DATA_IN();
           }
           else if(i == 17) // Wait for second ACK from SHT15
           {
                   DATA_OUT();

                   DATA_SET();

                   SCK_SET();
                   SCK_CLR();
           }
   }
   return val;
}
/***********************************************************************************
* @fn          sht15_write8
*
* @brief       
*
*              
*
* @param       uint8 val
*
* @return      none
*/
void sht15_write8(uint8_t val)
{
       uint16_t i;

       DATA_OUT();

       for(i = 0; i < 8; i++)
       {
               halMcuWaitUs(4);
               SCK_CLR();

               if(val & 0x80)
               {
                       DATA_SET();
               }
               else
               {
                       DATA_CLR();
               }
               val <<= 1;

               SCK_SET();
       }

       DATA_IN();

       SCK_CLR();

       while(DATA_VAL());

       SCK_SET();
       SCK_CLR();
}
/***********************************************************************************
* @fn          sht15_wait_measure
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_wait_measure()
{
     while(DATA_VAL());
}
/***********************************************************************************
* @fn          sht15_init
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_init()
{
   // DATA and SCK lines are I/O
    P3SEL &= ~BIT7;
    P5SEL &= ~BIT4;
   // Set SCK and DATA as output
   DATA_OUT();
   SCK_OUT();
   DATA_SET();
   SCK_SET();
}
/***********************************************************************************
* @fn          sht15_measure_temp
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_measure_temp()
{
       sht15_send_start();
       sht15_write8(3);
}
/***********************************************************************************
* @fn          sht15_measure_hum
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_measure_hum()
{
       sht15_send_start();
       sht15_write8(5);
}
/***********************************************************************************
* @fn          sht15_read_status
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_read_status()
{
       sht15_send_start();
       sht15_write8(7);
}
/***********************************************************************************
* @fn          sht15_write_status
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_write_status()
{
       sht15_send_start();
       sht15_write8(6);
}
/***********************************************************************************
* @fn          sht15_soft_reset
*
* @brief       
*
*              
*
* @param       none
*
* @return      none
*/
void sht15_soft_reset()
{
       sht15_send_start();
       sht15_write8(30);
}
