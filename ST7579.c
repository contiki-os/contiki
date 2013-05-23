/***************************************************************************//**
 *   @file   ST7579.c
 *   @brief  Implementation of ST7579 Driver.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ST7579.h"             // ST7579 definitions
#include "RDKRL78G14.h"         // YRDKRL78G14 definitions
#include "Communication.h"      // Communication definitions

/******************************************************************************/
/************************* Variables Definitions ******************************/
/******************************************************************************/

const unsigned char st7579Font[95][5] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00}, // sp
    {0x00, 0x00, 0x2f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0xc4, 0xc8, 0x10, 0x26, 0x46}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x00, 0x50, 0x30, 0x00}, // ,
    {0x10, 0x10, 0x10, 0x10, 0x10}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x55, 0x2A, 0x55, 0x2A, 0x55}, // /
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x08, 0x3e, 0x41, 0x41, 0x00}, // {
    {0x00, 0x00, 0x77, 0x00, 0x00}, // |
    {0x00, 0x41, 0x41, 0x3e, 0x08}, // }
    {0x02, 0x01, 0x02, 0x01, 0x00}  // ~
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief Transmits 8 bits to ST7579 controller.
 *
 * @param data - Data to transmit.
 *
 * @return None.
*******************************************************************************/
void ST7579_WriteByte(unsigned char data)
{
    SPI_Write(3,        // The ID of the selected slave device.
              &data,    // Write buffer.
              1);       // Number of bytes to write.
}

/***************************************************************************//**
 * @brief Sets X and Y adress of RAM.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 *
 * @return None.
*******************************************************************************/
void ST7579_GoTo(unsigned char yPosition,
                 unsigned char xPosition)
{
    ST7579_RS_LOW;
    /* Set X address of RAM. */
    ST7579_WriteByte(0x28);
    ST7579_WriteByte(0x80 + xPosition);
    /* Set Y address of RAM. */
    ST7579_WriteByte(0x28);
    ST7579_WriteByte(0x40 + yPosition);
    ST7579_RS_HIGH;
}

/***************************************************************************//**
 * @brief Clears ST7579 RAM.
 *
 * @return None.
*******************************************************************************/
void ST7579_Clear(void)
{
    unsigned short clearPosition = 0;
    unsigned char  yPosition     = 0;

    for(clearPosition = 0; clearPosition < (8 * 96); clearPosition++)
    {
        if(!(clearPosition % 96))
        {
            /* Set the X and Y address of RAM. */
            ST7579_GoTo(yPosition, 0);
            yPosition++;
        }
        ST7579_WriteByte(0x00);
    }
}

/***************************************************************************//**
 * @brief Initializes the ST7579 controller.
 *
 * @return status - Result of the initialization procedure.
 *                  Example: 0 - if initialization was successful;
 *                          -1 - if initialization was unsuccessful.
*******************************************************************************/
char ST7579_Init(void)
{
    char           status = -1;
    unsigned short delay  = 0;

    status = SPI_Init(0,        // Transfer format.
                      1000000,  // SPI clock frequency.
                      0,        // SPI clock polarity.
                      1);       // SPI clock edge.
    if(status != 0)
    {
        return status;
    }
    ST7579_RS_OUT;
    ST7579_RS_LOW;
    /* Select LCD bias ratio of the voltage required for driving the LCD. */
    ST7579_WriteByte(0x21);
    ST7579_WriteByte(0x16);
    /* Select Booster efficiency and Booster stage. */
    ST7579_WriteByte(0x23);
    ST7579_WriteByte(0x99);
    /* Set V0 Range. */
    ST7579_WriteByte(0x20);
    ST7579_WriteByte(0x04);
    /* 50 ms delay is recommended. */
    for(delay = 0; delay < 1000; delay++)
    {
        asm("NOP");
    }
    /* Set the frame frequency */
    ST7579_WriteByte(0x23);
    ST7579_WriteByte(0x0C);
    /* Set V0. */
    ST7579_WriteByte(0x21);
    ST7579_WriteByte(0xE7);
    /* Select the display mode */
    ST7579_WriteByte(0x20);
    ST7579_WriteByte(0x0C);
    ST7579_RS_HIGH;
    /* Clear RAM. */
    ST7579_Clear();

    return status;
}

