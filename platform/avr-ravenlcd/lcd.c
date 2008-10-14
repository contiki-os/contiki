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

#include "lcd.h"

/**
 * \addtogroup lcd
 * \{
*/

typedef enum {
    LCD_DUTY_STATIC = 0,
    LCD_DUTY_HALF   = 1,
    LCD_DUTY_THIRD  = 2,
    LCD_DUTY_QUART  = 3
} lcd_duty_t;

typedef enum {
    LCD_PM_0_12 = 0x0,
    LCD_PM_0_14 = 0x1,
    LCD_PM_0_16 = 0x2,
    LCD_PM_0_18 = 0x3,
    LCD_PM_0_20 = 0x4,
    LCD_PM_0_22 = 0x5,
    LCD_PM_0_23 = 0x6,
    LCD_PM_0_24 = 0x7,
    LCD_PM_0_26 = 0x8,
    LCD_PM_0_28 = 0x9,
    LCD_PM_0_30 = 0xA,
    LCD_PM_0_32 = 0xB,
    LCD_PM_0_34 = 0xC,
    LCD_PM_0_36 = 0xD,
    LCD_PM_0_38 = 0xE,
    LCD_PM_0_39 = 0xF
} lcd_pm_t;

#if defined( DOXYGEN )
static const seg_map[];
static const LCD_character_table[];
static const seg_inf[];
static const lcd_symbol_chart[LCD_SYMBOL_COUNT];
#else  /* !DOXYGEN */
/** \name Mapping of segments for different characters */
/** \{ */
static const unsigned char seg_map[] PROGMEM = {
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F                 , /* 0 */
                         NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C                                                                    , /* 1 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|                 NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|                 NUM_LCD_SYMBOL_G, /* 2 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|                                  NUM_LCD_SYMBOL_G, /* 3 */
                         NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|                                  NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* 4 */
        NUM_LCD_SYMBOL_A|                 NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|                 NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* 5 */
        NUM_LCD_SYMBOL_A|                 NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* 6 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C                                                                    , /* 7 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* 8 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|                 NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* 9 */
        NUM_LCD_SYMBOL_A|NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|                 NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* A */
                                          NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* B */
        NUM_LCD_SYMBOL_A|                                  NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F                 , /* C */
                         NUM_LCD_SYMBOL_B|NUM_LCD_SYMBOL_C|NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|                 NUM_LCD_SYMBOL_G, /* D */
        NUM_LCD_SYMBOL_A|                                  NUM_LCD_SYMBOL_D|NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* E */
        NUM_LCD_SYMBOL_A|                                                   NUM_LCD_SYMBOL_E|NUM_LCD_SYMBOL_F|NUM_LCD_SYMBOL_G, /* F */
                                                                                                                             0, /* ' ' (space) */
                                                                                                              NUM_LCD_SYMBOL_G  /*  - (minus) */
};
/** \} */

