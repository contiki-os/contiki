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
#include <util/delay.h>
#include <stdbool.h>
#include "main.h"
#include "sleep.h"
#include "uart.h"
#include "key.h"
#include "timer.h"
#include "lcd.h" //temp

/**
 * \addtogroup lcd
 * \{
*/

/*---------------------------------------------------------------------------*/

/**
 *   \brief Prepares for and executes sleep. This function sets up the
 *   processor to enter sleep mode, and to wake up when the joystick
 *   button (PE2/PCINT2) is pressed or after the specified interval.
 *
 *   \param howlong Seconds to sleep, 0=until button pushed
*/
void
sleep_now(int howlong)
{
    /* Disable watchdog (not currently used elsewhere) */
    wdt_disable();

    /* Setup sleep mode */
    if (howlong==0) {
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        MCUCR |= (1<<JTD);                      //Disable JTAG so clock can stop
    } else {
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
        /* Using 8 bit TIMER2 */
        TCNT2  = 0;
        TCCR2A = (1<<CS22)|(1<<CS21)|(1<<CS20); //Prescale by 1024
        TIMSK2 = (1<<TOIE2);                    //Enable overflow interrupt
        howlong*=30;                            //which is approximately 30 Hz

        /* Using 16 bit TIMER1, which takes a bit more power
        timer_stop;                             //Disable interrupt
        timer_init;                             //make sure initialized for 1 second
        timer_start;                            //Start timer, enable interrupt
        */
    }

    /* Enable pin change 0 wakeup interrupt */
    EIMSK |= (1 << PCIE0);
    /* Select joystick button input pin */
    PCMSK0 |= (1 << PCINT2);

    /* Sleep until timeout or button pushed */
    while (ENTER_PORT & (1<<ENTER_PIN)) {
        sleep_mode();
        if (!howlong--) break;
    }

    /* Disable the interrupts for the enter button and TIMER2 */
    EIMSK &= ~(1 << PCIE0);
    PCMSK0&= ~(1 <<PCINT2);
    TIMSK2&= ~(1 << TOIE2);
}

/*---------------------------------------------------------------------------*/

/**

 *   \brief This will send a wakeup command to ATmega1284p
 *   It may already be awake, if not it will respond during the next wake cycle
 *   Upon receiving the command it will return an acknowledgement frame
 *
 *   \brief This will send a single character forever to the ATmega1284p to cause a wakeup.
 *   The 1284p polls the USART for new data during each sleep cycle.  Upon receiving a
 *   character from the user LCD, it will wake up and send an acknowledgement frame.
*/
void
sleep_wakeup(void)
{
    lcd_puts_P(PSTR("WAKE 1284p"));

	/* Flood 1284p with wake commands until it responds*/
    for(;;){
        uart_serial_send_frame(SEND_WAKE,0,0);
	    _delay_us(1000);
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
/*---------------------------------------------------------------------------*/

/**
 *   \brief This is the timer2 overflow interrupt. When this interrupt fires,
 *   the CPU will wake.
*/
ISR
(TIMER2_OVF_vect)
{

}

/** \}   */