/***************************************************************************//**
 * @brief Sends a character to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param character - The character.
 * @param bigFont   - Big font selection.
 *
 * @return None.
*******************************************************************************/
void ST7579_Char(unsigned char yPosition,
                 unsigned char xPosition,
                 unsigned char character,
                 unsigned char bigFont)
{
    unsigned char column = 0;
    unsigned char data   = 0;

    if(bigFont)
    {
        /* Set the X and Y address of RAM. */
        ST7579_GoTo(yPosition, xPosition);
        /* Send character data. */
        for(column = 0; column < 5; column++)
        {
            data = st7579Font[character - 32][column];
            data = ((data & 0x01) ? 0x03 : 0) |
                   ((data & 0x02) ? 0x0C : 0) |
                   ((data & 0x04) ? 0x30 : 0) |
                   ((data & 0x08) ? 0xC0 : 0);
            ST7579_WriteByte(data);
            ST7579_WriteByte(data);
            ST7579_WriteByte(data);
        }
        ST7579_WriteByte(0);
        ST7579_WriteByte(0);
        ST7579_WriteByte(0);
        yPosition++;
        /* Set the X and Y address of RAM. */
        ST7579_GoTo(yPosition, xPosition);
        /* Send character data. */
        for(column = 0; column < 5; column++)
        {
            data = st7579Font[character - 32][column];
            data = ((data & 0x10) ? 0x03 : 0) |
                   ((data & 0x20) ? 0x0C : 0) |
                   ((data & 0x40) ? 0x30 : 0) |
                   ((data & 0x80) ? 0xC0 : 0);
            ST7579_WriteByte(data);
            ST7579_WriteByte(data);
            ST7579_WriteByte(data);
        }
        ST7579_WriteByte(0);
        ST7579_WriteByte(0);
        ST7579_WriteByte(0);
    }
    else
    {
        /* Set the X and Y address of RAM. */
        ST7579_GoTo(yPosition, xPosition);
        /* Send character data. */
        for(column = 0; column < 5; column++)
        {
            ST7579_WriteByte(st7579Font[character - 32][column] << 1);
        }
        ST7579_WriteByte(0x00);
    }
}

/***************************************************************************//**
 * @brief Sends a string to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param string    - The string.
 * @param bigFont   - Big font selection.
 *
 * @return None.
*******************************************************************************/
void ST7579_String(unsigned char yPosition,
                   unsigned char xPosition,
                   unsigned char* string,
                   unsigned char bigFont)
{
    while(*string)
    {
        /* Send each character of the string. */
        ST7579_Char(yPosition, xPosition, *string++, bigFont);
        xPosition += 6 + (bigFont * 12);
    }
}

/***************************************************************************//**
 * @brief Sends an integer number to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param number    - The number.
 * @param bigFont   - Big font selection.
 *
 * @return None.
*******************************************************************************/
void ST7579_Number(unsigned char yPosition,
                   unsigned char xPosition,
                   long number,
                   unsigned char bigFont)
{
    unsigned long mask     = 1000000000;
    unsigned char chNumber = 10;
    unsigned char chIndex  = 0;

    if(number < 0)
    {
        ST7579_Char(yPosition, xPosition, '-', bigFont);
        xPosition += 6 + (bigFont * 12);
        number *= -1;
    }
    else
    {
        if(number == 0)
        {
            ST7579_String(yPosition, xPosition, " 0", bigFont);
        }
        else
        {
            ST7579_Char(yPosition, xPosition, '+', bigFont);
            xPosition += 6 + (bigFont * 12);
        }
    }
    while(mask > number)
    {
        mask /= 10;
        chNumber--;
    }
    for(chIndex = 0; chIndex < chNumber; chIndex++)
    {
        ST7579_Char(yPosition, xPosition, ((number / mask) % 10) + 0x30, bigFont);
        mask /= 10;
        xPosition += 6 + (bigFont * 12);
    }
}

