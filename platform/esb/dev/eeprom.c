/**
 * \file
 * EEPROM functions.
 * \author Adam Dunkels <adam@sics.se> 
 */

/* Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: eeprom.c,v 1.1 2006/06/18 07:49:33 adamdunkels Exp $
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
Copyright 2003/2004, Freie Universitaet Berlin. All rights reserved.
                                                                                                                                     
These sources were developed at the Freie Universit\x{FFFF}t Berlin, Computer
Systems and Telematics group.
                                                                                                                                     
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
                                                                                                                                     
- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
                                                                                                                                     
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
                                                                                                                                     
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
                                                                                                                                     
This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.
                                                                                                                                     
This implementation was developed by the CST group at the FUB.
Contributors: Thomas Pietsch, Bjoern Lichtblau
                                                                                                                                     
For documentation and questions please use the web site
http://www.scatterweb.net and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2003/2004
*/

#include "contiki.h"
/* #include <msp430x14x.h> */

#include "dev/eeprom.h"

/* Temporary switch for the eeprom address. */
#define EEPROMADDRESS (0x00) /* use 8k EEPROM, future versions will have only one 64k eeprom at this address */
/*#define EEPROMADDRESS (0x02)*/   /* use 64k EEPROM */
 
/* must be set together with EEPROMADDRESS */
/*#define EEPROMPAGEMASK (0x1F) */ /*  8k EEPROM, 32b page writes possible */
#define EEPROMPAGEMASK (0x7F)   /* 64k EEPROM, 128b page writes possible */
 


/*
 * Macros for accessing the clock and data lines. Data is on P56 and
 * clock on P57.
 */
#define SDA_HIGH (P5OUT |= 0x04) /**< EEPROM data line high */
#define SDA_LOW (P5OUT &= 0xFB)  /**< EEPROM data line low */
#define SCL_HIGH (P5OUT |= 0x08) /**< EEPROM clock line high */
#define SCL_LOW (P5OUT &= 0xF7)  /**< EEPROM clock line low */
 

/*-----------------------------------------------------------------------------------*/
/**
 * \internal \name Basic functions for interfacing the i2c-like EEPROM bus.
 */
/** @{ */
 
/**
 * \internal
 * Put start condition on the bus.
 */
static void
start(void)
{
  P5DIR |= 0x0C; /* ensure: P52(SDA), P53(SCL) output */
  SCL_LOW;  _NOP(); _NOP();
  SDA_HIGH; _NOP(); _NOP();
  SCL_HIGH; _NOP(); _NOP();
  SDA_LOW;  _NOP(); _NOP();
  SCL_LOW;  _NOP(); _NOP();
}

/*-----------------------------------------------------------------------------------*/
/**
 * \internal
 * Put stop condition on the bus.
 */
static void
stop(void)
{
  //P5DIR |= 0x0C; /* ensure: P52(SDA), P53(SCL) output */
  SCL_LOW;  _NOP(); _NOP();
  SDA_LOW;  _NOP(); _NOP();
  SCL_HIGH; _NOP(); _NOP();
  SDA_HIGH; _NOP(); _NOP();
  SCL_LOW;  _NOP(); _NOP();
  P5DIR &= ~0x0C;
}
/*-----------------------------------------------------------------------------------*/
/**
 * \internal
 * Write a byte on the bus, return the acknowledge bit.
 */
