/*
 * Copyright (c) 2017, Sustainable Communication Networks, University of Bremen.
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

/**
 * \file
 *         Driver file for RFID Tag UID reader implementation on Z1
 * \author
 *         Mine Cetinkaya <mine.cetinkaya@gmail.com>
 *         Jens Dede <jd@comnets.uni-bremen.de>
 */

#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/spi.h"
#include "platform/z1/platform-conf.h"
#include "sys/timer.h"
/*-------------------------------------------------------------------*/
#define	uchar	unsigned char
#define	uint	unsigned int

static struct timer rxtimer;

//Chip Select(SDA) pin of the reader is connected to to P5.1 (JP1B, pin 28 of z1) as Output
#define SDA_CS 1 
//SDA is enabled when output is set to 0 (Active Low)
#define SPI_CS_ENABLE()  (P5OUT &= ~BV(SDA_CS)) 
//SDA is disabled when output is set to 1 
#define SPI_CS_DISABLE() (P5OUT |= BV(SDA_CS)) 
//Unlike in other adaptations, Reset pin of MFRC522 is not used throughout our code
//Reset Pin of the reader is connected to a 3V power supply, so that hard reset is totally avoided

//Pin layout of z1 and MFRC522 Reader is as follows:

/*Signal         MFRC522 Reader Pin      Zolertia z1 pin
 *					  JP1B East Port
 *---------------------------------------------------------
 *RST/Reset	     RST		 Pin#22/Pin Name: D+3V
 *SPI CS/SS/SDA      SDA		 Pin#28/Pin Name: I2C.SDA
 *SPI MOSI	     MOSI		 Pin#36/Pin Name: SPI.SIMO
 *SPI MISO	     MISO		 Pin#38/Pin Name: SPI.SOMI
 *SPI SCK/CLK        SCK		 Pin#34/Pin Name: SPI.CLK
 *GROUND	     GND		 Pin#18/Pin Name: CPDGND
 *3.3V		     3.3V		 Pin#20/Pin Name: CPD+3.3V
 */

//Maximum length of the array for the Tag UID reading
#define MAX_LEN 16

//MFRC522 Command word
#define PCD_IDLE              0x00               //NO action; Cancel the current command
#define PCD_RECEIVE           0x08               //Receive Data
#define PCD_TRANSCEIVE        0x0C               //Transmit and receive data
#define PCD_AUTHENT           0x0E               //Authentication Key
#define PCD_RESETPHASE        0x0F               //Reset

//MFRC522 Register

#define     VersionReg            0x37 << 1
#define     TxControlReg          0x14 << 1
#define     TModeReg              0x2A << 1
#define     TPrescalerReg         0x2B << 1
#define     TReloadRegH           0x2C << 1
#define     TReloadRegL           0x2D << 1
#define     ModeReg               0x11 << 1
#define     TxAutoReg             0x15 << 1
#define     CommandReg            0x01 << 1
#define     CollReg               0x0E << 1
#define     WaterLevelReg         0x0B << 1
#define     BitFramingReg         0x0D << 1
#define     CommIEnReg            0x02 << 1
#define     CommIrqReg            0x04 << 1
#define     FIFOLevelReg          0x0A << 1
#define     FIFODataReg           0x09 << 1
#define     ErrorReg              0x06 << 1
#define     ControlReg            0x0C << 1

//MIFARE One card Command word
#define PICC_REQALL          0x52        // find all the cards antenna area
#define PICC_REQIDL          0x26        // find the antenna area doesn't enter hibernation
#define PICC_ANTICOLL        0x93        // anti-collision

#define END_BYTE 0x00 << 1 // Stop command for SPI Write


//And MF522 The error code is returned at the communication instance
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2
/*-------------------------------------------------------------------*/

//function definitions
void MFRC522_Init();
void Write_MFRC522(uchar addr, uchar val);
void MFRC522_Reset();
void AntennaOn();
void SetBitMask(uchar reg, uchar mask);
uchar Read_MFRC522(uchar);
uchar MFRC522_Request(uchar reqMode, uchar *TagType);
void ClearBitMask(uchar reg, uchar mask);  
 uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen);
uchar MFRC522_Anticoll(uchar *serNum);
/*-------------------------------------------------------------------*/

void MFRC522_Init(void)
{

 
 MFRC522_Reset();		
 Write_MFRC522(TModeReg, 0x80);		
 Write_MFRC522(TPrescalerReg, 0xA9); 			
 Write_MFRC522(TReloadRegH, 0x03);			
 Write_MFRC522(TReloadRegL, 0xE8);
 Write_MFRC522(TxAutoReg, 0x40);     
 Write_MFRC522(ModeReg, 0x3D);        
 AntennaOn();
  	
}
/*-------------------------------------------------------------------*/
void MFRC522_Reset(void)
{
  Write_MFRC522(CommandReg, PCD_RESETPHASE);
  timer_set(&rxtimer, CLOCK_SECOND/20);
  while(!timer_expired(&rxtimer)){}
  while(Read_MFRC522(CommandReg)&(1<<4)){}

}
/*-------------------------------------------------------------------*/
void AntennaOn(void)
{
  uchar temp;
  temp= Read_MFRC522(TxControlReg);
  SetBitMask(TxControlReg, 0x03);
  if((temp & 0x03)!=0x03){
   Write_MFRC522(TxControlReg,temp|0x03);
  }
}
/*-------------------------------------------------------------------*/

