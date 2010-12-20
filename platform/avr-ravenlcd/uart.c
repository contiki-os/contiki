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
 *      Handles the control of the USART for communication with the ATmega1284p
 *      for sending commands.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include "uart.h"
#include "lcd.h"
#include "main.h"
#include "menu.h"
#include "beep.h"

/**
 *  \addtogroup lcd
 *  \{
*/

#define TIMEOUT (0xff)

/** \brief The RX circular buffer, for storing characters from serial port. */
tcirc_buf rxbuf;

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will intialize the circular buffer head and tail of tcirc_buf struct.
 *
 *   \param cbuf Pointer to buffer to initialize.
*/
void
uart_init_circ_buf(tcirc_buf *cbuf)
{
    cbuf->head = cbuf->tail = 0;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will add a new character to the circular buffer.
 *
 *   \param cbuf Pointer to buffer where character will be stored.
 *   \param ch Character to store into buffer.
*/
void
uart_add_to_circ_buf(tcirc_buf *cbuf, uint8_t ch)
{
    /* Add char to buffer */
    uint8_t newhead = cbuf->head;
    newhead++;
    if (newhead >= BUFSIZE){
        newhead = 0;
    }
    if (newhead == cbuf->tail){
        /* Buffer full, quit it */
        return;
    }

    cbuf->buf[cbuf->head] = ch;
    cbuf->head = newhead;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will get a character from the buffer requested.
 *
 *   \param cbuf Pointer to buffer to get character from.
 *
 *   \return retval Return character from buffer.
*/
uint8_t
uart_get_from_circ_buf(tcirc_buf *cbuf)
{
    /* Get char from buffer. */
    /* Be sure to check first that there is a char in buffer. */
    uint8_t newtail = cbuf->tail;
    uint8_t retval = cbuf->buf[newtail];

    newtail++;
    if (newtail >= BUFSIZE){
        /* Rollover */
        newtail = 0;
    }
    cbuf->tail = newtail;

    return retval;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will clear the RX buffer.
*/
void
uart_clear_rx_buf(void)
{
    rxbuf.tail = rxbuf.head = 0;
}

/**
 *   \brief This will check for a character in the requested buffer.
 *
 *   \param cbuf Pointer to buffer to check for any characters.
 *
 *   \return True if buffer empty.
*/
uint8_t
uart_circ_buf_has_char(tcirc_buf *cbuf)
{
    /* Return true if buffer empty */
    return (cbuf->head != cbuf->tail);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will convert a nibble to a hex value.
 *
 *   \param val Value to convert to hex.
 *
 *   \return val Converted hex value
*/
uint8_t
uip_ntohex(uint8_t val)
{
    /* Convert nibble to hex */
    if (val > 9){
        return val + 'A' - 10;
    }
    else{
        return val + '0';
    }
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Convert integer to hex value.
 *
 *   \param val Value to convert to hex.
 *   \param str Location to store converted value.
*/
void
itohex(uint8_t val,char *str)
{
    *str++ = uip_ntohex(val >> 8);
    *str = uip_ntohex(val & 0x0f);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will wait for a new character from the ATmega1284p and timeout
 *   if no new character is received.
 *
 *   \retval TIMEOUT Returns if timeout has occured.
 *   \return retval Character returned upon seeing rx_char_ready()
*/
uint8_t
uart_get_char_rx(void)
{
    /* Gets a serial char, and waits for timeout */
    uint32_t timex = 5000000;
    uint8_t retval;

    while (!rx_char_ready()){
        if (!timex--){
            /* Timeout, return timeout */
            return TIMEOUT;
        }
    }

    retval = uart_get_from_circ_buf(&rxbuf);
    return retval;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Initialize UART to 38400 Baud Rate and only
 *   enable UART for transmission.
*/
void
uart_init(void)
{
    /* For Mega3290P, enable the uart peripheral */
    PRR &= ~(1 << PRUSART0);

    uart_clear_rx_buf();
    /* 38400 baud @ 8 MHz internal RC oscillator (error = 0.2%) */
    UBRR0 = BAUD_RATE_38400;

    /* 8 bit character size, 1 stop bit and no parity mode */
    UCSR0C = ( 3 << UCSZ00);

    /* Enable RX,TX and RX interrupt on USART */
    UCSR0B = (1 << RXEN0)|(1 << TXEN0)|(1 << RXCIE0);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Turn off UART for sleep mode.
*/
void
uart_deinit(void)
{
    /* Disable RX,TX and RX interrupt on USART */
    UCSR0B = 0;

    /* for Mega3290P, disable the uart peripheral */
    PRR |= (1 << PRUSART0);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Send one byte over the uart. This is called to send binary commands.
 *
 *   \param byte The byte of data to send out the uart.
*/
void
uart_send_byte(uint8_t byte)
{
    /* Wait for last char to be gone... */
    while(!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = byte;

    /* Clear the TXC bit to allow transmit complete test before sleep*/
    UCSR0A |=(1 << TXC0);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This is the USART RX complete interrupt.
*/
ISR
(USART_RX_vect)
{
    /* Get byte from serial port, put in Rx Buffer. */
    uint8_t retval;

    retval = UDR0;
    uart_add_to_circ_buf(&rxbuf, retval);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This function builds and sends a binary command frame to the
 *   ATmega1284p.
 *
 *   \param cmd Command to send.
 *   \param payload_length Length of data to be sent with command.
 *   \param payload Pointer to data to send.
*/
void
uart_serial_send_frame(uint8_t cmd, uint8_t payload_length, uint8_t *payload)
{
    /* Send a frame to 1284p */
    int8_t i;

    uart_send_byte(SOF_CHAR);
    uart_send_byte(payload_length);
    uart_send_byte(cmd);
    for (i=0;i<=payload_length-1;i++){
   	    uart_send_byte(payload[i]);
   	}
    uart_send_byte(EOF_CHAR);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This displays a time out message to the user based on the parameter
 *   reason x.
 *
 *   \param x Reason for USART time out.
*/
void
uart_timeout_msg(uint8_t x)
{
    char str[20] = "TO     ";

    dectoascii(x, str+3);
    lcd_puts(str);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will receive a frame from the ATmega1284p and parse the incoming
 *   data.
 *
 *   If the incoming data is a binary command acknowledgement, then this will not
 *   parse any data. If the incoming data is test reports, the menu will store the
 *   data for end of test metrics.
 *
 *   \param wait_for_ack Flag used to wait for acknowledgement when receving a serial
 *   frame.
*/
void
uart_serial_rcv_frame(uint8_t wait_for_ack)
{
    /* Gets a serial frame, if any, and displays data appropriately */
    /* If wait_for_ack is true, this funtion will wait for serial chars. */
    volatile uint8_t ch;
    volatile uint8_t length;
    volatile uint8_t cmd;
    volatile uint8_t payload[20];
    uint16_t i;

    if (!wait_for_ack && !rx_char_ready()){
        return;
    }

    /* Check for SOF */
    ch = uart_get_char_rx();
    if (ch != SOF_CHAR){
        return uart_timeout_msg(1);
    }

    /* Turn on nose LED for activity indicator */
    led_on();

    /* Get length byte */
    ch = uart_get_char_rx();
    if (ch == TIMEOUT){
        return uart_timeout_msg(2);
    }
    /* Check for ACK Frame */
    if (ch >= 0x80){
        /* This is an ack frame, just get it and go away. */
        ch = uart_get_char_rx();
        if (ch != EOF_CHAR){
            uart_timeout_msg(3);
        }
        led_off();
        return;
    }

    length = ch;
    if (length > sizeof(payload)){
        /* invalid length */
        return;
    }

    /* Get cmd byte */
    ch = uart_get_char_rx();
    if (ch == TIMEOUT){
        return uart_timeout_msg(5);
    }
    cmd = ch;

    /* Get payload */
    for (i=0;i<length;i++){
        ch = uart_get_char_rx();
        if (ch == TIMEOUT){
            return uart_timeout_msg(i);
        }
        /* Save payload */
        payload[i] = ch;
    }

    /* Get EOF */
    ch = uart_get_char_rx();
    if (ch != EOF_CHAR){
        return uart_timeout_msg(7);
    }

    /* Process the received command */
    switch (cmd){
        case REPORT_PING:
            /*
             * This will update the lcd with the current ping status.
             * Store the sequence number away.
             */
            ping_response = payload[0];

            if(ping_response == 1){
                lcd_single_print_dig(ping_response, 3);
            }
            else if(ping_response == 2){
                lcd_single_print_dig(ping_response, 2);
            }
            else if(ping_response == 3){
                lcd_single_print_dig(ping_response, 1);
            }
            else if(ping_response == 4){
                lcd_single_print_dig(ping_response, 0);
            }

            timeout_flag = false;

            /* Beep on successful ping response. */
            lcd_symbol_set(LCD_SYMBOL_BELL);
            beep();
            lcd_symbol_clr(LCD_SYMBOL_BELL);
            break;
        case REPORT_TEXT_MSG:
            /* Copy text message to menu buffer and play ringtone */
            /* Prezero in case no string terminator in command */
            for (i=0;i<sizeof(top_menu_text);i++) top_menu_text[i]=0;
            memcpy(&top_menu_text,(char *)payload,sizeof(top_menu_text)-1);  //leave zero byte at end
            play_ringtone();
            break;
        case REPORT_PING_BEEP:
            lcd_symbol_set(LCD_SYMBOL_BELL);
            beep();
            lcd_symbol_clr(LCD_SYMBOL_BELL);
            break;
        case REPORT_WAKE:
            /* Indicates 1284 is awake*/
            break;
        default:
            break;
    }
    led_off();
}

/** \}   */
