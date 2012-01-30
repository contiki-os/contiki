/*
Copyright 2005, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
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

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2005
*/

/**
 * Part of the source code from ScatterWeb 2.2 (ScatterWeb.{Data,System}.c)
 * released by Freie Universitaet Berlin has been reworked and
 * reformatted to fit the Contiki ESB port.
 */

#include "contiki.h"
#include "dev/ds1629.h"

#define SDA_HIGH  (P5OUT |= 0x01) /* RTC data line high */
#define SDA_LOW   (P5OUT &= 0xFE) /* RTC data line low */
#define SCL_HIGH  (P5OUT |= 0x02) /* RTC clock line high */
#define SCL_LOW   (P5OUT &= 0xFD) /* RTC clock line low */
#define BUS_READ         0x9F
#define BUS_WRITE        0x9E
#define ACC_CSR          0xAC     /* Access Configuration/Control Register */
#define ACC_CLOCK        0xC0     /* Access Clock Register */
#define ACC_CLOCK_ALARM  0xC7     /* Access Clock Alarm Register */
#define ACC_TH           0xA1     /* Access Thermostat Setpoint High */
#define ACC_TL           0xA2     /* Access Thermostat Setpoint Low */
#define ACC_CSRAM        0x17     /* Access Clock 32 byte SRAM */
#define ACC_RT           0xAA     /* Access Read Temperatur Register */
#define CSR_OS1         (0x80)
#define CSR_OS0         (0x40)
#define CSR_A1          (0x20)
#define CSR_A0          (0x10)
#define CSR_CNV         (0x04)
#define CSR_POL         (0x02)
#define CSR_1SH         (0x01)
#define CSR_DEFAULT     (CSR_OS1 + CSR_OS0 + CSR_A1 + CSR_CNV + CSR_1SH + CSR_POL)

/**
 *  Temperature type (built on a signed int). It's a signed (twos complement)
 *  fixed point value with 8 bits before comma and 7 bits after. So Bit 15 is
 *  sign, Bit14-7 is before comma and Bit 6-0 after comma.
 *
 * @since         2.0
 */
typedef union { unsigned int u; signed int s; } temp_t;

/*--------------------------------------------------------------------------*/
/* Puts the start condition on bus. */
static void
cl_start(void)
{
  P5DIR |= 0x03; /* ensure: P50(SDA), P51(SCL) output */
  SCL_LOW;  _NOP(); _NOP();
  SDA_HIGH; _NOP(); _NOP();
  SCL_HIGH; _NOP(); _NOP();
  SDA_LOW;  _NOP(); _NOP();
  SCL_LOW;  _NOP(); _NOP();
}
/*--------------------------------------------------------------------------*/
/* Puts the stop condition on bus. */
static void
cl_stop()
{
  SCL_LOW;  _NOP(); _NOP();
  SDA_LOW;  _NOP(); _NOP();
  SCL_HIGH; _NOP(); _NOP();
  SDA_HIGH; _NOP(); _NOP();
  SCL_LOW;  _NOP(); _NOP();
  P5DIR &= ~0x03;
}
/*--------------------------------------------------------------------------*/
/* Writes a byte on the bus, returns the acknowledge bit. */
static uint16_t
cl_writeOnBus(uint8_t byte)
{
  uint16_t i, ack;
  for(i=0;i<8;i++) {
    if(byte & 0x80) SDA_HIGH; else SDA_LOW;
    SCL_HIGH;
    byte = byte << 1; _NOP();
    SCL_LOW; _NOP();
  }
  /* check ack */
  P5DIR &= 0xFE;                              /* P50(SDA) input */
  SCL_HIGH;
  if(P5IN & 0x01) ack = 0; else ack = 1;      /* test if ack=0, else error */
  _NOP();
  SCL_LOW;
  P5DIR |= 0x01;                              /* P50(SDA) output */
  return ack;
}
/*--------------------------------------------------------------------------*/
static uint8_t
cl_readFromBus(uint16_t ack)
{
  uint16_t i;
  uint8_t byte = 0;
  P5DIR &= 0xFE;                              /* P50(SDA) input */
  for(i=0;i<8;i++) {
    byte = byte << 1;
    SCL_HIGH;
    if(P5IN & 0x01) byte |= 0x01; else byte &= 0xFE;
    SCL_LOW;
  }
  P5DIR |= 0x01;                              /* P50(SDA) output */
  if(ack) SDA_LOW; else SDA_HIGH;
  SCL_HIGH;
  SCL_LOW;
  return byte;
}
/*--------------------------------------------------------------------------*/
static uint16_t
getReg16bit(uint8_t acc, uint16_t bitmask)
{
  uint16_t config = 0;
  do cl_start();
  while(!cl_writeOnBus(BUS_WRITE));
  cl_writeOnBus(acc);
  cl_start();
  cl_writeOnBus(BUS_READ);
  config = cl_readFromBus(1);
  config = config << 8;
  config += cl_readFromBus(0);
  cl_stop();
  config &= bitmask;
  _NOP();
  _NOP();
  return config;
}
/*--------------------------------------------------------------------------*/
/* Only first 8 bit of Configuration Status Register can be set */
static void
setCSReg(uint8_t setting)
{
  do cl_start();
  while(!cl_writeOnBus(BUS_WRITE));
  cl_writeOnBus(ACC_CSR);
  cl_writeOnBus(setting);
  cl_stop();
  _NOP();
  _NOP();
  _NOP();
  _NOP();
}
/*--------------------------------------------------------------------------*/
static void
System_startConversion(void)
{
  do cl_start();                    /* do start until BUS_WRITE is acked */
  while(!cl_writeOnBus(BUS_WRITE)); /* control byte */
  cl_writeOnBus(0xEE);              /* start conversion */
  cl_stop();
}
/*--------------------------------------------------------------------------*/
/* RTC initialization. Initializes RTC with ::CSR_DEFAULT. */
static void
initClock(void)
{
  uint8_t csr = getReg16bit(ACC_CSR,0xFF00) >> 8;
  if(csr!=CSR_DEFAULT) setCSReg(CSR_DEFAULT); /* if desired config isnt in clock => set it */
  /* IMPORTANT: Ensure quartz is generating 32768 Hz */
  /* (sometimes CH bit gets set when clock is read while reset) */
  do cl_start();                    /* Do start until BUS_WRITE is acked. */
  while(!cl_writeOnBus(BUS_WRITE)); /* Send control byte */
  cl_writeOnBus(ACC_CLOCK);         /* Send command byte ::ACC_CLOCK. */
  cl_writeOnBus(0x00);              /* Send starting address 0x00. */
  cl_writeOnBus(0x00);              /* Set CH to 0, tseconds and seconds will also be reset! */
  cl_stop();                        /* Stop condition. */
}
/*--------------------------------------------------------------------------*/
void
ds1629_init()
{
  initClock();
}
/*--------------------------------------------------------------------------*/
void
ds1629_start()
{
  System_startConversion();
}
/*--------------------------------------------------------------------------*/
signed int
ds1629_temperature()
{
  temp_t temperature;

  ds1629_start();

  temperature.u = getReg16bit(ACC_RT,0xFFFF);
  return temperature.s;
}
