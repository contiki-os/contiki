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

/* Chip Select(SDA) pin of the reader is connected to to P5.1 (JP1B, pin 28 of z1) as Output */
#define SDA_CS 1
/*SDA is enabled when output is set to 0 (Active Low) */
#define SPI_CS_ENABLE()  (P5OUT &= ~BV(SDA_CS))
/* SDA is disabled when output is set to 1 */
#define SPI_CS_DISABLE() (P5OUT |= BV(SDA_CS))
/*Unlike in other adaptations, Reset pin of MFRC522 is not used throughout our code
   *Reset Pin of the reader is connected to a 3V power supply, so that hard reset is totally avoided
 */

/*Pin layout of z1 and MFRC522 Reader is as follows:
 *
 **Signal         MFRC522 Reader Pin      Zolertia z1 pin
 *					  JP1B East Port
 **---------------------------------------------------------
 **RST/Reset	     RST		 Pin#22/Pin Name: D+3V
 **SPI CS/SS/SDA      SDA		 Pin#28/Pin Name: I2C.SDA
 **SPI MOSI	     MOSI		 Pin#36/Pin Name: SPI.SIMO
 **SPI MISO	     MISO		 Pin#38/Pin Name: SPI.SOMI
 **SPI SCK/CLK        SCK		 Pin#34/Pin Name: SPI.CLK
 **GROUND	     GND		 Pin#18/Pin Name: CPDGND
 **3.3V		     3.3V		 Pin#20/Pin Name: CPD+3.3V
 */

/* Maximum length of the array for the Tag UID reading */
#define MAX_LEN 16

/* MFRC522 Command words */
#define PCD_IDLE              0x00               /* No action; Cancel the current command */
#define PCD_RECEIVE           0x08               /* Receive Data */
#define PCD_TRANSCEIVE        0x0C               /* Transmit and receive data */
#define PCD_AUTHENT           0x0E               /* Authentication Key */
#define PCD_RESETPHASE        0x0F               /* Reset */

/* MFRC522 Registers */

#define     VERSIONREG            0x37 << 1
#define     TXCONTROLREG          0x14 << 1
#define     TMODEREG              0x2A << 1
#define     TPRESCALERREG         0x2B << 1
#define     TRELOADREGH           0x2C << 1
#define     TRELOADREGL           0x2D << 1
#define     MODEREG               0x11 << 1
#define     TXAUTOREG             0x15 << 1
#define     COMMANDREG            0x01 << 1
#define     COLLREG               0x0E << 1
#define     WATERLEVELREG         0x0B << 1
#define     BITFRAMINGREG         0x0D << 1
#define     COMMIENREG            0x02 << 1
#define     COMMIRQREG            0x04 << 1
#define     FIFOLEVELREG          0x0A << 1
#define     FIFODATAREG           0x09 << 1
#define     ERRORREG              0x06 << 1
#define     CONTROLREG            0x0C << 1

/* MIFARE One card Command words */
#define PICC_REQALL          0x52        /* find all the cards antenna area */
#define PICC_REQIDL          0x26        /* find the antenna area doesn't enter hibernation */
#define PICC_ANTICOLL        0x93        /* anti-collision */

#define END_BYTE 0x00 << 1 /* Stop command for SPI Write */

/* And MF522 The error code is returned at the communication instance */
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

static struct timer rxtimer;
/*-------------------------------------------------------------------*/

/* function definitions */
void mfrc522_init();
static void rfid_write(unsigned char addr, unsigned char val);
static void rfid_reset();
static void antenna_on();
static void set_bit_mask(unsigned char reg, unsigned char mask);
unsigned char mfrc522_read(unsigned char);
unsigned char mfrc522_request(unsigned char req_mode, unsigned char *tag_type);
static void clear_bit_mask(unsigned char reg, unsigned char mask);
static unsigned char rfid_to_card(unsigned char command, unsigned char *send_data, unsigned char send_len, unsigned char *back_data, unsigned int *back_len);
unsigned char mfrc522_anticoll(unsigned char *ser_num);
/*-------------------------------------------------------------------*/

void
mfrc522_init(void)
{

  rfid_reset();
  rfid_write(TMODEREG, 0x80);
  rfid_write(TPRESCALERREG, 0xA9);
  rfid_write(TRELOADREGH, 0x03);
  rfid_write(TRELOADREGL, 0xE8);
  rfid_write(TXAUTOREG, 0x40);
  rfid_write(MODEREG, 0x3D);
  antenna_on();
}
/*-------------------------------------------------------------------*/
static void
rfid_reset(void)
{
  rfid_write(COMMANDREG, PCD_RESETPHASE);
  timer_set(&rxtimer, CLOCK_SECOND / 20);
  while(!timer_expired(&rxtimer)) ;
  while(mfrc522_read(COMMANDREG) & (1 << 4)) ;
}
/*-------------------------------------------------------------------*/
static void
antenna_on(void)
{
  unsigned char temp;
  temp = mfrc522_read(TXCONTROLREG);
  set_bit_mask(TXCONTROLREG, 0x03);
  if((temp & 0x03) != 0x03) {
    rfid_write(TXCONTROLREG, temp | 0x03);
  }
}
/*-------------------------------------------------------------------*/

static void
rfid_write(unsigned char addr, unsigned char val)
{
  spi_init();
  P5SEL |= BV(SDA_CS);
  P5DIR |= BV(SDA_CS);
  SPI_CS_DISABLE();
  timer_restart(&rxtimer);
  while(!timer_expired(&rxtimer)) ;

  SPI_CS_ENABLE();
  SPI_WRITE_FAST(((addr) & (0x7E)));
  SPI_WRITE_FAST(val);
  SPI_WRITE_FAST(END_BYTE);
  SPI_CS_DISABLE();
}
/*-------------------------------------------------------------------*/