static int
write_bus(unsigned char byte)
{
  int i, ack;

  /* Write byte, one bit at a time. Start with the leftmost (most
     significant) bit and roll in bits from the right. */
  for(i = 0; i < 8; ++i) {
    if(byte & 0x80) {
      SDA_HIGH;
    } else {
      SDA_LOW;
    }
    _NOP();
    _NOP();
    SCL_HIGH;
    _NOP();
    _NOP();
    byte = byte << 1;
    SCL_LOW;
    _NOP();
    _NOP();
  }
  
  /* check ack */
  P5DIR &= 0xFB;                              /* P52(SDA) input */
  SCL_HIGH;
  _NOP();
  _NOP();
  if(P5IN & 0x04) {
    ack = 0;
  } else {
    ack = 1;      /* test if ack=0, else error */
  }
  SCL_LOW;
  _NOP();
  _NOP();
  P5DIR |= 0x04;                              /* P52(SDA) output */
  _NOP();
  _NOP();
  _NOP();
  return ack;
}
/*-----------------------------------------------------------------------------------*/                                                                                                                                     
/**
 * \internal
 * Read one byte from the bus.
 *
 * \param ack If set, the ackbit after the received byte will be set.
 */
static unsigned char
read_bus(unsigned char ack)
{
  int i;
  unsigned char byte = 0;
                                                                                                                                     
  P5DIR &= 0xFB;                              /* P52(SDA) input */
                                                                                                                                     
  for(i = 0; i < 8; ++i) {
    byte = byte << 1;
    SCL_HIGH;
    _NOP();
    _NOP();
    if(P5IN & 0x04) {
      byte |= 0x01;
    } else {
      byte &= 0xFE;
    }
    _NOP();
    SCL_LOW;
    _NOP();
    _NOP();
  }
  
  P5DIR |= 0x04;                              /* P52(SDA) output */
  if(ack) {
    SDA_LOW;
  } else {
    SDA_HIGH;
  }
  _NOP();
  SCL_HIGH;
  _NOP();
  SCL_LOW;
  _NOP();
  return byte;
}
/** @} */
/*-----------------------------------------------------------------------------------*/                                                                                                                                     
/**
 * Read bytes from the EEPROM using sequential read.
 */
void
eeprom_read(unsigned short addr, unsigned char *buf, int size)
{
  unsigned int i;

  if(size <= 0) {
    return;
  }
  
  do {
    /* Wait if the writecycle has not finished. */
    start();
    /* 1010 control, 000 address, 0=write  --- but only inits address */
  } while(!write_bus(0xa0 | EEPROMADDRESS));
           

  /* Write address to bus, low byte first. */
  write_bus(addr >> 8);
  write_bus(addr & 0xff);
  start();
  /* 1010 control, 000 address, 1=read */
  write_bus(0xa1 | EEPROMADDRESS); 

  for(i = 0; i < (size - 1); ++i){
    buf[i] = read_bus(1);
  }
  buf[size - 1] = read_bus(0);
  stop();
}
/*-----------------------------------------------------------------------------------*/                                                                                                                                     
/**
 * Write bytes to EEPROM using sequencial write.
 */
void
eeprom_write(unsigned short addr, unsigned char *buf,
	     int size)
{
  unsigned int i = 0;
  unsigned int curaddr;

  if(size <= 0) {
    return;
  }
  
  /* Disable write protection. */
  P5OUT &= 0xEF; 

  curaddr = addr;
  for(i = 0; i < size; ++i) {
    /* If we are writing the first byte or are on a 128b page boundary
       we have to start a new write. */
    if(i == 0 || (curaddr & EEPROMPAGEMASK) == 0) {
      do {
	start();
	/* 1010 control, 000 addresse, 0=write */
      } while(!write_bus(0xa0 | EEPROMADDRESS));

      /* Write the new address to the bus. */
      if(write_bus(curaddr >> 8) == 0) {
	return;
      }
      if(write_bus(curaddr) == 0) {
	return;
      }
    }
  
    /* Write byte. */
    if(write_bus(buf[i]) == 0) {
      return;
    }
    
    /* If we are writing the last byte totally or of a 128b page
       generate a stop condition */
    if(i == size - 1 || (curaddr & EEPROMPAGEMASK) == EEPROMPAGEMASK) {
      stop();
    }
    
    ++curaddr;
  }
  
  /* Enable write protection. */
  P5OUT |= 0x10;                              
}
                                                                                                                                     
/*-----------------------------------------------------------------------------------*/ 
