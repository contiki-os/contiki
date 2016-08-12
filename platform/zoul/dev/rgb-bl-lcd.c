/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-lcd-backlight-lcd
 * @{
 *
 * \file
 *         Grove LCD with RGB backlight driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/rgb-bl-lcd.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
typedef struct {
  uint8_t display_func;
  uint8_t display_ctrl;
  uint8_t display_mode;
  uint8_t num_lines;
  uint8_t cur_line;
} rgb_lcd_config_t;

static rgb_lcd_config_t lcd;
/*---------------------------------------------------------------------------*/
static const unsigned char rgb_color[7][3] =
{
  { 0xFF, 0xFF, 0xFF }, /**< White       */
  { 0xFF, 0x00, 0x00 }, /**< Red         */
  { 0x00, 0xFF, 0x00 }, /**< Green       */
  { 0x00, 0x00, 0xFF }, /**< Blue        */
  { 0xFF, 0xFF, 0x00 }, /**< Yellow      */
  { 0x00, 0xFF, 0xFF }, /**< Purple      */
  { 0x00, 0x00, 0x00 }, /**< Black (off) */
};
/*---------------------------------------------------------------------------*/
static int
lcd_backlight_write_reg(uint8_t addr, uint8_t val)
{
  uint8_t buf[2];
  buf[0] = addr;
  buf[1] = val;

  i2c_master_enable();
  if(i2c_burst_send(LCD_RGB_ADDR, buf, 2) == I2C_MASTER_ERR_NONE) {
    return LCD_RGB_SUCCESS;
  }
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_backlight_color(uint8_t color)
{
  lcd_backlight_write_reg(LCD_RGB_LED_RED, rgb_color[color][0]);
  lcd_backlight_write_reg(LCD_RGB_LED_GREEN, rgb_color[color][1]);
  lcd_backlight_write_reg(LCD_RGB_LED_BLUE, rgb_color[color][2]);

  return LCD_RGB_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
lcd_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("LCD: invalid write values\n");
    return LCD_RGB_ERROR;
  }

  i2c_master_enable();
  if(i2c_burst_send(LCD_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return LCD_RGB_SUCCESS;
  }
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
lcd_cmd(uint8_t value)
{
  uint8_t buf[2];
  buf[0] = LCD_RGB_COMMAND_BYTE;
  buf[1] = value;

  if(lcd_write_reg(buf, 2) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }

  PRINTF("LCD: failed to send command 0x%02X\n", value);
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_clear_display(void)
{
  if(lcd_cmd(LCD_RGB_CLEAR_DISPLAY) == LCD_RGB_SUCCESS) {
    clock_delay_usec(LCD_RGB_DELAY_2MS);
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to clear LCD\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_return_home(void)
{
  if(lcd_cmd(LCD_RGB_RETURN_HOME) == LCD_RGB_SUCCESS) {
    clock_delay_usec(LCD_RGB_DELAY_2MS);
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to return home\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_set_cursor(uint8_t col, uint8_t row)
{
  uint8_t buf[2];
  buf[0] = LCD_RGB_SETDDRAM_ADDR;
  buf[1] = col;
  buf[1] += (!row) ? LCD_RGB_START_1ST_ROW : LCD_RGB_START_2ND_ROW;

  if(lcd_write_reg(buf, 2) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }

  PRINTF("LCD: failed to set cursor\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_display(uint8_t state)
{
  lcd.display_ctrl &= ~LCD_RGB_DISPLAY_ON;
  if(state) {
    lcd.display_ctrl |= LCD_RGB_DISPLAY_ON;
  }

  if(lcd_cmd(LCD_RGB_DISPLAY_CONTROL + lcd.display_ctrl) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to set display\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_cursor(uint8_t state)
{
  lcd.display_ctrl &= ~LCD_RGB_DISPLAY_CURSOR_ON;
  if(state) {
    lcd.display_ctrl |= LCD_RGB_DISPLAY_CURSOR_ON;
  }

  if(lcd_cmd(LCD_RGB_DISPLAY_CONTROL + lcd.display_ctrl) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to set cursor\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_blink(uint8_t state)
{
  lcd.display_ctrl &= ~LCD_RGB_DISPLAY_BLINK_ON;
  if(state) {
    lcd.display_ctrl |= LCD_RGB_DISPLAY_BLINK_ON;
  }

  if(lcd_cmd(LCD_RGB_DISPLAY_CONTROL + lcd.display_ctrl) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to set blink\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_scroll_display(uint8_t direction, uint8_t num)
{
  uint8_t i;

  /* FIXME: add check for num */

  for(i = 0; i < num; i++) {
    if(lcd_cmd(LCD_RGB_CURSOR_SHIFT + LCD_RGB_CURSOR_DISPLAY_MOVE +
               direction) != LCD_RGB_SUCCESS) {
      PRINTF("LCD: failed to set scroll\n");
      return LCD_RGB_ERROR;
    }
  }
  return LCD_RGB_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
lcd_text_direction(uint8_t direction)
{
  lcd.display_mode &= ~LCD_RGB_ENTRY_MODE_LEFT;
  if(direction) {
    lcd.display_mode |= LCD_RGB_ENTRY_MODE_LEFT;
  }

  if(lcd_cmd(LCD_RGB_ENTRY_MODE_SET + lcd.display_mode) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to set text direction\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
int
lcd_autoscroll(uint8_t state)
{
  lcd.display_mode &= ~LCD_RGB_ENTRY_SHIFT_INCREMENT;
  if(state) {
    lcd.display_mode |= LCD_RGB_ENTRY_SHIFT_INCREMENT;
  }

  if(lcd_cmd(LCD_RGB_ENTRY_MODE_SET + lcd.display_mode) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  PRINTF("LCD: failed to set autoscroll\n");
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
lcd_write_byte(int c)
{
  uint8_t buf[2];
  buf[0] = LCD_RGB_SETCGRAM_ADDR;
  buf[1] = c;

  if(lcd_write_reg(buf, 2) == LCD_RGB_SUCCESS) {
    return LCD_RGB_SUCCESS;
  }
  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
uint8_t
lcd_write(const char *s)
{
  uint8_t i = 0;
  while(s && *s != 0) {
    lcd_write_byte(*s++);
    i++;
  }

  PRINTF("LCD: wrote %u bytes\n", i);
  return i;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{

  if(type != LCD_RGB_ACTIVE) {
    PRINTF("LCD: option not supported\n");
    return LCD_RGB_ERROR;
  }

  switch(type) {

  /* Default initialization value is 16 columns and 2 rows */
  case LCD_RGB_ACTIVE:
    if(value) {
      i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
               I2C_SCL_NORMAL_BUS_SPEED);

      lcd.display_func = LCD_RGB_FUNCTION_SET_2_LINE +
                         LCD_RGB_FUNCTION_SET_5x8_DOTS;

      /* wait at least 50ms for the LCD to initialize */
      clock_delay_usec(LCD_RGB_DELAY_50MS);

      /* Send function set command sequence */
      if(lcd_cmd(LCD_RGB_FUNCTION_SET + lcd.display_func) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }
      clock_delay_usec(LCD_RGB_DELAY_4_5MS);

      /* Datasheet instructs to repeat a second time... */
      if(lcd_cmd(LCD_RGB_FUNCTION_SET + lcd.display_func) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }
      clock_delay_usec(LCD_RGB_DELAY_150US);

      /* and a third... */
      if(lcd_cmd(LCD_RGB_FUNCTION_SET + lcd.display_func) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }

      /* Now we can configure everything */
      if(lcd_cmd(LCD_RGB_FUNCTION_SET + lcd.display_func) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }

      /* Turn on the display */
      lcd.display_ctrl = LCD_RGB_DISPLAY_ON + LCD_RGB_DISPLAY_CURSOR_OFF +
                            LCD_RGB_DISPLAY_BLINK_OFF;
      if(lcd_cmd(LCD_RGB_DISPLAY_CONTROL + lcd.display_ctrl) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }

      /* Clear the display */
      if(lcd_clear_display() == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }

      /* Initialize text direction (the LCD supports japanese, cool! */
      lcd.display_mode = LCD_RGB_ENTRY_MODE_LEFT + LCD_RGB_ENTRY_SHIFT_DECREMENT;

      /* configure the entry mode */
      if(lcd_cmd(LCD_RGB_ENTRY_MODE_SET + lcd.display_mode) == LCD_RGB_ERROR) {
        return LCD_RGB_ERROR;
      }

      /* Backlight initialization */
      lcd_backlight_write_reg(LCD_RGB_LED_MODE_1, LCD_RGB_LED_MODE_DEFAULT);
      lcd_backlight_write_reg(LCD_RGB_LED_MODE_2, LCD_RGB_LED_MODE_DEFAULT);
      lcd_backlight_write_reg(LCD_RGB_LED_OUT, LCD_RGB_LED_OUT_PWM_CTRL);

      /* Set the backlight color */
      lcd_backlight_color(LCD_RGB_RED);

      PRINTF("LCD: initialized\n");
      enabled = 1;
      return LCD_RGB_SUCCESS;
    } else {
      lcd_display(LCD_RGB_DISPLAY_OFF);
      lcd_backlight_color(LCD_RGB_BLACK);
      enabled = 0;
    }
  }

  return LCD_RGB_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(rgb_bl_lcd, RGB_BACKLIGHT_LCD, NULL, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
