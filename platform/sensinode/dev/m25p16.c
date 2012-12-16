/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *   This file provides functions to control the M25P16 on sensinode N740s.
 *     This is a Numonyx Forte Serial Flash Memory (16Mbit)
 *     The S signal (Chip Select) is controlled via 0x02 on the 74HC595D
 *     The other instructions and timing are performed with bit bang
 *
 *     We can enable, disable, read/write data, erase pages, hold, enter/exit
 *     deep sleep etc.
 *
 *     Clock  (C) => P1_5,
 *     Ser. I (D) => P1_6,
 *     Ser. O (Q) => P1_7,
 *     Hold       => Pull Up,
 *     Write Prot => Pull Up,
 *     Chip Sel   => 74HC595D (0x02)
 *
 *   This file can be placed in any bank.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "dev/n740.h"
#include "dev/m25p16.h"
#include "sys/clock.h"
#include "sys/energest.h"
#include "cc2430_sfr.h"

#define CLOCK_RISING()  {M25P16_PIN_CLOCK = 1; M25P16_PIN_CLOCK = 0;}
#define CLOCK_FALLING() {M25P16_PIN_CLOCK = 0; M25P16_PIN_CLOCK = 1;}
/*---------------------------------------------------------------------------*/
/* Bit-Bang write a byte to the chip */
static void
bit_bang_write(uint8_t byte) CC_NON_BANKED
{
  uint8_t i;
  uint8_t bit;

  /* bit-by-bit */
  for(i = 0x80; i > 0; i >>= 1) {
    /* Is the bit set? */
    bit = 0;
    if(i & byte) {
      /* If it was set, we want to send 1 */
      bit = 1;
    }
    /* Send the bit */
    M25P16_PIN_SER_I = bit;
    /* Clock - Rising */
    CLOCK_RISING();
  }
}
/*---------------------------------------------------------------------------*/
/* Bit-Bang read a byte from the chip */
static uint8_t
bit_bang_read() CC_NON_BANKED
{
  int8_t i;
  uint8_t bits = 0;

  /* bit-by-bit */
  for(i = 7; i >= 0; i--) {
    /* Clock - Falling */
    CLOCK_FALLING();

    /* Read the bit */
    bits |= (M25P16_PIN_SER_O << i);
  }
  return bits;
}
/*---------------------------------------------------------------------------*/
static void
select() CC_NON_BANKED
{
  /* Read current ser/par value */
  uint8_t ser_par = n740_ser_par_get();

  M25P16_PIN_CLOCK = 0;

  ser_par &= ~N740_SER_PAR_CHIP_SEL;  /* Select Flash */

  /* Write the new status back to the ser/par */
  n740_ser_par_set(ser_par);
}
/*---------------------------------------------------------------------------*/
static void
deselect() CC_NON_BANKED
{
  /* Read current ser/par value */
  uint8_t ser_par = n740_ser_par_get();

  ser_par |= N740_SER_PAR_CHIP_SEL;    /* De-Select Flash  */

  /* Write the new status back to the ser/par */
  n740_ser_par_set(ser_par);
}
/*---------------------------------------------------------------------------*/
void
m25p16_wren()
{
  select();
  bit_bang_write(M25P16_I_WREN);
  deselect();

  while(!M25P16_WEL());
}
/*---------------------------------------------------------------------------*/
void
m25p16_wrdi()
{
  select();
  bit_bang_write(M25P16_I_WRDI);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_rdid(struct m25p16_rdid *rdid)
{
  uint8_t i;

  select();
  bit_bang_write(M25P16_I_RDID);

  rdid->man_id = bit_bang_read();
  rdid->mem_type = bit_bang_read(); /* Device ID MSB */
  rdid->mem_size = bit_bang_read(); /* Device ID LSB */
  rdid->uid_len = bit_bang_read();
  for(i = 0; i < rdid->uid_len; i++) {
    rdid->uid[i] = bit_bang_read();
  }
  deselect();
}
/*---------------------------------------------------------------------------*/
uint8_t
m25p16_rdsr()
{
  uint8_t rv;

  select();
  bit_bang_write(M25P16_I_RDSR);
  rv = bit_bang_read();
  deselect();

  return rv;
}
/*---------------------------------------------------------------------------*/
void
m25p16_wrsr(uint8_t val)
{
  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_WRSR);
  bit_bang_write(val);
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_read(uint8_t * addr, uint8_t * buff, uint8_t buff_len)
{
  uint8_t i;

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_READ);

#if M25P16_READ_FAST
  bit_bang_write(M25P16_I_FAST_READ);
#else
  bit_bang_write(M25P16_I_READ);
#endif

  /* Write the address, MSB in addr[0], bits [7:5] of the MSB: 'don't care' */
  for(i = 0; i < 3; i++) {
    bit_bang_write(addr[i]);
  }

  /* For FAST_READ, send the dummy byte */
#if M25P16_READ_FAST
  bit_bang_write(M25P16_DUMMY_BYTE);
#endif

  for(i = 0; i < buff_len; i++) {
    buff[i] = ~bit_bang_read();
  }
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_READ);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_pp(uint8_t * addr, uint8_t * buff, uint8_t buff_len)
{
  uint8_t i;

  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_PP);

  /* Write the address, MSB in addr[0] */
  for(i = 0; i < 3; i++) {
    bit_bang_write(addr[i]);
  }

  /* Write the bytes */
  for(i = 0; i < buff_len; i++) {
    bit_bang_write(~buff[i]);
  }
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_se(uint8_t s)
{
  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_SE);
  bit_bang_write(s);
  bit_bang_write(0x00);
  bit_bang_write(0x00);
  deselect();
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
}
/*---------------------------------------------------------------------------*/
void
m25p16_be()
{
  m25p16_wren(); /* Write Enable */

  select();
  bit_bang_write(M25P16_I_BE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_dp()
{
  select();
  bit_bang_write(M25P16_I_DP);
  deselect();
}
/*---------------------------------------------------------------------------*/
/*
 * Release Deep Power Down. We do NOT read the Electronic Signature
 */
void
m25p16_res()
{
  select();
  bit_bang_write(M25P16_I_RES);
  deselect();
  /* a few usec between RES and standby */
  while(M25P16_WIP());
}
/*---------------------------------------------------------------------------*/
/**
 * Release Deep Power Down. Read and return the Electronic Signature
 * must return 0x14
 *
 * \return The old style Electronic Signature. This must be 0x14
 */
uint8_t
m25p16_res_res()
{
  uint8_t rv;

  select();
  bit_bang_write(M25P16_I_RES);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);

  rv = bit_bang_read();

  deselect();

  /* a few usec between RES and standby */
  while(M25P16_WIP());
  return rv;
}
/*---------------------------------------------------------------------------*/