/* Look-up tables for 14-segment characters */
static const unsigned int LCD_character_table[] PROGMEM = /* Character definitions table. */
{
	0x0000,		/* '*' (?) */
	0x2830,		/* '+' */
	0x0000,		/* ',' (Not defined) */
	0x0810,		/* '-' */
	0x0200,		/* '.' */
	0x0240,		/* '/' */
	0x93C5,		/* '0' */
	0x80C0,		/* '1' */
	0x1994,		/* '2' */
	0x9894,		/* '3' */
	0x8891,		/* '4' */
	0x9815,		/* '5' */
	0x9915,		/* '6' */
	0x8084,		/* '7' */
	0x9995,		/* '8' */
	0x9895,		/* '9' */
	0x0000,		/* ':' (Not defined) */
	0x0000,		/* ';' (Not defined) */
	0x0000,		/* '<' (Not defined) */
	0x0000,		/* '=' (Not defined) */
	0x0202,		/* '>' */
	0x0000,		/* '?' (Not defined) */
	0x8E53,		/* '@' (redefined as '%') */
	0x8995,		/* 'A' (+ 'a') */
	0xB8A4,		/* 'B' (+ 'b') */
	0x1105,		/* 'C' (+ 'c') */
	0xB0A4,		/* 'D' (+ 'd') */
	0x1915,		/* 'E' (+ 'e') */
	0x0915,		/* 'F' (+ 'f') */
	0x9905,		/* 'G' (+ 'g') */
	0x8991,		/* 'H' (+ 'h') */
	0x2020,		/* 'I' (+ 'i') */
	0x9180,		/* 'J' (+ 'j') */
	0x0551,		/* 'K' (+ 'k') */
	0x1101,		/* 'L' (+ 'l') */
	0x81C3,		/* 'M' (+ 'm') */
	0x8583,		/* 'N' (+ 'n') */
	0x9185,		/* 'O' (+ 'o') */
	0x0995,		/* 'P' (+ 'p') */
	0x9585,		/* 'Q' (+ 'q') */
	0x0D95,		/* 'R' (+ 'r') */
	0x1406,		/* 'S' (+ 's') */
	0x2024,		/* 'T' (+ 't') */
	0x9181,		/* 'U' (+ 'u') */
	0x0341,		/* 'V' (+ 'v') */
	0x8781,		/* 'W' (+ 'w') */
	0x0642,		/* 'X' (+ 'x') */
	0x2042,		/* 'Y' (+ 'y') */
	0x1244,		/* 'Z' (+ 'z') */
	0x0000,		/* '[' (Not defined) */
	0x0000,		/* '\' (Not defined) */
	0x0000,		/* ']' (Not defined) */
	0x0000,		/* '^' (Not defined) */
	0x0000,		/* '_' (Not defined) */
  0x0004,   /* A */
  0x0080,   /* B */
  0x8000,   /* C */
  0x1000,   /* D */
  0x0100,   /* E */
  0x0001,   /* F */
  0x0002,   /* G */
  0x0020,   /* H */
  0x0040,   /* J */
  0x0800,   /* K */
  0x0400,   /* L */
  0x2000,   /* M */
  0x0200,   /* N */
  0x0010,   /* O */
  0x0000,
  0x0000,
  0x0000
};

/** \brief Seven segment reference guide in flash. */
static const unsigned char seg_inf[] PROGMEM = {
        2<<5|19, /* A */
        1<<5|19, /* B */
        1<<5|9,  /* C */
        2<<5|4,  /* D */
        2<<5|9,  /* E */
        2<<5|14, /* F */
        1<<5|14  /* G */
};

/** \brief LCD symbol chart located in flash. */
static const lcd_symbol_t lcd_symbol_chart[LCD_SYMBOL_COUNT] PROGMEM= {
    /* Raven */
    LCD_SYMBOL_RAVEN   ,

    /* Audio */
    LCD_SYMBOL_BELL    ,
    LCD_SYMBOL_TONE    ,
    LCD_SYMBOL_MIC     ,
    LCD_SYMBOL_SPEAKER ,

    /* Status */
    LCD_SYMBOL_KEY     ,
    LCD_SYMBOL_ATT     ,

    /* Time */
    LCD_SYMBOL_SUN     ,
    LCD_SYMBOL_MOON    ,
    LCD_SYMBOL_AM      ,
    LCD_SYMBOL_PM      ,

    /* Radio comus */
    LCD_SYMBOL_RX      ,
    LCD_SYMBOL_TX      ,
    LCD_SYMBOL_IP      ,
    LCD_SYMBOL_PAN     ,
    LCD_SYMBOL_ZLINK   ,
    LCD_SYMBOL_ZIGBEE  ,

    /* Antenna status */
    LCD_SYMBOL_ANT_FOOT,
    LCD_SYMBOL_ANT_SIG1,
    LCD_SYMBOL_ANT_SIG2,
    LCD_SYMBOL_ANT_SIG3,
    LCD_SYMBOL_ANT_DIS ,

    /* Battery status */
    LCD_SYMBOL_BAT_CONT,
    LCD_SYMBOL_BAT_CAP1,
    LCD_SYMBOL_BAT_CAP2,
    LCD_SYMBOL_BAT_CAP3,

    /* Envelope status */
    LCD_SYMBOL_ENV_OP  ,
    LCD_SYMBOL_ENV_CL  ,
    LCD_SYMBOL_ENV_MAIN,

    /* Temperature */
    LCD_SYMBOL_C       ,
    LCD_SYMBOL_F       ,

    /* Numeric */
    LCD_SYMBOL_MINUS   ,
    LCD_SYMBOL_DOT     ,
    LCD_SYMBOL_COL
};
#endif /* !DOXYGEN */

