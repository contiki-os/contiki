/*
 *  Copyright (c) 2008  Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      This file provides Raven LCD support.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef __LCD_H__
#define __LCD_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LCD_SYMBOL_FISRT      0
#define LCD_SYMBOL_COUNT     34

/* Duplicate usage of segments */
#define LCD_SYMBOL_ARR_UP  LCD_SYMBOL_ENV_OP
#define LCD_SYMBOL_ARR_DN  LCD_SYMBOL_ENV_CL

#define LCD_CLCK_FRQ    32768

/** @name Constant defines for making seg_map[] table */
/** @{ */
/*
 - a -
 f   b
 |-g-|
 e   c
 - d -
*/
#define NUM_LCD_SYMBOL_A 0x01
#define NUM_LCD_SYMBOL_B 0x02
#define NUM_LCD_SYMBOL_C 0x04
#define NUM_LCD_SYMBOL_D 0x08
#define NUM_LCD_SYMBOL_E 0x10
#define NUM_LCD_SYMBOL_F 0x20
#define NUM_LCD_SYMBOL_G 0x40
/** @} */

/** @name Constant defines for indexing seg_map[] table */
/** @{ */
#define LCD_SEV_SEG_INDEX_0     (0)
#define LCD_SEV_SEG_INDEX_1     (1)
#define LCD_SEV_SEG_INDEX_2     (2)
#define LCD_SEV_SEG_INDEX_3     (3)
#define LCD_SEV_SEG_INDEX_4     (4)
#define LCD_SEV_SEG_INDEX_5     (5)
#define LCD_SEV_SEG_INDEX_6     (6)
#define LCD_SEV_SEG_INDEX_7     (7)
#define LCD_SEV_SEG_INDEX_8     (8)
#define LCD_SEV_SEG_INDEX_9     (9)
#define LCD_SEV_SEG_INDEX_A     (10)
#define LCD_SEV_SEG_INDEX_B     (11)
#define LCD_SEV_SEG_INDEX_C     (12)
#define LCD_SEV_SEG_INDEX_D     (13)
#define LCD_SEV_SEG_INDEX_E     (14)
#define LCD_SEV_SEG_INDEX_F     (15)
#define LCD_SEV_SEG_INDEX_SPACE (16)
#define LCD_SEV_SEG_INDEX_MINUS (17)
/** @} */

#define lcd_symbol_set_all() lcd_symbol_set_group(0, sizeof(lcd_symbol_chart))
#define lcd_symbol_clr_all() lcd_symbol_clr_group(0, sizeof(lcd_symbol_chart))

typedef enum {
    LCD_NUM_PADDING_ZERO,
    LCD_NUM_PADDING_SPACE
} lcd_padding_t;

