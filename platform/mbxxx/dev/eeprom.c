/*
 * Copyright (c) 2013, IDentification Automation Laboratory
 * IDALab (http://www.idalab.unisalento.it)
 * Department of Innovation Engineering - University of Salento
 *
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
 * \file  platform/mbxxx/dev/eeprom.c
 * \brief ST M24C64W EEPROM driver.
 * \author Maria Laura Stefanizzi <laura28582@gmail.com>
 * \date   2013-11-20
 */

#include PLATFORM_HEADER
#include <dev/eeprom.h>
#include <dev/i2c.h>


#define EE_HW_ADDRESS            0xA0
#define EE_PAGESIZE              32
#define EE_PAGEMASK              0x1F
#define EE_MAX_TRIALS            300

/* Write Cycle polling
 *
 * During the internal Write cycle, the device disconnects itself from the bus,
 * and writes a copy of the data from its internal latches to the memory cells.
 */
static inline void
eeprom_wait(void)
{
  uint32_t trials = 0;

  /* Keep looping till the slave acknowledge his address or maximum number of
     trials is reached */
  do {
    /* Generate start */
    i2c_start();

    /* Device select in in write mode */
    i2c_write(EE_HW_ADDRESS | 0x0);

    /* Check if the maximum allowed number of trials has been reached */
    if(trials++ == EE_MAX_TRIALS) {
      /* If the maximum number of trials has been reached, exit the function */
      break;
    }
  } while((SC2_STAT(SC_TWIRXNAK)) == SC_TWIRXNAK);
  /* eeprom reply with an ACK then it has terminated the internal Write cycle */

  i2c_stop();
}

/**
 * eeprom initializzation function
 */
void
eeprom_init(void)
{
  /* Nothing must be done here */
}

/**
 * Write data to eeprom
 * @param addr The eeprom memory address
 * @param buf It is the buffer of bytes that will be written to the eeprom
 * @param size It is the number of byte to write
 */
void
eeprom_write(eeprom_addr_t addr, unsigned char *buf, int size)
{
  unsigned int i = 0;
  unsigned int curaddr;

  curaddr = addr;
  for(i = 0; i < size; i++) {
    /* If we are writing the first byte or are on a EE_PAGEMASK page boundary
       we have to start a new write. */
    if(i == 0 || (curaddr & EE_PAGEMASK) == 0) {

      i2c_start();
      i2c_write(EE_HW_ADDRESS | 0x0);

      /* Write the new address to the bus. */
      i2c_write((curaddr & 0xFF00) >> 8);
      i2c_write(curaddr & 0xFF);
    }

    /* Write byte. */
    i2c_write(buf[i] & 0xFF);

    /* If we are writing the last byte totally or of a 32b page
       generate a stop condition */
    if(i == size - 1 || (curaddr & EE_PAGEMASK) == EE_PAGEMASK) {
      i2c_stop();
      eeprom_wait();
    }

    curaddr++;
  }
}

/**
 * Read data from eeprom
 * @param addr The eeprom memory address
 * @param buf It is the destination buffer in witch the bytes will be written
 * @param size It is the number of byte to read
 */
void
eeprom_read(eeprom_addr_t addr, unsigned char *buf, int size)
{
  uint8_t i;

  i2c_start();

  /* Select eeprom with write mode bit enabled */
  i2c_write(EE_HW_ADDRESS | 0x0);

  /* Send address */
  i2c_write((addr & 0xFF00) >> 8);
  i2c_write(addr & 0xFF);

  i2c_start();

  /* Select eeprom with write mode bit disabled */
  i2c_write(EE_HW_ADDRESS | 0x1);

  for(i = 0; i < size; i++) {
    if(i < (size - 1)) {
      /* Read data and send ACK */
      *(buf + i) = i2c_read(1);
    } else {
      /* Last data, don't send ACK */
      *(buf + i) = i2c_read(0);
    }
  }

  i2c_stop();
}

/** @} */