static void
set_bit_mask(unsigned char reg, unsigned char mask)
{
  unsigned char tmp;
  tmp = mfrc522_read(reg);
  rfid_write(reg, tmp | mask);
}
/*-------------------------------------------------------------------*/

unsigned char
mfrc522_read(unsigned char addr)
{
  unsigned char *rx_bits;

  spi_init();
  P5SEL |= BV(SDA_CS);
  P5DIR |= BV(SDA_CS);
  SPI_CS_DISABLE();
  timer_restart(&rxtimer);
  while(!timer_expired(&rxtimer)) ;
  SPI_CS_ENABLE();
  SPI_WRITE_FAST((0x80) | ((addr) & (0x7E)));
  SPI_WRITE_FAST(END_BYTE);
  SPI_FLUSH();
  SPI_READ(rx_bits);
  SPI_CS_DISABLE();

  return (unsigned char)rx_bits;
}
/*-------------------------------------------------------------------*/
unsigned char
mfrc522_request(unsigned char req_mode, unsigned char *tag_type)
{
  unsigned char status;
  unsigned int back_bits;
  rfid_write(BITFRAMINGREG, 0x07);   /* TxLastBists = BitFramingReg[2..0] */
  tag_type[0] = req_mode;
  status = rfid_to_card(PCD_TRANSCEIVE, tag_type, 1, tag_type, &back_bits);
  if((status != MI_OK) || (back_bits != 0x10)) {
    status = MI_ERR;
  }

  return status;
}
/*-------------------------------------------------------------------*/
static unsigned char
rfid_to_card(unsigned char command, unsigned char *send_data, unsigned char send_len, unsigned char *back_data, unsigned int *back_len)
{
  unsigned char status = MI_ERR;
  unsigned char irq_en = 0x00;
  unsigned char wait_irq = 0x00;
  unsigned char last_bits;
  unsigned char n;
  unsigned int i;

  switch(command) {

  case PCD_AUTHENT:       /* Certification cards close by */
  {
    irq_en = 0x12;
    wait_irq = 0x10;
    break;
  }
  case PCD_TRANSCEIVE:    /* Transmit FIFO data */
  {
    irq_en = 0x77;
    wait_irq = 0x30;
    break;
  }
  default:
    break;
  }
  rfid_write(COMMIENREG, irq_en | 0x80); /* Interrupt request */
  clear_bit_mask(COMMIRQREG, 0x80);        /* Clear all interrupt request bit */
  set_bit_mask(FIFOLEVELREG, 0x80);        /* FlushBuffer=1, FIFO Initialization */
  rfid_write(COMMANDREG, PCD_IDLE);
  /* Writing data to the FIFO */
  for(i = 0; i < send_len; i++) {
    rfid_write(FIFODATAREG, send_data[i]);
  }

  /* Execute the command */
  rfid_write(COMMANDREG, command);
  if(command == PCD_TRANSCEIVE) {
    set_bit_mask(BITFRAMINGREG, 0x80);    /* StartSend=1,transmission of data starts */
  }

  /* Waiting to receive data to complete*/
  i = 2000;
  do {
    /* CommIrqReg[7..0]
     * Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
     */
    n = mfrc522_read(COMMIRQREG);
    i--;
  } while((i != 0) && !(n & (0x01)) && !(n & wait_irq));

  if(i != 0) {

    if(!(mfrc522_read(ERRORREG) & 0x13)) { /* Collerr CRCErr ProtecolErr */

      status = MI_OK;
      if(n & irq_en & 0x01) {
        status = MI_NOTAGERR;
      }

      if(command == PCD_TRANSCEIVE) {
        n = mfrc522_read(FIFOLEVELREG);

        last_bits = mfrc522_read(CONTROLREG) & 0x07;
        if(last_bits) {
          *back_len = (n - 1) * 8 + last_bits;
        } else {
          *back_len = n * 8;
        }

        if(n == 0) {
          n = 1;
        }
        if(n > MAX_LEN) {
          n = MAX_LEN;
        }
        /* Reading the received data in FIFO */
        for(i = 0; i < n; i++) {
          back_data[i] = mfrc522_read(FIFODATAREG);
        }
      }
    } else {
      printf("~~~ collerr, crcerr, or protecolerr\r\n");
      status = MI_ERR;
    }
  }
  return status;
}
/*-------------------------------------------------------------------*/

static void
clear_bit_mask(unsigned char reg, unsigned char mask)
{
  unsigned char tmp;
  unsigned char invertmask = ~mask;
  tmp = mfrc522_read(reg);
  rfid_write(reg, tmp & invertmask);    /* clear bit mask */
}
/*-------------------------------------------------------------------*/
unsigned char
mfrc522_anticoll(unsigned char *ser_num)
{
  unsigned char status;
  unsigned char i;
  unsigned char ser_num_check = 0;
  unsigned int un_len;
  rfid_write(BITFRAMINGREG, 0x00);    /* TxLastBists = BitFramingReg[2..0] */
  ser_num[0] = PICC_ANTICOLL;
  ser_num[1] = 0x20;
  status = rfid_to_card(PCD_TRANSCEIVE, ser_num, 2, ser_num, &un_len);

  if(status == MI_OK) {
    /* Check card serial number */
    for(i = 0; i < 4; i++) {
      ser_num_check ^= ser_num[i];
    }

    if(ser_num_check != ser_num[i]) {
      status = MI_ERR;
    }
  }

  return status;
}
/*-------------------------------------------------------------------*/