typedef enum {
/*  name               = (bit_number << bit_number_offset) | mem_offset*/
    /* Raven */
    LCD_SYMBOL_RAVEN   = (7<<5) |  3,

    /* Audio */
    LCD_SYMBOL_BELL    = (2<<5) | 17,
    LCD_SYMBOL_TONE    = (6<<5) | 17,
    LCD_SYMBOL_MIC     = (3<<5) |  3,
    LCD_SYMBOL_SPEAKER = (2<<5) | 18,

    /* Status */
    LCD_SYMBOL_KEY     = (3<<5) |  2,
    LCD_SYMBOL_ATT     = (7<<5) |  2,

    /* Time */
    LCD_SYMBOL_SUN     = (6<<5) | 13,
    LCD_SYMBOL_MOON    = (6<<5) |  3,
    LCD_SYMBOL_AM      = (2<<5) | 15,
    LCD_SYMBOL_PM      = (6<<5) | 15,

    /* Radio commuication status */
    LCD_SYMBOL_RX      = (6<<5) | 18,
    LCD_SYMBOL_TX      = (5<<5) | 13,
    LCD_SYMBOL_IP      = (7<<5) | 13,
    LCD_SYMBOL_PAN     = (7<<5) | 18,
    LCD_SYMBOL_ZLINK   = (5<<5) |  8,
    LCD_SYMBOL_ZIGBEE  = (5<<5) |  3,

    /* Antenna status */
    LCD_SYMBOL_ANT_FOOT= (5<<5) | 18,
    LCD_SYMBOL_ANT_SIG1= (3<<5) |  0,
    LCD_SYMBOL_ANT_SIG2= (7<<5) |  0,
    LCD_SYMBOL_ANT_SIG3= (3<<5) |  1,
    LCD_SYMBOL_ANT_DIS = (7<<5) |  1,

    /* Battery status */
    LCD_SYMBOL_BAT_CONT= (4<<5) | 18,
    LCD_SYMBOL_BAT_CAP1= (4<<5) |  3,
    LCD_SYMBOL_BAT_CAP2= (4<<5) |  8,
    LCD_SYMBOL_BAT_CAP3= (4<<5) | 13,

    /* Envelope status */
    LCD_SYMBOL_ENV_OP  = (6<<5) |  8,
    LCD_SYMBOL_ENV_CL  = (0<<5) |  4,
    LCD_SYMBOL_ENV_MAIN= (2<<5) |  4,

    /* Temperature */
    LCD_SYMBOL_C       = (6<<5) | 16,
    LCD_SYMBOL_F       = (2<<5) | 16,

    /* Numeric */
    LCD_SYMBOL_MINUS   = (7<<5) |  8,
    LCD_SYMBOL_DOT     = (4<<5) |  4,
    LCD_SYMBOL_COL     = (6<<5) |  4,
} lcd_symbol_t;

typedef enum {
    LCD_WAVE_DEFAULT   = 0,
    LCD_WAVE_LOW_POWER = 1
} lcd_wave_t;

typedef enum {
    LCD_BUFFER_ON  = 0,
    LCD_BUFFER_OFF = 1
} lcd_buffer_t;

typedef enum {
    LCD_BLANKING_OFF = 0,
    LCD_BLANKING_ON  = 1
} lcd_blanking_t;

typedef enum {
    LCD_CLOCK_SYSTEM = 0,
    LCD_CLOCK_EXTERN = 1
} lcd_clock_t;

typedef enum {
    LCD_BIAS_THIRD = 0,
    LCD_BIAS_HALF  = 1
} lcd_bias_t;

typedef enum {
    LCD_PRESCL_16   = 0x0,
    LCD_PRESCL_64   = 0x1,
    LCD_PRESCL_128  = 0x2,
    LCD_PRESCL_256  = 0x3,
    LCD_PRESCL_512  = 0x4,
    LCD_PRESCL_1024 = 0x5,
    LCD_PRESCL_2048 = 0x6,
    LCD_PRESCL_4096 = 0x7,
} lcd_prescl_t;

typedef enum {
    LCD_DIV_1 = 0,
    LCD_DIV_2 = 1,
    LCD_DIV_3 = 2,
    LCD_DIV_4 = 3,
    LCD_DIV_5 = 4,
    LCD_DIV_6 = 5,
    LCD_DIV_7 = 6,
    LCD_DIV_8 = 7
} lcd_div_t;

/** Bit LCDDC2:0 and LCMDT */
typedef enum {
    LCD_DRIVE_300  = 0x0,
    LCD_DRIVE_70   = 0x2,
    LCD_DRIVE_150  = 0x4,
    LCD_DRIVE_450  = 0x6,
    LCD_DRIVE_575  = 0x8,
    LCD_DRIVE_850  = 0xA,
    LCD_DRIVE_1150 = 0xC,
    LCD_DRIVE_HALF = 0xE,
    LCD_DRIVE_FULL = 0xF
} lcd_drive_t;

