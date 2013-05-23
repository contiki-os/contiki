/***************************************************************************//**
 *   @file   ST7579.h
 *   @brief  Header file of ST7579 Driver.
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
#ifndef __ST7579_H__
#define __ST7579_H__

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Transmits 8 bits to ST7579 controller. */
void ST7579_WriteByte(unsigned char data);

/*! Sets X and Y adress of RAM. */
void ST7579_GoTo(unsigned char yPosition,
                 unsigned char xPosition);

/*! Clears ST7579 RAM. */
void ST7579_Clear(void);

/*! Initializes the ST7579 controller. */
char ST7579_Init(void);

/*! Sends a character to ST7579 controller. */
void ST7579_Char(unsigned char yPosition,
                 unsigned char xPosition,
                 unsigned char character,
                 unsigned char bigFont);

/*! Sends a string to ST7579 controller. */
void ST7579_String(unsigned char yPosition,
                   unsigned char xPosition,
                   unsigned char* string,
                   unsigned char bigFont);

/*! Sends an integer number to ST7579 controller. */
void ST7579_Number(unsigned char yPosition,
                   unsigned char xPosition,
                   long number,
                   unsigned char bigFont);

/*! Sends a float number to ST7579 controller. */
void ST7579_FloatNumber(unsigned char yPosition,
                        unsigned char xPosition,
                        float number,
                        unsigned char resolution,
                        unsigned char bigFont);

/*! Sends an integer number in Hexa format to ST7579 controller. */
void ST7579_HexNumber(unsigned char yPosition,
                      unsigned char xPosition,
                      long number,
                      unsigned char bigFont);

/*! Sends an image array to ST7579 controller. */
void ST7579_Image(unsigned char yPosition,
                  unsigned char xPosition,
                  const unsigned char image[],
                  unsigned char width,
                  unsigned char height);

#endif // __ST7579_H__