/** LCD text buffer */
static unsigned char lcd_text[20];

/** Textd buffer read pointer for text field in LCD display. When ptr>0 characters in front will be cleared (space) */
static int lcd_text_rd_ptr = 0;

/** Text pointer for writing new chars to text buffer */
static int lcd_text_wr_ptr = 0;

static bool lcd_scroll_enable;
static int  lcd_scroll_prescale;
static int  lcd_scroll_prescale_value;
static int  lcd_num_print(uint16_t numb, bool negative, lcd_padding_t padding);
static void lcd_nmb_print_dig(uint8_t val, int dig);
static int  lcd_text_sl(void);
static int  lcd_text_put(const char* s, int pos);
static int  lcd_char_put(unsigned char c, int pos);

/*---------------------------------------------------------------------------*/

/**
 *  \brief This function will initialize the proper settings for the LCD driver.
 *
 *  This ATmega3290p can directly support an LCD through register mapping.
 *
 *  \return 0
*/
int
lcd_init(void)
{
    /*
     * Configuring LCD with Extern clock (TOSC, 32.768kHz)
     *                      32786 Hz          32786 Hz
     *  frame_rate = ------------------ = ------------- = 32 Hz
     *               8 * .prescl * .div     8 * 16 * 8
     */

    lcd_config_t lcd_config ;
    lcd_config.blanking   = LCD_BLANKING_OFF;
    lcd_config.buffer     = LCD_BUFFER_ON;
    lcd_config.wave       = LCD_WAVE_LOW_POWER;
    lcd_config.clock      = LCD_CLOCK_EXTERN;
    lcd_config.bias       = LCD_BIAS_HALF;
    lcd_config.prescl     = LCD_PRESCL_16;
    lcd_config.div        = LCD_DIV_8;
    lcd_config.drive      = LCD_DRIVE_450;
    lcd_config.contrast   = LCD_CONTRAST_3_30;

    /* Enable module */
    PRR &= ~(1 << PRLCD);

    /* Configure and enable LCD controller */
    LCDCRB = lcd_config.lcdcrb|(LCD_PM_0_39<<LCDPM0)|(LCD_DUTY_QUART<<LCDMUX0); /* Add port mask/mux */
    LCDFRR = lcd_config.lcdfrr;
    LCDCCR = lcd_config.lcdccr;
    LCDCRA = lcd_config.lcdcra|(1<<LCDEN)|(1<<LCDIE); /* Add interrupt- and LCD- enable */

    /* clear screen */
    lcd_symbol_clr_all();

    /* Calculate scrolling value */
    lcd_scroll_prescale_value = LCD_CLCK_FRQ/128;
    lcd_scroll_prescale_value >>= (lcd_config.prescl == 0) ? 4 : (5+lcd_config.prescl);
    lcd_scroll_prescale_value /= (lcd_config.div+1);
    lcd_scroll_prescale_value = (lcd_scroll_prescale_value==0) ? 1 : lcd_scroll_prescale_value;
    lcd_scroll_prescale = lcd_scroll_prescale_value;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will disable the LCD operation.
*/
void
lcd_deinit(void)
{
    while (!(LCDCRA & (1<<LCDIF)))
        ;
    /*
     * Set LCD Blanking and clear interrupt flag
     * by writing a logical one to the flag.
     */

    LCDCRA = (1<<LCDEN)|(1<<LCDIF)|(1<<LCDBL);
    /* Wait until LCD Blanking is effective. */
    while ( !(LCDCRA & (1<<LCDIF)) )
        ;

    /* Disable LCD */
    LCDCRA = (0<<LCDEN) | (0<<LCDIE);

    /* power LCD down */
    PRR |= (1 << PRLCD);
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will convert the incoming decimal number to BCD.
 *
 *  \param inNumber Decimal number to convert
 *
 *  \return newByte The converted deicmal number as byte.
*/
uint8_t
itobcd(uint8_t inNumber)
{
	int newByte;

	newByte = 0;

	while (inNumber >= 10){
		inNumber -= 10;
		newByte++;
	}

	newByte = newByte << 4;
	newByte = (newByte | inNumber);

	return newByte;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a HEX value on the LCD that represents the input
 *  parameter.
 *
 *  \param numb Number to display as HEX.
 *  \param padding This pads the location to place the value on the LCD.
 *
 *  \return lcd_num_print()
*/
int
lcd_num_puthex(uint16_t numb, lcd_padding_t padding)
{
    return lcd_num_print(numb, false, padding);
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a DEC value on the LCD that represents the input
 *  parameter.
 *
 *  \param numb Number to display as DEC.
 *  \param padding This pads the location to place the value on the LCD.
 *
 *  \return lcd_num_print()
*/
int
lcd_num_putdec(int numb, lcd_padding_t padding)
{
    uint16_t bcd;

    /* Check for overflow */
    if (numb > 9999) {
        numb = 9999;
    } else if (numb < -9999) {
        numb = -9999;
    }

    /* Convert to BCD */
    bcd = itobcd(abs(numb));

    /* Print */
    return lcd_num_print(bcd, (bool)(numb<0), padding);
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will clear numbers displayed on the LCD.
 *
 *  \return 0
*/
int
lcd_num_clr(void)
{
    volatile unsigned char* lcd_data = (volatile unsigned char*)0xEC;
    int i,j;

    for (i=0;i<4;++i){
        for (j=0;j<7;++j){
            lcd_data[pgm_read_byte(&seg_inf[j])&0x1F] &= ~((pgm_read_byte(&seg_inf[j])>>5)<<(i*2));
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a string of characters out to the LCD.
 *
 *  \param s First character pointer of string.
 *
 *  \return 0
*/
int
lcd_puts(const char* s)
{
    strcpy((char*)lcd_text, s);
    lcd_text_wr_ptr = strlen(s);
    lcd_text_rd_ptr = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 1);

    lcd_scroll_enable = (lcd_text_wr_ptr > 7) ? true : false;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a string of characters of a certain length out to the LCD.
 *
 *  \param length Length of string to print.
 *  \param s First character pointer of string.
 *
 *  \return 0
*/
int
lcd_puta(size_t length, const uint8_t *s)
{
    memcpy((void*)lcd_text, (void const*)s, length);
    lcd_text_wr_ptr = length;
    lcd_text_rd_ptr = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 1);

    lcd_scroll_enable = (lcd_text_wr_ptr > 7) ? true : false;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a string out to the LCD from Flash.
 *
 *  \param s First character pointer of the string located in Flash
 *
 *  \return 0
*/
int
lcd_puts_P(const char *s)
{
    strcpy_P((char*)lcd_text, s);
    lcd_text_wr_ptr = strlen_P(s);
    lcd_text_rd_ptr = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 1);

    lcd_scroll_enable = (lcd_text_wr_ptr > 7) ? true : false;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a single character out to the LCD.
 *
 *  \param c Character to display on LCD.
 *
 *  \return 0
*/
int
lcd_putchar(unsigned char c)
{
    lcd_text[lcd_text_wr_ptr++] = c;
    lcd_text[lcd_text_wr_ptr] = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 1);

    lcd_scroll_enable = (lcd_text_wr_ptr > 7) ? true : false;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will enable any of the symbols on the Raven LCD.
 *
 *  \param symbol Specific symbol to enable on the LCD.
*/
void
lcd_symbol_set(lcd_symbol_t symbol)
{
    unsigned char mem_offset;
    unsigned char bit_offset;
    volatile unsigned char* lcd_data = (volatile unsigned char*)0xEC;

    /* Symbol format = bbbnnnnn where b is bit and n is offset */
    bit_offset = (symbol >> 5);
    mem_offset = (symbol & 0x1F);
    if ( mem_offset >= 20 ){
        return;  /* Data out of range of the LCD registers */
    }
    lcd_data = lcd_data + mem_offset;	/* Point to the  relevant LCDDR */

    *lcd_data = *lcd_data | ( 1 << bit_offset);
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will clear any symbol on the Raven LCD.
 *
 *  \param symbol Specific symbol to clear from the LCD.
*/
void
lcd_symbol_clr(lcd_symbol_t symbol)
{
    unsigned char offset;
    unsigned char setbit;
    volatile unsigned char* lcd_data = (volatile unsigned char*)0xEC;

    /* symbol format = bbbnnnnn where b is bit and n is offset */
    setbit = (symbol >> 5);
    offset = (symbol & 0x1F);
    if ( offset >= 20 ){
        return;  /* data out of range of the LCD registers */
    }

    lcd_data = lcd_data + offset;	/* Point to the  relevant LCDDR */

    *lcd_data = *lcd_data & ~( 1 << setbit);
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will enable a group of symbols from the lcd_symbol_chart.
 *
 *  \param start Position of table to start from.
 *  \param count Number of symbols to enable from start position.
*/
void
lcd_symbol_set_group(int start, int count)
{
    count = (start + count)>LCD_SYMBOL_COUNT ?
        LCD_SYMBOL_COUNT - start :
        count;
    int i;

    for(i=start; i<start+count; ++i){
        lcd_symbol_set(pgm_read_byte(&lcd_symbol_chart[i]));
    }
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will disable a group of symbols from the lcd_symbol_chart.
 *
 *  \param start Position of table to start from.
 *  \param count Number of symbols to disable from start position.
*/
void
lcd_symbol_clr_group(int start, int count)
{
    count = (start + count)>LCD_SYMBOL_COUNT ?
        LCD_SYMBOL_COUNT - start :
        count;
    int i;

    for(i=start; i<count; ++i){
        lcd_symbol_clr(pgm_read_byte(&lcd_symbol_chart[i]));
    }
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will print a number to the LCD with the following parameters.
 *
 *  \param numb Number to display on LCD.
 *  \param negative Display negative sign in the next digit field.
 *  \param padding This pads the location to place the value on the LCD.
 *
 *  \return 0
*/
static int
lcd_num_print(uint16_t numb, bool negative, lcd_padding_t padding)
{
    int i;
    for (i=0;i<4;/**/) {
        /* Get segments for this digit and print it */
        lcd_nmb_print_dig(pgm_read_byte(&seg_map[(numb&(0xF<<4*i))>>4*i]), i);

        /* If rest of number is zero */
        if (++i<4) {
            if (numb >> 4*i == 0) {
                if (negative == true) { /* print a 'minus' in the next digit field */
                    lcd_nmb_print_dig(pgm_read_byte(&seg_map[(padding == LCD_NUM_PADDING_ZERO) ? LCD_SEV_SEG_INDEX_0 : LCD_SEV_SEG_INDEX_MINUS]), i++);
                    if (padding == LCD_NUM_PADDING_ZERO) {
                        lcd_symbol_set(LCD_SYMBOL_MINUS);
                    }
                } else {
                    lcd_symbol_clr(LCD_SYMBOL_MINUS);
                }
                while (i<4){
                    lcd_nmb_print_dig(pgm_read_byte(&seg_map[(padding == LCD_NUM_PADDING_ZERO) ? LCD_SEV_SEG_INDEX_0 : LCD_SEV_SEG_INDEX_SPACE]), i++);
                }
            }
        } else {
            if (negative == true) {
                lcd_symbol_set(LCD_SYMBOL_MINUS);
            } else {
                lcd_symbol_clr(LCD_SYMBOL_MINUS);
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will print a number according to the segment map of the LCD.
 *
 *  \param val Number that is to be matched to appropriate segments.
 *  \param dig Segment to enable
*/
static void
lcd_nmb_print_dig(uint8_t val, int dig)
{
    volatile unsigned char* lcd_data = (volatile unsigned char*)0xEC;
    int j;

    for (j=0;j<7;++j){
        if (val & (1<<j)) {
            lcd_data[pgm_read_byte(&seg_inf[j])&0x1F] |= (pgm_read_byte(&seg_inf[j])>>5)<<(dig*2);
        }
        else {
            lcd_data[pgm_read_byte(&seg_inf[j])&0x1F] &= ~((pgm_read_byte(&seg_inf[j])>>5)<<(dig*2));
        }
    }
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will scroll the test on the LCD.
 *
 *  \return 0
*/
static int
lcd_text_sl(void)
{
    static int pos = 1;
    if (lcd_text[lcd_text_rd_ptr] == 0) {
        lcd_text_rd_ptr = 0;
        pos = 7;
    }
    else {
        if (pos){
            pos--;
        }
        else {
            lcd_text_rd_ptr++;
        }
    }

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], pos);
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put test out to the LCD at a certain location padded with
 *  spaces.
 *
 *  \param s First character pointer to the string of test to print.
 *  \param pos Count of spaces entered before printing the text.
 *
 *  \return 0
*/
static int
lcd_text_put(const char* s, int pos)
{
    int i;

    /* Pad with spaces in front if offset > 0 */
    for (i=1; i<pos; i++) {
         lcd_char_put(' ', i);
    }

    /* Print characters, overwrite with spaces at end if necessary */
    for ( i=pos; i<=7; ++i) {
        if (*s == 0) {
            lcd_char_put(' ', i);
        }
        else {
            lcd_char_put( (unsigned char)*s++, i);
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will put a single char out to the LCD by looking up the
 *  proper segments.
 *
 *  \param c Character to display on the LCD.
 *  \param pos This will add spaces for positioning the text on the LCD.
 *
 *  \return 0
*/
static int
lcd_char_put(unsigned char c, int pos)
{
	unsigned int seg, segMask;
	unsigned char i;
	unsigned char mask, nibble, nibbleMask;

	volatile unsigned char* lcd_data = (volatile unsigned char*)0xEC;
    unsigned char lcd_reg;

    if (pos > 7){
      return EOF;
    }

	/* Lookup character table for segmet data */
	if (((c >= '*') && (c <= 'z')) || (c == ' ')){
		if (c >= 'a' ){
            c &= ~0x20; /* c is in character_table. Convert to upper if necessarry. */
        }
        if (c == ' ') {
            c = 0x00;
        }
		else {
            c -= '*';
        }
		if ( c > 0x35 ){
		   return EOF;     /* c points outside array */
		}
		else{
			seg = pgm_read_dword(&LCD_character_table[c]);
		}
	}
	else {
		return EOF;		/* ASCII code out of range */
	}


	/* Adjust mask according to digit */
	segMask = 0x4008;  /* masking out two bits */

	i = pos-1;		/*i used as pointer offset */
	i >>= 1;
	lcd_data += i;	/* Point to the first relevant LCDDR; i = {0,0,1,1,2,2} */

	i = 4;			/*i used as loop counter */
	do{
        nibble = seg & 0x000F;
        nibbleMask = segMask & 0x000F;

        seg >>= 4;
        segMask >>= 4;

        if (pos & 0x01) {
            mask = 0xF0 | nibbleMask;
        }
        else {
            nibble <<= 4;
            mask = 0x0F | ( nibbleMask <<4 );
        }
        lcd_reg = *lcd_data;
        *lcd_data |= (lcd_reg & mask) | nibble;  /* Write new bit values */

        lcd_reg = *lcd_data;
        *lcd_data &= (lcd_reg & mask) | nibble;

        lcd_data += 5;
    } while ( --i );

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This is the LCD Start of Frame Interrupt Subroutine.
 *
 *  This interrupt fires at the beginning of a new frame.
*/
ISR
(LCD_vect)
{
    if (lcd_scroll_enable) {
        if (--lcd_scroll_prescale == 0) {
            lcd_text_sl();
            lcd_scroll_prescale = lcd_scroll_prescale_value;
        }
    }
}

/*---------------------------------------------------------------------------*/

/**
 * \brief Turns the Raven nose LED on.
*/
void
led_on(void)
{
    DDRB  |=  0x80;
    PORTB &= ~0x80;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief Turns the Raven nose LED off.
*/
void
led_off(void)
{
    DDRB &= ~0x80;
    PORTB |= 0x80;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will add the passed in number to any of the four locations of
 *  the four digit segment display on the LCD.
 *
 *  \param numb Number to display.
 *  \param pos Position to display number.
*/
void
lcd_single_print_dig(uint8_t numb, uint8_t pos)
{
    lcd_nmb_print_dig(pgm_read_byte(&seg_map[numb]), pos);
}

/** \}   */
