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
 *      This controls the sleep operation for the LCD.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include "sleep.h"
#include "uart.h"

/**
 * \addtogroup lcd
 * \{
*/

/*---------------------------------------------------------------------------*/

/**
 *   \brief Prepares for and executes sleep. This function sets up the
 *   processor to enter sleep mode, and to wake up when the joystick
 *   button (PE2/PCINT2) is pressed.
*/
void
sleep_now(void)
{
    /* Disable watchdog */
    wdt_disable();

    /* Setup sleep mode */
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    /* Enable wakeup interrupt */
    EIMSK |= (1 << PCIE0);
    /* Enable PCINT2 as interrupt */
    PCMSK0 |= (1 << PCINT2);

    /* Go to sleep now */
    sleep_mode();

    /* Disable the interrupt for the enter button */
    EIMSK &= ~(1 << PCIE0);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will send a single character forever to the ATmega1284p to cause a wakeup.
 *
 *   The 1284p polls the USART for new data during each sleep cycle.  Upon receiving a
 *   character from the user LCD, it will wake up and send an acknowledgement frame.
*/
void
sleep_wakeup(void)
{
    /* First, clear the input buffer and get any chars waiting */
    while(rx_char_ready()){
        uart_get_char_rx();
    }

    /* Flood 1284p with serial chars until it responds. */
    for(;;){
        uart_send_byte('x');
        if (rx_char_ready())
            break;
    }
    /* Get a frame back */
    uart_serial_rcv_frame(true);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This is the wake up button interrupt. When this interrupt fires,
 *   nothing is done. The program will simply continue from the end of the sleep
 *   command.
*/
ISR
(PCINT0_vect)
{

}

/** \}   */
