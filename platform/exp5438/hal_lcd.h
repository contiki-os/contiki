/*******************************************************************************
 *
 *  hal_lcd.h
 *
 *  Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef HAL_LCD_H
#define HAL_LCD_H

#define LCD_BACKLT_OUT      P8OUT
#define LCD_BACKLT_DIR      P8DIR
#define LCD_BACKLT_SEL      P8SEL
#define LCD_BACKLIGHT_PIN   BIT3
#define LCD_CS_RST_DIR      P9DIR
#define LCD_CS_RST_OUT      P9OUT
#define LCD_CS_PIN          BIT6
#define LCD_RESET_PIN       BIT7
#define LCD_SPI_SEL                     P9SEL
#define LCD_SPI_DIR                     P9DIR
#define LCD_MOSI_PIN            BIT1
#define LCD_MISO_PIN            BIT2
#define LCD_CLK_PIN             BIT3

#define LCD_ROW                 110
#define LCD_COL                 138
#define LCD_Size                3505
#define LCD_MEM_Size            110 * 17
#define LCD_Max_Column_Offset   0x10

#define LCD_Last_Pixel          3505

#define LCD_MEM_Row             0x11
#define LCD_Row                 0x20

// Grayscale level definitions
#define PIXEL_OFF               0
#define PIXEL_LIGHT             1
#define PIXEL_DARK              2
#define PIXEL_ON                3

#define INVERT_TEXT             BIT0
#define OVERWRITE_TEXT          BIT2
#define GRAYSCALE_TEXT                  BIT1

/*-------------------------------------------------------------
 *                  Function Prototypes
 * ------------------------------------------------------------*/
extern void halLcdInit(void);
extern void halLcdShutDown(void);
extern void halLcdBackLightInit(void);
extern void halLcdSetBackLight(unsigned char BackLightLevel);
extern unsigned int halLcdGetBackLight(void);
extern void halLcdShutDownBackLight(void);

extern void halLcdSendCommand(unsigned char Data[]);
extern void halLcdSetContrast(unsigned char ContrastLevel);
extern unsigned char halLcdGetContrast(void);
extern void halLcdStandby(void);
extern void halLcdActive(void);

//Move to specified LCD address
extern void halLcdSetAddress(int Address);

//Draw at current segment location
extern void halLcdDrawCurrentBlock(unsigned int Value);
extern void halLcdDrawCurrentLine(const unsigned int *value, int length);

//Draw at specified location by calling
//LCD_Set_Address(Address) & LCD_Draw_Current_Block( value )
extern void halLcdDrawBlock(unsigned int Address, unsigned int Value);

//Read value from LCD CGRAM
extern int halLcdReadBlock(unsigned int Address);

//Clear LCD Screen
extern void halLcdClearScreen(void);

//Invert black to white and vice versa
extern void halLcdReverse(void);

// Draw a Pixel @ (x,y) with GrayScale level
extern void halLcdPixel(int x,  int y, unsigned char GrayScale);

//Draw Line from (x1,y1) to (x2,y2) with GrayScale level
extern void halLcdLine(int x1,  int y1,  int x2,  int y2, unsigned char GrayScale);
extern void halLcdHLine(int x1, int x2, int y, unsigned char GrayScale);
extern void halLcdVLine(int x1, int x2, int y, unsigned char GrayScale);

extern void halLcdCircle(int x, int y, int Radius, int GrayScale);

extern void halLcdImage(const unsigned int Image[], int Columns, int Rows, int x, int y);
extern void halLcdClearImage(int Columns, int Rows,  int x, int y);

//Print String of Length starting at current LCD location
extern void halLcdPrint(char String[], unsigned char TextStyle);

//Print String of Length starting at (x,y)
extern void halLcdPrintXY(char String[], int x, int y, unsigned char TextStyle);

//Print String of Length starting at (x,y)
extern void halLcdPrintLine(char String[], unsigned char Line, unsigned char TextStyle);
extern void halLcdPrintLineCol(char String[], unsigned char Line, unsigned char Col,
                               unsigned char TextStyle);

extern void halLcdCursor(void);
extern void halLcdCursorOff(void);

//Scroll a single row of pixels
extern void halLcdScrollRow(int y);

//Scroll a number of consecutive rows from yStart to yEnd
extern void halLcdHScroll(int yStart, int yEnd);

//Scroll a line of text
extern void halLcdScrollLine(int Line);

#endif /* HAL_LCD_H */
