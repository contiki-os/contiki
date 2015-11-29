/*
 * Copyright (c) 2015, Zolertia
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-mp3-wtv020sd
 * @{
 *
 * Driver to control the MP3 WTV020SD board in MP3 mode (GPIO based) and the
 * 2-line serial mode (CLK/DI).  Loop Mode and Key Modes not implemented.
 * More product information available at:
 * http://avrproject.ru/chasy-budilnik/WTV020SD.pdf
 * An example on how to wire with a sound power amplifier and speakers at
 * http://www.hackster.io/zolertia
 * @{
 *
 * \file
 * Header file for the MP3 WTV020SD driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/gpio.h"
#include "mp3-wtv020sd.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/

/*
 * The WTV020SD can be used in MP3 mode (GPIO-controlled) or 2-line mode (CLK
 * and DATA line).  The following pin-out can be implemented without reusing
 * the pins as below (in 2-line mode the CLK/DATA functions replace the VOL+/-
 * keys, others remain the same), but this would require more GPIOs to
 * interface all functions, so we chose the configuration that uses the less
 * number of GPIOs, and emulate all functions available in each mode
 */
#ifndef MP3_WTV020SD_P07_PORT
#define MP3_WTV020SD_P07_PORT    GPIO_B_NUM
#endif
#ifndef MP3_WTV020SD_P07_PIN
#define MP3_WTV020SD_P07_PIN     0
#endif
#ifndef MP3_WTV020SD_P02_PORT
#define MP3_WTV020SD_P02_PORT    GPIO_B_NUM
#endif
#ifndef MP3_WTV020SD_P02_PIN
#define MP3_WTV020SD_P02_PIN     1
#endif
#ifndef MP3_WTV020SD_P06_PORT
#define MP3_WTV020SD_P06_PORT    GPIO_C_NUM
#endif
#ifndef MP3_WTV020SD_P06_PIN
#define MP3_WTV020SD_P06_PIN     1
#endif
#ifndef MP3_WTV020SD_P04_PORT
#define MP3_WTV020SD_P04_PORT    GPIO_B_NUM
#endif
#ifndef MP3_WTV020SD_P04_PIN
#define MP3_WTV020SD_P04_PIN     0
#endif
#ifndef MP3_WTV020SD_P05_PORT
#define MP3_WTV020SD_P05_PORT    GPIO_B_NUM
#endif
#ifndef MP3_WTV020SD_P05_PIN
#define MP3_WTV020SD_P05_PIN     1
#endif
#ifndef MP3_WTV020SD_RESET_PORT
#define MP3_WTV020SD_RESET_PORT  GPIO_B_NUM
#endif
#ifndef MP3_WTV020SD_RESET_PIN
#define MP3_WTV020SD_RESET_PIN   1
#endif

/* The BUSY pin is shared between operation modes */
#define MP3_BUSY_PORT_BASE          GPIO_PORT_TO_BASE(MP3_WTV020SD_P06_PORT)
#define MP3_BUSY_PIN_MASK           GPIO_PIN_MASK(MP3_WTV020SD_P06_PIN)

#define MP3_PLAY_PORT_BASE          GPIO_PORT_TO_BASE(MP3_WTV020SD_P07_PORT)
#define MP3_PLAY_PIN_MASK           GPIO_PIN_MASK(MP3_WTV020SD_P07_PIN)
#define MP3_NEXT_PORT_BASE          GPIO_PORT_TO_BASE(MP3_WTV020SD_P02_PORT)
#define MP3_NEXT_PIN_MASK           GPIO_PIN_MASK(MP3_WTV020SD_P02_PIN)

#define MP3_RESET_PORT_BASE         GPIO_PORT_TO_BASE(MP3_WTV020SD_RESET_PORT)
#define MP3_RESET_PIN_MASK          GPIO_PIN_MASK(MP3_WTV020SD_RESET_PIN)
#define MP3_CLK_PORT_BASE           GPIO_PORT_TO_BASE(MP3_WTV020SD_P04_PORT)
#define MP3_CLK_PIN_MASK            GPIO_PIN_MASK(MP3_WTV020SD_P04_PIN)
#define MP3_DATA_PORT_BASE          GPIO_PORT_TO_BASE(MP3_WTV020SD_P05_PORT)
#define MP3_DATA_PIN_MASK           GPIO_PIN_MASK(MP3_WTV020SD_P05_PIN)