typedef enum {
    LCD_CONTRAST_2_60 = 0x0,
    LCD_CONTRAST_2_65 = 0x1,
    LCD_CONTRAST_2_70 = 0x2,
    LCD_CONTRAST_2_75 = 0x3,
    LCD_CONTRAST_2_80 = 0x4,
    LCD_CONTRAST_2_85 = 0x5,
    LCD_CONTRAST_2_90 = 0x6,
    LCD_CONTRAST_2_95 = 0x7,
    LCD_CONTRAST_3_00 = 0x8,
    LCD_CONTRAST_3_05 = 0x9,
    LCD_CONTRAST_3_10 = 0xA,
    LCD_CONTRAST_3_15 = 0xB,
    LCD_CONTRAST_3_20 = 0xC,
    LCD_CONTRAST_3_25 = 0xD,
    LCD_CONTRAST_3_30 = 0xE,
    LCD_CONTRAST_3_35 = 0xF
} lcd_contrast_t;

typedef struct {
    union {
        struct {
            unsigned int    blanking: 1;
            unsigned int            : 1;
            unsigned int    buffer  : 1;
            unsigned int            : 3;
            unsigned int    wave    : 1;
            unsigned int            : 1;
        };
        uint8_t             lcdcra;
    };
    union {
        struct {
            unsigned int            : 6;
            unsigned int    bias    : 1;
            unsigned int    clock   : 1;
        };
        uint8_t             lcdcrb;
    };
    union {
        struct {
            unsigned int    div     : 3;
            unsigned int            : 1;
            unsigned int    prescl  : 3;
            unsigned int            : 1;
        };
        uint8_t             lcdfrr;
    };
    union {
        struct {
            unsigned int    contrast: 4;
            unsigned int    drive   : 4;
        };
        uint8_t             lcdccr;
    };
} lcd_config_t;

/*========================= PUBLIC VARIABLES         =========================*/


/*========================= PUBLIC FUNCTIONS         =========================*/

/** Initialization and configuration of the LCD controller
 *
 *  Example:
 *  \code
 *
 *      Configuring LCD with Extern clock (TOSC, 32.768kHz)
 *                         32786 Hz          32786 Hz
 *       frame_rate = ------------------ = ------------- = 32 Hz
 *                   8 * .prescl * .div     8 * 16 * 8
 *
 *      lcd_config_t lcd_config = {
 *          .wave       = LCD_WAVE_LOW_POWER,
 *          .buffer     = LCD_BUFFER_ON,
 *          .blanking   = LCD_BLANKING_OFF,
 *          .clock      = LCD_CLOCK_EXTERN,
 *          .bias       = LCD_BIAS_HALF,
 *          .prescl     = LCD_PRESCL_16,
 *          .div        = LCD_DIV_8,
 *          .drive      = LCD_DRIVE_450,
 *          .contrast   = LCD_CONTRAST_3_30
 *      };
 *
 *      lcd_init(lcd_config);
 *  \endcode
*/
int  lcd_init(void);
void lcd_deinit(void);
uint8_t itobcd(uint8_t inNumber);

/** @name Numeric functions */
/** @{ */
int  lcd_num_puthex(uint16_t numb, lcd_padding_t padding);
int  lcd_num_putdec(int numb, lcd_padding_t padding);
int  lcd_num_clr(void);
void lcd_single_print_dig(uint8_t numb, uint8_t pos);
/** @} */

/** @name Text functions */
/** @{ */
int  lcd_puts(const char* s);
int  lcd_puts_P(const char *s);
int  lcd_puta(size_t length, const uint8_t *s);
int  lcd_putchar(unsigned char c);
/** @} */

/** @name Symbol functions */
/** @{ */
void lcd_symbol_set(lcd_symbol_t symbol);
void lcd_symbol_set_group(int start, int count);
void lcd_symbol_clr(lcd_symbol_t symbol);
void lcd_symbol_clr_group(int start, int count);
/** @} */

/** @name LED Functions */
/** @{ */
void led_on(void);
void led_off(void);
/** @} */

#endif /* __LCD_H__ */