void Write_MFRC522(uchar addr, uchar val) 
{
  spi_init();	
  P5SEL |= BV(SDA_CS);
  P5DIR|=BV(SDA_CS);
  SPI_CS_DISABLE();	
  timer_restart(&rxtimer);
  while(!timer_expired(&rxtimer)){}

  SPI_CS_ENABLE();  
  SPI_WRITE_FAST(((addr)&(0x7E)));	
  SPI_WRITE_FAST(val);
  SPI_WRITE_FAST(END_BYTE);
  SPI_CS_DISABLE(); 
}

/*-------------------------------------------------------------------*/

void SetBitMask(uchar reg, uchar mask)  
{
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp | mask);  
}

/*-------------------------------------------------------------------*/

uchar Read_MFRC522(uchar addr) 
{
  uchar *rx_bits;
  
  spi_init();	
  P5SEL |= BV(SDA_CS);
  P5DIR|=BV(SDA_CS);
  SPI_CS_DISABLE();
  timer_restart(&rxtimer);
  while(!timer_expired(&rxtimer)){}
  SPI_CS_ENABLE();
  SPI_WRITE_FAST((0x80)|((addr)&(0x7E)));	
  SPI_WRITE_FAST(END_BYTE);
  SPI_FLUSH();
  SPI_READ(rx_bits);
  SPI_CS_DISABLE();  
  
  return (uchar) rx_bits; 
}
/*-------------------------------------------------------------------*/
uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;
  uint backBits; 
  Write_MFRC522(BitFramingReg, 0x07);   // TxLastBists = BitFramingReg[2..0]
  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
  if ((status != MI_OK) || (backBits != 0x10)) {
    status = MI_ERR;
  }

  return status;
}
/*-------------------------------------------------------------------*/
 uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
  uchar status = MI_ERR;
  uchar irqEn = 0x00;
  uchar waitIRq = 0x00;
  uchar lastBits;
  uchar n;
  uint i;

  switch (command)
  {
    case PCD_AUTHENT:     // Certification cards close
      {
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
      }
    case PCD_TRANSCEIVE:  // Transmit FIFO data
      {
        irqEn = 0x77;
        waitIRq = 0x30;
	break;
      }
    default:
      break;
  }
 Write_MFRC522(CommIEnReg, irqEn|0x80);  // Interrupt request
 ClearBitMask(CommIrqReg, 0x80);         // Clear all interrupt request bit
 SetBitMask(FIFOLevelReg, 0x80);         // FlushBuffer=1, FIFO Initialization
 Write_MFRC522(CommandReg, PCD_IDLE);  
 // Writing data to the FIFO
 for (i=0; i<sendLen; i++)
 {
  Write_MFRC522(FIFODataReg, sendData[i]);
 }

 // Execute the command
 Write_MFRC522(CommandReg, command);
 if (command == PCD_TRANSCEIVE)
 {
  SetBitMask(BitFramingReg, 0x80);      // StartSend=1,transmission of data starts  
 }

 // Waiting to receive data to complete
 i=2000;
 do
 {
 // CommIrqReg[7..0]
 // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
 n = Read_MFRC522(CommIrqReg);
 i--;
 }while ((i!=0) && !(n & (0x01)) && !(n&waitIRq));
	
 if (i != 0)
  {
   	
   if(!(Read_MFRC522(ErrorReg) & 0x13))  //Collerr CRCErr ProtecolErr
    {
     status = MI_OK;
     if (n & irqEn & 0x01)
      {
       status = MI_NOTAGERR;             
      }
     if (command == PCD_TRANSCEIVE)
      {
        n = Read_MFRC522(FIFOLevelReg);
        lastBits = Read_MFRC522(ControlReg) & 0x07;		
        if (lastBits)
         {
           *backLen = (n-1)*8 + lastBits;
       	 }
       	else
       	 {
           *backLen = n*8;
         }

       	if (n == 0)
       	 {
           n = 1;
         }
        if (n > MAX_LEN)
         {
           n = MAX_LEN;
         }
        // Reading the received data in FIFO
        for (i=0; i<n; i++)
         {
           backData[i] = Read_MFRC522(FIFODataReg);
         }
      }
    }
     
   else 
    {
     printf("~~~ collerr, crcerr, or protecolerr\r\n");
     status = MI_ERR;
    }
  }
  return status;
}

/*-------------------------------------------------------------------*/

void ClearBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    uchar invertmask=~mask;
    Write_MFRC522(reg, tmp & invertmask);  // clear bit mask
} 
/*-------------------------------------------------------------------*/
uchar MFRC522_Anticoll(uchar *serNum)
{
  uchar status;
  uchar i;
  uchar serNumCheck=0;
  uint unLen;
  Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK)
  {
    //Check card serial number
    for (i=0; i<4; i++)
    {   
      serNumCheck ^= serNum[i];
    }
	
    if (serNumCheck != serNum[i])
    {   
      status = MI_ERR;    
    }
    
   }

  return status;
} 
/*-------------------------------------------------------------------*/
