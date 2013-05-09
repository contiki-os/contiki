/***************************************************************************//**
 *   @file   Communication.h
 *   @brief  Header file of the Communication Driver for RL78G14 processor.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: $WCREV$
*******************************************************************************/
#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "RDKRL78G14.h"

/******************************************************************************/
/*************************** Macros Definitions *******************************/
/******************************************************************************/
#define CS_PIN_LOW  PMOD1_CS_LOW
#define CS_PIN_HIGH PMOD1_CS_HIGH
#define MISO_PIN    PMOD1_MISO

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Initializes the SPI communication peripheral. */
char SPI_Init(char lsbFirst,
              long clockFreq,
              char clockPol,
              char clockEdg);

/*! Writes data to SPI. */
char SPI_Write(char slaveDeviceId,
               unsigned char* data,
               char bytesNumber);

/*! Reads data from SPI. */
char SPI_Read(char slaveDeviceId,
              unsigned char* data,
              char bytesNumber);

/*! Initializes the I2C communication peripheral. */
char I2C_Init(long clockFreq);

/*! Writes data to a slave device. */
char I2C_Write(char slaveAddress,
               unsigned char* dataBuffer,
               char bytesNumber,
               char stopBit);

/*! Reads data from a slave device. */
char I2C_Read(char slaveAddress,
              unsigned char* dataBuffer,
              char bytesNumber,
              char stopBit);

#endif // __COMMUNICATION_H__