/***************************************************************************//**
 * @brief Sends a float number to ST7579 controller.
 *
 * @param yPosition  - Y address of RAM.
 * @param xPosition  - X address of RAM.
 * @param number     - The number.
 * @param resolution - Float resolution.
 * @param bigFont    - Big font selection.
 *
 * @return None.
*******************************************************************************/
void ST7579_FloatNumber(unsigned char yPosition,
                        unsigned char xPosition,
                        float number,
                        unsigned char resolution,
                        unsigned char bigFont)
{
    unsigned long  mask           = 1000000000;
    unsigned char  chNumber       = 10;
    unsigned char  index          = 0;
    long           display        = 1;
    long           multiplication = 1;


    for(index = 0; index < resolution; index++)
    {
        multiplication *= 10;
    }
    display = (long)(number * multiplication);
    if(display < 0)
    {
        ST7579_Char(yPosition, xPosition, '-', bigFont);
        xPosition += 6 + (bigFont * 12);
        display *= -1;
    }
    else
    {
        if(display == 0)
        {
            xPosition += 6 + (bigFont * 12);
            ST7579_Char(yPosition, xPosition, '0', bigFont);
            xPosition += 6 + (bigFont * 12);
            if(resolution)
            {
                ST7579_Char(yPosition, xPosition, '.', bigFont);
                xPosition += 6 + (bigFont * 12);
            }
            for(index = 0; index < resolution; index++)
            {
                ST7579_Char(yPosition, xPosition, '0', bigFont);
                xPosition += 6 + (bigFont * 12);
            }
            return;
        }
        else
        {
            ST7579_Char(yPosition, xPosition, '+', bigFont);
            xPosition += 6 + (bigFont * 12);
        }
    }
    while((mask > display) && (chNumber > resolution))
    {
        mask /= 10;
        chNumber--;
    }
    if((display > 0) && (display < multiplication))
    {
        ST7579_String(yPosition, xPosition, "0.", bigFont);
        xPosition += 12 + (bigFont * 24);
    }
    for(index = 0; index < chNumber; index++)
    {
        ST7579_Char(yPosition, xPosition, ((display / mask) % 10) + 0x30, bigFont);
        mask /= 10;
        xPosition += 6 + (bigFont * 12);
        if((multiplication != 1) && (mask == (multiplication / 10)))
        {
            ST7579_Char(yPosition, xPosition, '.', bigFont);
            xPosition += 6 + (bigFont * 12);
        }
    }
}

/***************************************************************************//**
 * @brief Sends an integer number in Hexa format to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param number    - The number.
 * @param bigFont   - Big font selection.
 *
 * @return None.
*******************************************************************************/
void ST7579_HexNumber(unsigned char yPosition,
                      unsigned char xPosition,
                      long number,
                      unsigned char bigFont)
{
    unsigned long long mask     = 268435456;
    unsigned char      chNumber = 8;
    unsigned char      chIndex  = 0;
    unsigned char      display  = 0;

    ST7579_String(yPosition, xPosition, "0x", bigFont);
    xPosition += (6 + (bigFont * 12)) * 2;
    if(number == 0)
    {
        ST7579_Char(yPosition, xPosition, '0', bigFont);
    }
    while(mask > number)
    {
        mask /= 16;
        chNumber--;
    }
    for(chIndex = 0; chIndex < chNumber; chIndex++)
    {
        display = (number / mask) % 16;
        mask /= 16;
        if(display <= 9)
        {
            display += 0x30;
        }
        else
        {
            display += 0x37;
        }
        ST7579_Char(yPosition, xPosition, display, bigFont);
        xPosition += 6 + (bigFont * 12);
    }
}

/***************************************************************************//**
 * @brief Sends an image array to ST7579 controller.
 *
 * @param yPosition - Y address of RAM.
 * @param xPosition - X address of RAM.
 * @param image     - The image array.
 * @param width     - The width of the image (pixels).
 * @param height    - The height of the image (pixels).
 *
 * @return None.
*******************************************************************************/
void ST7579_Image(unsigned char yPosition,
                  unsigned char xPosition,
                  const unsigned char image[],
                  unsigned char width,
                  unsigned char height)
{
    unsigned short index = 0;

    for(index = 0; index < (width * height / 8); index++)
    {
        if(!(index % width))
        {
            /* Set the X and Y address of RAM. */
            ST7579_GoTo(yPosition, xPosition);
            yPosition ++;
        }
        ST7579_WriteByte(image[index]);
    }
}
