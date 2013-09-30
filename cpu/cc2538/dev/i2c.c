/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */
 /**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-12c cc2538 i2c
 *
 * Driver for the cc2538 i2c controller
 * @{
 *
 * \file
 * C file for the cc2538 i2c driver
 *
 * \author
 *          Adam Rea <areairs@gmail.com>
*/

#include "contiki.h"
#include "dev/i2c.h"
#include "sys/energest.h"
#include "dev/sys-ctrl.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "reg.h"
#include <stdio.h>


/* These values were lifted from cc2538 User Guied p. 438 based on 16MHz clock.
/  The numbers after in comments are the 32MHz clock values.  The XTAL on the
/  board seems to be a 32MHz, but other code I have seen here seem to use a
/  16MHz basis
*/
#define I2C_SPEED_400K  0x01   /* 0x03 */
#define I2C_SPEED_100K  0x07   /* 0x13 */


/* Due to the nature of the I/O crossbar, define these based on your own
/  hookup locations
/  In my wiring to the dk board:
/     SCL is hooked to RF1.13  -> PB3
/     SDA is hooked to RF1.17  -> PB5
*/
#define I2C_SCL_PORT_BASE   GPIO_B_BASE
#define I2C_SCL_PORT        GPIO_B_NUM
#define I2C_SCL_PIN         3
#define I2C_SDA_PORT_BASE   GPIO_B_BASE
#define I2C_SDA_PORT        GPIO_B_NUM
#define I2C_SDA_PIN         5

/*---------------------------------------------------------------------------*/
/* The init command for the i2c master driver.  Does what you would expect
/ from an init.  Sets pin functions and dirs and preps the i2c subsystem.
*/
void
i2c_init(void)
{
  
  /* Turn on the i2c clock in all states */
  REG(SYS_CTRL_RCGCI2C) |= 0x0001;
  REG(SYS_CTRL_SCGCI2C) |= 0x0001;
  REG(SYS_CTRL_DCGCI2C) |= 0x0001;
  
  /* Setup the SDA and SCL to the right port functions and map
  /  the port:pin to the I2CMSSxx registers to grab those pins
  /  Set the pins to periphreal mode (SCL and SDA)
  */
  REG(I2C_SCL_PORT_BASE + GPIO_AFSEL) |= (0x0001 << I2C_SCL_PIN);
  ioc_set_sel(I2C_SCL_PORT, I2C_SCL_PIN, IOC_PXX_SEL_I2C_CMSSCL);
  ioc_set_over(I2C_SCL_PORT, I2C_SCL_PIN,IOC_OVERRIDE_DIS);
  REG(IOC_I2CMSSCL) |= ( (I2C_SCL_PORT << 3) + I2C_SCL_PIN);

  REG(I2C_SDA_PORT_BASE + GPIO_AFSEL) |= (0x0001 << I2C_SDA_PIN);
  ioc_set_sel(I2C_SDA_PORT, I2C_SDA_PIN, IOC_PXX_SEL_I2C_CMSSDA);
  ioc_set_over(I2C_SDA_PORT, I2C_SDA_PIN,IOC_OVERRIDE_DIS);
  REG(IOC_I2CMSSDA) |= ( (I2C_SDA_PORT << 3) + I2C_SDA_PIN);


  /*
  / Enable the master block.
  */
  REG(I2CM_CR) |= I2CM_CR_MFE;
  
  /*
  /  Get the desired SCL speed. Can be set with _CONF_'s in project-conf.h
  */
  
#if I2C_CONF_HI_SPEED /* 400k */
  REG(I2CM_TPR) = I2C_SPEED_400K;
#else /*  I2C_CONF_HI_SPEED */ /* 100k */
  REG(I2CM_TPR) = I2C_SPEED_100K;
#endif /*  I2C_CONF_HI_SPEED */
    
  /* Should be ready to go as a master */
}

/*---------------------------------------------------------------------------*/
/* This is a write command that supports arbitrary lenght i2c sends
/     args:  uint8_t* b -- pass an array of what needs written to the port
/                             MUST BE NULL TERMINATED!!!
/            uint8_t  slaveaddr  -- the (raw) i2c address for the device (<0x80)
/ 
/     return: 0 error, 1 fell through (should be unreachable), 2 successful
/                 byte write, 3 successful multibyte write
*/