/*---------------------------------------------------------------------------*/
static uint8_t initialized = 0;
static int mp3_line_command(uint16_t cmd);
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_config(uint8_t mode)
{
  if(mode != MP3_WTV020SD_GPIO_MODE && mode != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }

  if(mode == MP3_WTV020SD_GPIO_MODE) {
    GPIO_SOFTWARE_CONTROL(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
    GPIO_SET_OUTPUT(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
    GPIO_SET_PIN(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
    GPIO_SOFTWARE_CONTROL(MP3_NEXT_PORT_BASE, MP3_NEXT_PIN_MASK);
    GPIO_SET_OUTPUT(MP3_NEXT_PORT_BASE, MP3_NEXT_PIN_MASK);
    GPIO_SET_PIN(MP3_NEXT_PORT_BASE, MP3_NEXT_PIN_MASK);
  } else {
    GPIO_SOFTWARE_CONTROL(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
    GPIO_SET_OUTPUT(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
    GPIO_SET_PIN(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
    GPIO_SOFTWARE_CONTROL(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
    GPIO_SET_OUTPUT(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
    GPIO_SET_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
    GPIO_SOFTWARE_CONTROL(MP3_DATA_PORT_BASE, MP3_DATA_PIN_MASK);
    GPIO_SET_OUTPUT(MP3_DATA_PORT_BASE, MP3_DATA_PIN_MASK);
    GPIO_SET_PIN(MP3_DATA_PORT_BASE, MP3_DATA_PIN_MASK);
  }

  GPIO_SOFTWARE_CONTROL(MP3_BUSY_PORT_BASE, MP3_BUSY_PIN_MASK);
  GPIO_SET_INPUT(MP3_BUSY_PORT_BASE, MP3_BUSY_PIN_MASK);

  initialized = mode;
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_gpio_play(void)
{
  if(initialized != MP3_WTV020SD_GPIO_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  GPIO_CLR_PIN(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_gpio_stop(void)
{
  if(initialized != MP3_WTV020SD_GPIO_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  GPIO_SET_PIN(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_gpio_next(void)
{
  if(initialized != MP3_WTV020SD_GPIO_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  GPIO_CLR_PIN(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
  clock_delay_usec(MP3_USEC_DELAY);
  GPIO_SET_PIN(MP3_PLAY_PORT_BASE, MP3_PLAY_PIN_MASK);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_busy(void)
{
  if((initialized != MP3_WTV020SD_GPIO_MODE) &&
     (initialized != MP3_WTV020SD_LINE_MODE)) {
    return MP3_WTV020SD_ERROR;
  }
  if(GPIO_READ_PIN(MP3_BUSY_PORT_BASE, MP3_BUSY_PIN_MASK)) {
    return MP3_WTV020SD_BUSY;
  }
  return MP3_WTV020SD_IDLE;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_reset(void)
{
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  GPIO_CLR_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
  GPIO_SET_PIN(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
  GPIO_CLR_PIN(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
  clock_delay_usec(MP3_USEC_DELAY);
  GPIO_SET_PIN(MP3_RESET_PORT_BASE, MP3_RESET_PIN_MASK);
  GPIO_SET_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
  clock_delay_usec(MP3_USEC_RESET_DELAY);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_sync_play(uint16_t track)
{
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  mp3_line_command(track);
  while(mp3_wtv020sd_busy());
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_async_play(uint16_t track)
{
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  mp3_line_command(track);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_stop(void)
{
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  mp3_line_command(MP3_WTV020SD_STOP_VAL);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_wtv020sd_pause(void)
{
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  mp3_line_command(MP3_WTV020SD_PLAY_PAUSE_VAL);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
mp3_line_command(uint16_t cmd)
{
  uint16_t mask;
  if(initialized != MP3_WTV020SD_LINE_MODE) {
    return MP3_WTV020SD_ERROR;
  }
  GPIO_CLR_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
  clock_delay_usec(MP3_USEC_CMD_DELAY / 10);
  for(mask = 0x8000; mask > 0; mask >> 1) {
    GPIO_CLR_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
    clock_delay_usec(MP3_USEC_CMD_DELAY / 2);
    if(cmd & mask) {
      GPIO_SET_PIN(MP3_DATA_PORT_BASE, MP3_DATA_PIN_MASK);
    } else {
      GPIO_CLR_PIN(MP3_DATA_PORT_BASE, MP3_DATA_PIN_MASK);
    }
    clock_delay_usec(MP3_USEC_CMD_DELAY / 2);
    GPIO_SET_PIN(MP3_CLK_PORT_BASE, MP3_CLK_PIN_MASK);
    clock_delay_usec(MP3_USEC_CMD_DELAY);
    if(mask > 0x0001) {
      clock_delay_usec(MP3_USEC_CMD_DELAY / 10);
    }
  }
  clock_delay_usec(MP3_USEC_CMD_DELAY / 8);
  return MP3_WTV020SD_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
