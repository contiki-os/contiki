/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Platform specific functions for LIS3DH accelerometer on the Mulle platform.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "lis3dh.h"
#include "MK60N512VMD100.h"
#include "interrupt.h"
#include "power-control.h"
#include <stdint.h>
#include <stdbool.h>

#define LIS3DH_CHIP_SELECT_PIN 0
#define LIS3DH_CTAR_NUMBER 1

/* Glueing the CTAR number together with a prefix to form register names */
#define LIS3DH_PREPROCESSOR_PASTE(a, b) a##b
#define LIS3DH_CTAR_APPEND_NUMBER(b) LIS3DH_PREPROCESSOR_PASTE(SPI0_CTAR, b)
#define LIS3DH_CTAR_REG LIS3DH_CTAR_APPEND_NUMBER(LIS3DH_CTAR_NUMBER)

/**
 * Perform a one byte transfer over SPI.
 *
 * \param data The data to write to the slave.
 * \param cont If set, keep asserting the chip select pin after the current byte transfer ends.
 * \param blocking If set, wait until all bits have been transferred before returning.
 * \return The byte received from the slave during the same transfer.
 *
 * \note There is no need for separate read and write functions, since SPI transfers work like a shift register (one bit out, one bit in.)
 *
 * \todo Make SPI abstraction standalone.
 */
uint8_t
spi_transfer(const uint8_t data, const bool cont, const bool blocking)
{
  uint32_t spi_pushr;

  spi_pushr = SPI_PUSHR_TXDATA(data);
  spi_pushr |= SPI_PUSHR_CTAS(LIS3DH_CTAR_NUMBER);
  spi_pushr |= SPI_PUSHR_PCS((1 << LIS3DH_CHIP_SELECT_PIN));
  if(cont) {
    spi_pushr |= SPI_PUSHR_CONT_MASK;
  }

  /* Clear transfer complete flag */
  SPI0_SR |= SPI_SR_TCF_MASK;

  /* Shift a frame out/in */
  SPI0_PUSHR = spi_pushr;

  if(blocking) {
    /* Wait for transfer complete */
    while(!(SPI0_SR & SPI_SR_TCF_MASK));
  }

  /* Pop the buffer */
  return (0xFF & SPI0_POPR);
}

/**
 * Write a single byte to the LIS3DH.
 *
 * \param addr The target register.
 * \param value The value to write.
 */
void
lis3dh_write_byte(const lis3dh_reg_addr_t addr, const uint8_t value)
{
  MK60_ENTER_CRITICAL_REGION();
  spi_transfer((addr & LIS3DH_SPI_ADDRESS_MASK) | LIS3DH_SPI_WRITE_MASK | LIS3DH_SPI_SINGLE_MASK, true, true);  /* Write address */
  spi_transfer(value, false, true);     /* Write data */
  MK60_LEAVE_CRITICAL_REGION();
}

/**
 * Read a single byte from the LIS3DH.
 *
 * \param addr The source register.
 * \return The value of the register.
 */
uint8_t
lis3dh_read_byte(const lis3dh_reg_addr_t addr)
{
  uint8_t data;

  MK60_ENTER_CRITICAL_REGION();
  spi_transfer((addr & LIS3DH_SPI_ADDRESS_MASK) | LIS3DH_SPI_READ_MASK | LIS3DH_SPI_SINGLE_MASK, true, true);   /* Write address */
  data = spi_transfer(0, false, true);  /* Dummy write to get data */
  MK60_LEAVE_CRITICAL_REGION();
  return data;
}

/**
 * Read a 16-bit integer from the LIS3DH.
 *
 * \param lsb_addr The lower address of the two source registers.
 * \return The value of the register, byte order depends on the big/little endian setting of the LIS3DH.
 * \note The BLE bit of CTRL_REG4 will affect the byte order of the return value.
 */
int16_t
lis3dh_read_int16(const lis3dh_reg_addr_t lsb_addr)
{
  int16_t data;

  MK60_ENTER_CRITICAL_REGION();

  /*
   * We will do a multi byte read from the LIS3DH.
   * After the first read the address will be automatically increased by one.
   */
  /* Write address */
  spi_transfer((lsb_addr & LIS3DH_SPI_ADDRESS_MASK) |
               LIS3DH_SPI_READ_MASK | LIS3DH_SPI_MULTI_MASK, true, true);

  /*
   * Do not modify the following two statements into a single statement, the
   * spi_transfer calls must be made in the right order.
   * Google "sequence points".
   */
  data = spi_transfer(0xFF, true, true);        /* Read LSB, keep holding chip select active. */
  data |= (spi_transfer(0xFF, false, true) << 8);       /* Read MSB, release chip select. */

  MK60_LEAVE_CRITICAL_REGION();
  return (int16_t) data;
}

