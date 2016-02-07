/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * OUT OF THE USE OF THIS SOcFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-lcd-backlight-lcd Grove LCD with RGB backlight
 * @{
 *
 * \file
 *         Grove LCD with RGB backlight header
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/* -------------------------------------------------------------------------- */
#ifndef RGB_BL_LCD_H_
#define RGB_BL_LCD_H_
/* -------------------------------------------------------------------------- */
/**
 * \name LCD w/ backlight enums
 * @{
 */
enum {
  LCD_RGB_WHITE  = 0x00,
  LCD_RGB_RED    = 0x01,
  LCD_RGB_GREEN  = 0x02,
  LCD_RGB_BLUE   = 0x03,
  LCD_RGB_BLACK  = 0x04,
  LCD_RGB_YELLOW = 0x05,
  LCD_RGB_PURPLE = 0x06,
};
/* -------------------------------------------------------------------------- */
enum {
  LCD_RGB_1ST_ROW  = 0x00,
  LCD_RGB_2ND_ROW  = 0x01,
};
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name LCD w/ backlight address, registers and bitmasks
 * @{
 */
#define LCD_ADDR                        0x3E
#define LCD_RGB_ADDR                    0x62
/* -------------------------------------------------------------------------- */
#define LCD_RGB_REG_MODE1               0x00
#define LCD_RGB_REG_MODE2               0x01
#define LCD_RGB_REG_OUTPUT              0x08
/* -------------------------------------------------------------------------- */
#define LCD_RGB_COMMAND_BYTE            0x80
/* -------------------------------------------------------------------------- */
#define LCD_RGB_CLEAR_DISPLAY           0x01
#define LCD_RGB_RETURN_HOME             0x02
#define LCD_RGB_ENTRY_MODE_SET          0x04
#define LCD_RGB_DISPLAY_CONTROL         0x08
#define LCD_RGB_CURSOR_SHIFT            0x10
#define LCD_RGB_FUNCTION_SET            0x20
#define LCD_RGB_SETCGRAM_ADDR           0x40
#define LCD_RGB_SETDDRAM_ADDR           0x80
/* -------------------------------------------------------------------------- */
#define LCD_RGB_ENTRY_MODE_RIGHT        0x00
#define LCD_RGB_ENTRY_MODE_LEFT         0x02
#define LCD_RGB_ENTRY_SHIFT_INCREMENT   0x01
#define LCD_RGB_ENTRY_SHIFT_DECREMENT   0x00
/* -------------------------------------------------------------------------- */
#define LCD_RGB_DISPLAY_ON              0x04
#define LCD_RGB_DISPLAY_OFF             0x00
#define LCD_RGB_DISPLAY_CURSOR_ON       0x02
#define LCD_RGB_DISPLAY_CURSOR_OFF      0x00
#define LCD_RGB_DISPLAY_BLINK_ON        0x01
#define LCD_RGB_DISPLAY_BLINK_OFF       0x00
/* -------------------------------------------------------------------------- */
#define LCD_RGB_CURSOR_DISPLAY_MOVE     0x08
#define LCD_RGB_CURSOR_MOVE             0x00
#define LCD_RGB_CURSOR_MOVE_RIGHT       0x04
#define LCD_RGB_CURSOR_MOVE_LEFT        0x00
/* -------------------------------------------------------------------------- */
#define LCD_RGB_FUNCTION_SET_8BIT       0x10
#define LCD_RGB_FUNCTION_SET_4BIT       0x00
#define LCD_RGB_FUNCTION_SET_2_LINE     0x08
#define LCD_RGB_FUNCTION_SET_1_LINE     0x00
#define LCD_RGB_FUNCTION_SET_5x10_DOTS  0x04
#define LCD_RGB_FUNCTION_SET_5x8_DOTS   0x00
/* -------------------------------------------------------------------------- */
#define LCD_RGB_LED_MODE_1              0x00
#define LCD_RGB_LED_MODE_2              0x01
#define LCD_RGB_LED_OUT                 0x08
/* -------------------------------------------------------------------------- */
#define LCD_RGB_LED_RED                 0x04
#define LCD_RGB_LED_GREEN               0x03
#define LCD_RGB_LED_BLUE                0x02
/* -------------------------------------------------------------------------- */
#define LCD_RGB_LED_MODE_DEFAULT        0x00
#define LCD_RGB_LED_OUT_PWM_CTRL        0xAA
/* -------------------------------------------------------------------------- */
#define LCD_RGB_DELAY_50MS              50000
#define LCD_RGB_DELAY_4_5MS             4500
#define LCD_RGB_DELAY_150US             150
#define LCD_RGB_DELAY_2MS               2000
/* -------------------------------------------------------------------------- */
#define LCD_RGB_START_1ST_ROW           0x80
#define LCD_RGB_START_2ND_ROW           0xC0
/* -------------------------------------------------------------------------- */
#define LCD_RGB_ACTIVE                  SENSORS_ACTIVE
#define LCD_RGB_ERROR                   (-1)
#define LCD_RGB_SUCCESS                 0x00
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name TSL2563 return and command values
 * @{
 */
/* LCD functions */
uint8_t lcd_write(const char *s);
int lcd_set_cursor(uint8_t col, uint8_t row);
int lcd_autoscroll(uint8_t state);
int lcd_scroll_display(uint8_t direction, uint8_t num);
int lcd_blink(uint8_t state);
int lcd_clear_display(void);
int lcd_return_home(void);
int lcd_display(uint8_t state);
int lcd_cursor(uint8_t state);
int lcd_text_direction(uint8_t direction);

/* Backlight functions */
int lcd_backlight_color(uint8_t color);
/** @} */
/* -------------------------------------------------------------------------- */
#define RGB_BACKLIGHT_LCD "LCD with RGB backlight"
extern const struct sensors_sensor rgb_bl_lcd;
/* -------------------------------------------------------------------------- */
#endif /* ifndef RGB_BL_LCD_ */
/**
 * @}
 * @}
 */