uint8_t 
i2c_write_bytes(uint8_t* b, uint8_t slaveaddr)
{
  
  uint8_t buflen, c;
  /* check that slaveaddr only has 7 bits active */
  if ( slaveaddr >> 7)
    return 0;

  /* Set slave addr (data sheet says 0:6 are address with a s/r bit but
  /  the diagram shows address is 1:7 with the s/r bit in 0...I''m trusting
  /  the picture - makes most sense. Also matches the calcualtion on pp 453.
  /  Send is bit 0 low Rx is bit 0 high
  /  now that we checked the addr put it in to write
  */
  REG(I2CM_SA) = (slaveaddr << 1);
  
  /* Calculate the length of array b for use later */
  buflen = 0;
  while (b[buflen] != 0x00)
    {
      buflen++;
    }
 
  if (buflen == 1) /* a single byte command */
    {
      REG(I2CM_DR) = b[0];
      REG(I2CM_CTRL) = I2C_MASTER_CMD_SINGLE_SEND;
      
      /* wait on busy then check error flag */
      while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
      if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
	return 0;
      
      /* This was NOT according to data sheet.  Just waiting on the busy flag
      /  was resulting in weird offsets and random behavior.  Even if you are
      /  not using interrupts directly, if you spin on the RIS, it will synch
      /  your results.
      */  
      while(!REG(I2CM_RIS));
      REG(I2CM_ICR) |= 0x01;
      return 2;  /* return successful single byte write */
    }
  else /* multi-byte command */
    {
      for(c = 0; c < buflen; c++) {
	REG(I2CM_DR) = b[c];
	if(c == 0) { 
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_SEND_START;
	} else if (c == buflen -1) {  
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_SEND_FINISH;
	} else {   
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_SEND_CONT;
	} 

	/* wait on busy then check error flag */
      while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
      if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
	return 0;
      
      /* This was NOT according to data sheet.  Just waiting on the busy flag
      /  was resulting in weird offsets and random behavior.  Even if you are
      /  not using interrupts directly, if you spin on the RIS, it will synch
      /  your results.
      */  
      while(!REG(I2CM_RIS));
      REG(I2CM_ICR) |= 0x01;
      }
      return 3; //return successful multi byte write
    }
  return 1;  
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* This is a write command that supports one byte i2c sends (written for
/ debugging, but probably still a useful command to leave in.
/     args:  uint8_t value -- the byte to be written
/            uint8_t  slaveaddr  -- the (raw) i2c address for the device (<0x80)
/ 
/     return: 0 error,  2 successful single byte write (for consistency above)
*/
uint8_t 
i2c_write_byte(uint8_t value, uint8_t slaveaddr)
{
 
  /* check that slaveaddr only has 7 bits active */
  if ( slaveaddr >> 7)
    return 0;

  /* Set slave addr (data sheet says 0:6 are address with a s/r bit but
  /  the diagram shows address is 1:7 with the s/r bit in 0...I''m trusting
  /  the picture - makes most sense. Also matches the calcualtion on pp 453.
  /  Send is bit 0 low Rx is bit 0 high
  /  now that we checked the addr put it in to write
  */
  REG(I2CM_SA) = (slaveaddr << 1);

  REG(I2CM_DR) = value;
  REG(I2CM_CTRL) = I2C_MASTER_CMD_SINGLE_SEND;

  /* wait on busy then check error flag */
  while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
  if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
    return 0;
      
  /* This was NOT according to data sheet.  Just waiting on the busy flag
  /  was resulting in weird offsets and random behavior.  Even if you are
  /  not using interrupts directly, if you spin on the RIS, it will synch
  /  your results.
  */  
  while(!REG(I2CM_RIS));
  REG(I2CM_ICR) |= 0x01;
  return 2;
}
/*---------------------------------------------------------------------------*/
/*This is a read command that supports arbitrary lenght i2c sends 
/     args:  uint8_t* b -- pass an array to written into
/            uint8_t len --  how many bytes to be written -- NO PROTECTION
/                                FOR ARRAY OVERRUNS -- USER MUST DO SO!!!
/            uint8_t  slaveaddr  -- the (raw) i2c address for the device (<0x80)
/ 
/     return: 0 error, 1 fell through (should be unreachable), 2 successful
/                 byte write, 3 successful multibyte write
*/
uint8_t
i2c_read_bytes(uint8_t* b, uint8_t len, uint8_t slaveaddr)
{
  uint8_t c;
  
  //check that slaveaddr only has 7 bits active
  if ( slaveaddr >> 7)
    return 0;
  
  /* Set slave addr (data sheet says 0:6 are address with a s/r bit but
  /  the diagram shows address is 1:7 with the s/r bit in 0...I''m trusting
  /  the picture - makes most sense. Also matches the calcualtion on pp 453.
  /  Send is bit 0 low.  Rx is bit 0 high
  /  now that we checked the addr put it in to read
  */
  REG(I2CM_SA) = (slaveaddr << 1) + 1;
  
  if (len == 1) /* a single byte read */
    {
      REG(I2CM_CTRL) = I2C_MASTER_CMD_SINGLE_RECEIVE;
     
      /* wait on busy then check error flag */
      while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
      if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
	return 0;
      
      /* This was NOT according to data sheet.  Just waiting on the busy flag
      /  was resulting in weird offsets and random behavior.  Even if you are
      /  not using interrupts directly, if you spin on the RIS, it will synch
      /  your results.
      */  
      while(!REG(I2CM_RIS));
      REG(I2CM_ICR) |= 0x01;
      b[0] = REG(I2CM_DR);
      return 2;  /* successful single byte read */
    }
  else /* multi-byte command */
    {
      for(c = 0; c < len; c++) {
	b[c] = 0;
	if(c == 0) {  
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_RECEIVE_START;
	} else if (c == len -1) { 
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_RECEIVE_FINISH;
	} else {  
	  REG(I2CM_CTRL) = I2C_MASTER_CMD_BURST_RECEIVE_CONT;
	} 

	/* wait on busy then check error flag */
	while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
	if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
	  return 0;
      
	/* This was NOT according to data sheet.  Just waiting on the busy flag
	/  was resulting in weird offsets and random behavior.  Even if you are
	/  not using interrupts directly, if you spin on the RIS, it will synch
	/  your results.
	*/  
	while(!REG(I2CM_RIS));
	REG(I2CM_ICR) |= 0x01;
	b[c] = REG(I2CM_DR);
      }
      return 3; /* successful multi byte write */
    }
  return 1;  
}
/*---------------------------------------------------------------------------*/
/* This is a write command that supports single byte i2c sends.  Was written for
/  debugging but left in as it is probably a useful function generally 
/     args:  uint8_t  slaveaddr  -- the (raw) i2c address for the device (<0x80)
/ 
/     return: 0 error else the return is the byte read from the bus (could 
/                   collide with 0)
*/
uint8_t
i2c_read_byte(uint8_t slaveaddr)
{
  //check that slaveaddr only has 7 bits active
  if ( slaveaddr >> 7)
    return 0;

  /* Set slave addr (data sheet says 0:6 are address with a s/r bit but
  /  the diagram shows address is 1:7 with the s/r bit in 0...I''m trusting
  /  the picture - makes most sense. Also matches the calcualtion on pp 453.
  /  Send is bit 0 low.  Rx is bit 0 high
  /  now that we checked the addr put it in to read
  */
  REG(I2CM_SA) = (slaveaddr << 1) + 1;
  
  REG(I2CM_CTRL) = I2C_MASTER_CMD_SINGLE_RECEIVE;

   /* wait on busy then check error flag */
  while ( REG(I2CM_STAT) & I2CM_STAT_BUSY);
  if ( REG(I2CM_STAT) & I2CM_STAT_ERROR) 
    return 0;
  
  /* This was NOT according to data sheet.  Just waiting on the busy flag
  /  was resulting in weird offsets and random behavior.  Even if you are
  /  not using interrupts directly, if you spin on the RIS, it will synch
  /  your results.
  */  
  while(!REG(I2CM_RIS));
  REG(I2CM_ICR) |= 0x01;
  
  /* return the byte in the DR register */
  return REG(I2CM_DR);
}

/** @} */
