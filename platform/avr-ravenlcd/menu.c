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
 *      This file operates the menu flow chart described in the readme
 *      notes. This will create the proper commands needed to control the 1284p.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include <avr/eeprom.h>
#include <util/delay.h>
#include "menu.h"
#include "main.h"
#include "lcd.h"
#include "key.h"
#include "uart.h"
#include "sleep.h"
#include "temp.h"
#include "beep.h"

uint8_t sleep_count;
uint8_t ping_count;
uint8_t ping_response;
bool ping_mode;
bool timeout_flag;
bool temp_flag;
bool temp_mode;
bool auto_temp=true;

/**
 *  \addtogroup lcd
 *  \{
*/

/*---------------------------------------------------------------------------*/

/**
 *   \brief This function will convert decimal to ascii.
 *
 *   \param val Decimal value to convert.
 *   \param str Address location to store converted value.
*/
void
dectoascii(uint8_t val, char *str)
{
    *(str+1) = (val % 10) + '0';
    *str = (val / 10) + '0';
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will convert a signed decimal number to ASCII.
 *
 *   \param n Signed number
 *   \param str Pointer to store converted value.
 *
 *   \return *p Address of stored conversion.
*/
uint8_t
*signed_dectoascii(int16_t n, uint8_t *str)
{
    uint8_t * p = str;
    uint8_t neg = 0;

    if(n < 0){
        neg = 1;
        n = -n;
    }

    *p-- = 0x00;

    /* Determine the unit of conversion. */
    if(temp_mode == TEMP_UNIT_CELCIUS){
        /* Add ASCII C to string. */
        *p-- = 'C';
    }
    else{
        /* Add ASCII F to string. */
        *p-- = 'F';
    }

    /* Add a space before unit symbol. */
    *p-- = ' ';

    /* For zero, just print zero. */
    if (!n){
        *p = '0';
        return p;
    }

    while (n){
        *p-- = (n%10) + '0';
        n/= 10;
    }

    if (neg){
        *p-- = '-';
    }

    return ++p;
}

/*---------------------------------------------------------------------------*/

/**
 *  \brief This will check for DEBUG mode after power up.
*/
void
eeprom_init(void)
{
    uint8_t val;
    if(0xFF == eeprom_read_byte(EEPROM_DEBUG_ADDR)){
        /* Disable - Reverse logic. */
        val = 1;
        menu_debug_mode(&val);
    }
    else{
        /* Enable - Reverse logic. */
        val = 0;
        menu_debug_mode(&val);
    }
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will start a sleep operation.
 *
 *   \param val Used for remembering the new menu to display after a wakeup.
*/
void
menu_run_sleep(uint8_t *val)
{
    /* Turn off LED, LCD, ADC, Timer 1, SPI */
    led_off();
    lcd_deinit();
 	key_deinit();
    PRR |= (1 << PRTIM1) | (1 << PRSPI);

    /* Tell the 1284P to turn off the radio and sleep */
	sleep_count=0;
    uart_serial_send_frame(SEND_SLEEP, 1, (uint8_t *)&sleep_count);

    /* Turn off UART when transmission is complete */
	while(!(UCSR0A & (1 << TXC0)));
    _delay_us(10000); //deinit trash clears done flag on 1284p
	uart_deinit();

    /* Go to sleep until button is pushed */
    sleep_now(0);

    /* Yawn, waking up, turn on LCD with Raven Logo */
    lcd_init();
    lcd_symbol_set(LCD_SYMBOL_RAVEN);

	/* Disable interrupts before powering everything up */
    cli();
    key_init();
    PRR &= ~((1 << PRTIM1) | (1 << PRSPI));
 	uart_init();

    /* Enable interrupts, Wake up 1284p and radio */
	sei();
    sleep_wakeup();
//	uart_init();//flush receive buffer

    /* Wait for buttons up */
    while (key_state_get() != KEY_NO_KEY)
        ;
    if (is_button()){
        get_button();
    }
}
/*---------------------------------------------------------------------------*/

/**
 *   \brief This will start a sleep with wakes for temperature measurement and web requests.
 *
 *   \param val Used for remembering the new menu to display after a wakeup.
*/
void
menu_run_doze(uint8_t *val)
{
    /* Turn off LED, LCD */
    led_off();
    lcd_deinit();

    /* Debounce */
    while (key_state_get() != KEY_NO_KEY) ;
     
    /* Stay in doze loop until button is pressed*/
    while (ENTER_PORT & (1<<ENTER_PIN)) {
 
     /* Tell 1284p to sleep for 4 seconds */
	 /* It will ignore the request if TCP/IP sessions are active */
     /* Alter these timings as desired, or comment out to sleep only the 3290p */
		sleep_count=4;
        uart_serial_send_frame(SEND_SLEEP, 1, (uint8_t *)&sleep_count);

     /* Wait for transmission complete, then sleep 3290p for 5 seconds */
 		while(!(UCSR0A & (1 << TXC0)));
//		uart_deinit();
        sleep_now(sleep_count+1);
//		uart_init();

    /* 1284p should be awake by now, update temperature and give it time to process */
		menu_send_temp();
	    _delay_us(20000);
 	}

    /* Wake LCD, turn on Raven logo */
    lcd_init();
    lcd_symbol_set(LCD_SYMBOL_RAVEN);
    sleep_wakeup();
    /* Wait for buttons up */
    while (key_state_get() != KEY_NO_KEY)
        ;
    if (is_button()){
        get_button();
    }
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will setup a ping request command to the 1284p and reset the ping counter.
 *
 *   \param val place holder
*/
void
menu_ping_request(uint8_t * val)
{
    uint8_t i;
    ping_mode = true;
    ping_count = 0;
    ping_response = 0;

    /* Initialize the numerical display with dashes */
    for(i=0; i<4; i++){
        lcd_single_print_dig(LCD_SEV_SEG_INDEX_MINUS, i);
    }

    menu_send_ping();

    /* Reset the timer for 1 sec resolution between pings. */
    TCNT1 = 0;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will send the ping request to the 1284p via the serial port.
 *
 *   \return ping_count The number of ping attempts.
*/
uint8_t
menu_send_ping(void)
{
    /*
     * Check for previous ping timeout. If menu_send_ping() was called before receiving
     * a response, update the LCD.
     */
    timeout_flag = true;
    ping_count++;
    /* Send the ping command with one byte payload of the current sequence number. */
    uart_serial_send_frame(SEND_PING, 1, (uint8_t *)&ping_count);
    return ping_count;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will stop the ping request.
*/
void
menu_stop_ping(void)
{
    ping_mode = false;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will enable or disable the JTAG debug interface to allow for
 *   proper temperature sensor readings.
 *
 *   \param val Flag to trigger the proper debug mode.
*/
void
menu_debug_mode(uint8_t *val)
{
    uint8_t sreg = SREG;
    cli();
    if(*val){
        /* Disable - Could use inline ASM to meet timing requirements. */
        MCUCR |= (1 << JTD);
        MCUCR |= (1 << JTD);
        /* Needed for timing critical JTD disable. */
        temp_init();
        /* Store setting in EEPROM. */
        eeprom_write_byte(EEPROM_DEBUG_ADDR, 0xFF);
    }
    else{
        /* Enable - Could use inline ASM to meet timing requirements. */
        MCUCR &= ~(1 << JTD);
        MCUCR &= ~(1 << JTD);
        /* Store setting in EEPROM. */
        eeprom_write_byte(EEPROM_DEBUG_ADDR, 0x01);
    }
    SREG = sreg;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will display the temperature in degrees F or C.
 *
 *   \param val Flag to trigger F or C temperature conversion.
*/
void
menu_read_temp(uint8_t *val)
{
    if(*val){
        temp_mode = TEMP_UNIT_CELCIUS;
    }
    else{
        temp_mode = TEMP_UNIT_FAHRENHEIT;
    }

    temp_flag = true;

    menu_display_temp();
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will display the temperature in degrees F or C.
*/
void
menu_display_temp(void)
{
    int16_t result = temp_get(temp_mode);

    /* Display the temp result on the lower 4 digit display with the proper symbol. */
    if(temp_mode == TEMP_UNIT_CELCIUS){
        lcd_symbol_clr(LCD_SYMBOL_F);
        lcd_symbol_set(LCD_SYMBOL_C);
    }
    else{
        lcd_symbol_clr(LCD_SYMBOL_C);
        lcd_symbol_set(LCD_SYMBOL_F);
    }

    /* Check for the DEBUG JTAG enable bit and display a CAUTION symbol to the user. */
    /* CAUTION represents false value. */
    if(MCUCR & 0x80){
        lcd_symbol_clr(LCD_SYMBOL_ATT);
    }
    else{
        lcd_symbol_set(LCD_SYMBOL_ATT);
    }

    lcd_num_putdec(result, LCD_NUM_PADDING_SPACE);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will clear the temperature displayed in the 4 digit LCD segments.
*/
void
menu_clear_temp(void)
{
    temp_flag = false;
    lcd_symbol_clr(LCD_SYMBOL_F);
    lcd_symbol_clr(LCD_SYMBOL_C);
    lcd_symbol_clr(LCD_SYMBOL_ATT);
    lcd_num_clr();
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will setup the current temperature for transfer to the ATmega1284p via a binary
 *   command transfer.
 *
 *   \param val This is used to determine sending once or auto based on the timer.
*/
void
menu_prepare_temp(uint8_t *val)
{
    if(*val){
        /* Only send the temp value once. */
        auto_temp = false;
    }
    else{
        /* Auto send the temp value based on TIMER1 interval. */
        auto_temp = true;
    }

    menu_send_temp();
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will stop the auto sending of temperature data.
*/
void
menu_stop_temp(void)
{
    auto_temp = false;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will send the data via the serial port.
*/
#if MEASURE_ADC2
extern uint16_t ADC2_reading;
#endif
void
menu_send_temp(void)
{
    int16_t result;
    uint8_t str[12];
    uint8_t * p = 0;

    /* Turn on nose LED for activity indicator */
    led_on();

    /* Get the latest temp value. */
    result = temp_get(temp_mode);

    /* Convert signed decimal number to ASCII. */
    p = signed_dectoascii(result, (str + 10));

    /* Send frame via serial port. */
    uart_serial_send_frame(SEND_TEMP, 1+strlen((char *)p), p);

#if MEASURE_ADC2
    /* Send ADC2 via serial port. */
    p = signed_dectoascii(ADC2_reading, (str + 10));
    str[9]='m';str[10]='V';str[11]=0;   //convert degrees to millivolts ;)
    uart_serial_send_frame(SEND_ADC2, 1+strlen((char *)p), p);
#endif

    led_off();
}

/** \}   */
