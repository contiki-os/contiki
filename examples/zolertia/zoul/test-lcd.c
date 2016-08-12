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
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-rgb-lcd-test Test the LCD with RGB backlight
 *
 * Demonstrates the use of the LCD with on-board RGB backlight
 * @{
 *
 * \file
 *         A quick program for testing the Grove's LCD with RGB backlight
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/rgb-bl-lcd.h"
#include "dev/button-sensor.h"
/*---------------------------------------------------------------------------*/
#define SCROLL_PERIOD    (CLOCK_SECOND / 6)
/*---------------------------------------------------------------------------*/
PROCESS(remote_lcd_process, "Grove LCD backlight test");
AUTOSTART_PROCESSES(&remote_lcd_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_lcd_process, ev, data)
{
  PROCESS_BEGIN();

  static uint8_t i = 0;
  static char buf[16];
  static uint16_t counter;

  /* Enable the LCD, default configuration is 2 rows and 16 columns, RGB
   * backlight on with red color, no cursor/blink */
  SENSORS_ACTIVATE(rgb_bl_lcd);

  /* Set the cursor to column 17 and row 0 to not make it visible yet */
  lcd_set_cursor(17, LCD_RGB_1ST_ROW);
  lcd_write("Hello Contiki!");

  /* Now make the text appear from right to left and stay aligned left */
  while(i < 16) {
    etimer_set(&et, SCROLL_PERIOD);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    lcd_scroll_display(LCD_RGB_CURSOR_MOVE_LEFT, 1);
    i++;
    etimer_restart(&et);
  }

  /* wait 2 seconds */
  etimer_set(&et, (CLOCK_SECOND * 2));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Clear the welcome message */
  lcd_clear_display();

  /* Spin the timer and print the counter value on the LCD */
  etimer_set(&et, CLOCK_SECOND);

  lcd_set_cursor(0, LCD_RGB_1ST_ROW);
  lcd_write("Press the button!");

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      snprintf(buf, 15, "Counter: %05u", counter);
      lcd_set_cursor(0, LCD_RGB_2ND_ROW);
      lcd_write(buf);
      printf("Counter: %05u\n", counter);
      counter++;
      etimer_restart(&et);
    } else if(ev == sensors_event) {
      if(data == &button_sensor) {
        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
           BUTTON_SENSOR_PRESSED_LEVEL) {
          printf("Button pressed!!\n");
          lcd_set_cursor(0, LCD_RGB_1ST_ROW);
          lcd_write("Button pressed!!");
        } else {
          lcd_set_cursor(0, LCD_RGB_1ST_ROW);
          lcd_write("Press the button!");
        }
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

