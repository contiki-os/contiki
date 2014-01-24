/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Dragos Bogdan <Dragos.Bogdan@Analog.com>
 */

#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

/******************************************************************************/
/*************************** Macros Definitions *******************************/
/******************************************************************************/
#define SPI_MISO        PMOD1_MISO

#define GPIO1_PIN_OUT   PMOD1_GPIO1_PIN_OUT
#define GPIO1_LOW       PMOD1_GPIO1_LOW
#define GPIO1_HIGH      PMOD1_GPIO1_HIGH

#define GPIO2_PIN_OUT   PMOD1_GPIO2_PIN_OUT
#define GPIO2_LOW       PMOD1_GPIO2_LOW
#define GPIO2_HIGH      PMOD1_GPIO2_HIGH

#define GPIO3_PIN_OUT   PMOD1_GPIO3_PIN_OUT
#define GPIO3_LOW       PMOD1_GPIO3_LOW
#define GPIO3_HIGH      PMOD1_GPIO3_HIGH

#define GPIO4_PIN_OUT   PMOD1_GPIO4_PIN_OUT
#define GPIO4_LOW       PMOD1_GPIO4_LOW
#define GPIO4_HIGH      PMOD1_GPIO4_HIGH

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

enum CSI_Bus {
  CSI00,
  CSI01,
  CSI10,
  CSI11,
  CSI20,
  CSI21,
  CSI30,
  CSI31,
};

/*! Initializes the SPI communication peripheral. */
char SPI_Init(enum CSI_Bus bus,
              char lsbFirst,
              long clockFreq,
              char clockPol,
              char clockEdg);

/*! Writes data to SPI. */
char SPI_Write(enum CSI_Bus bus,
               char slaveDeviceId,
               unsigned char *data,
               char bytesNumber);

/*! Reads data from SPI. */
char SPI_Read(enum CSI_Bus bus,
              char slaveDeviceId,
              unsigned char *data,
              char bytesNumber);

/*! Initializes the I2C communication peripheral. */
char I2C_Init(long clockFreq);

/*! Writes data to a slave device. */
char I2C_Write(char slaveAddress,
               unsigned char *dataBuffer,
               char bytesNumber,
               char stopBit);

/*! Reads data from a slave device. */
char I2C_Read(char slaveAddress,
              unsigned char *dataBuffer,
              char bytesNumber,
              char stopBit);

#endif /* __COMMUNICATION_H__ */