/**
 * Read multiple bytes from the LIS3DH.
 *
 * \param start_address The lower address of the source registers.
 * \param buffer A buffer to write the read values into.
 * \param count Number of bytes to read.
 */
void
lis3dh_memcpy_from_device(const lis3dh_reg_addr_t start_address,
                          uint8_t * buffer, uint8_t count)
{
  /*
   * This function must not be interrupted by anything using the SPI bus or
   * it will mess up the transfer.
   */
  MK60_ENTER_CRITICAL_REGION();

  /*
   * We will do a multi byte read from the LIS3DH.
   * After the first read the address will be automatically increased by one
   * for each subsequent transfer.
   */
  /* Write address */
  spi_transfer((start_address & LIS3DH_SPI_ADDRESS_MASK) |
               LIS3DH_SPI_READ_MASK | LIS3DH_SPI_MULTI_MASK, true, true);

  while(count > 1) {
    /* Read byte, keep holding chip select active. */
    *buffer = spi_transfer(0xFF, true, true);
    ++buffer;
    --count;
  }
  /* Read last byte, release chip select. */
  *buffer = spi_transfer(0xFF, false, true);

  MK60_LEAVE_CRITICAL_REGION();
}

/**
 * Write multiple bytes to the LIS3DH.
 *
 * \param start_address The lower address of the target registers.
 * \param buffer A buffer to read the values from.
 * \param count Number of bytes to write.
 */
void
lis3dh_memcpy_to_device(const lis3dh_reg_addr_t start_address,
                        const uint8_t * buffer, uint8_t count)
{
  /*
   * This function must not be interrupted by anything using the SPI bus or
   * it will mess up the transfer.
   */
  MK60_ENTER_CRITICAL_REGION();

  /*
   * We will do a multi byte write to the LIS3DH.
   * After the first write the address will be automatically increased by one
   * for each subsequent transfer.
   */
  /* Write address */
  spi_transfer((start_address & LIS3DH_SPI_ADDRESS_MASK) |
               LIS3DH_SPI_WRITE_MASK | LIS3DH_SPI_MULTI_MASK, true, true);

  while(count > 1) {
    /* Write byte, keep holding chip select active. */
    spi_transfer(*buffer, true, true);
    ++buffer;
    --count;
  }
  /* Write last byte, release chip select. */
  spi_transfer(*buffer, false, true);

  MK60_LEAVE_CRITICAL_REGION();
}

/**
 * Perform the platform specific part of the initialization process of the LIS3DH.
 * This function is expected to set up the SPI module for the LIS3DH.
 */
void
lis3dh_arch_init()
{
  /* Enable clock gate on PTD (for SPI0) */
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  /* Note: Interrupts will need to enable clock gate on PTC as well */

  /* Enable clock gate for SPI0 module */
  SIM_SCGC6 |= SIM_SCGC6_DSPI0_MASK;

  /* Configure SPI0 */
  /* Master mode */
  /* all peripheral chip select signals are active low */
  /* Disable TX,RX FIFO */
  SPI0_MCR = SPI_MCR_MSTR_MASK | SPI_MCR_PCSIS(0x1F) | SPI_MCR_DIS_RXF_MASK | SPI_MCR_DIS_TXF_MASK;     /* 0x803F3000; */

  /* 8 bit frame size */
  /* Set up different delays and clock scalers */
  /* TODO: These need tuning */
  /* FIXME: Coordinate SPI0 parameters between different peripheral drivers */
  /* IMPORTANT: Clock polarity is active low! */
  LIS3DH_CTAR_REG = SPI_CTAR_FMSZ(7) | SPI_CTAR_CSSCK(2) | SPI_CTAR_ASC(2) | SPI_CTAR_DT(2) | SPI_CTAR_BR(4) | SPI_CTAR_CPOL_MASK | SPI_CTAR_CPHA_MASK; /*0x38002224; *//* TODO: Should be able to speed up */

  /* Mux SPI0 on port D */
  PORTD_PCR0 = PORT_PCR_MUX(2); /* SPI0_PCS0 */
  PORTD_PCR1 = PORT_PCR_MUX(2); /* SPI0_SCK */
  PORTD_PCR2 = PORT_PCR_MUX(2); /* SPI0_SOUT */
  PORTD_PCR3 = PORT_PCR_MUX(2); /* SPI0_SIN */

  return;
}
