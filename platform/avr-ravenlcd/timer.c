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
 *      This file will configure the timer during LCD operation.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include "lcd.h"
#include "timer.h"

/**
 *  \addtogroup lcd
 *  \{
*/

bool timer_flag;

/*---------------------------------------------------------------------------*/

/**
 *   \brief TIMER1 init for timer used by LCD.
 *
 *   The FPU of 8MHz with a prescaler of 1024 equals 7812.5 timer ticks creating a
 *   resolution of 128us per tick.
 *
 *   NOTE: The TIMER1 interrupt is not enabled in this routine
*/
void
timer_init(void)
{
    /*
     * Set TIMER1 output compare mode to clear on compare match for OC1A. Also
     * set the pre-scaler to 1024.
     */
    TCCR1B = (1 << WGM12)|(1 << CS12)|(1 << CS10);

    /* Set TIMER1 output compare register approx. 1 sec resolution. */
    OCR1A = _1_SEC;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Start the timer.
*/
void
timer_start(void)
{
    /* Clear TIMER1 timer counter value. */
    TCNT1 = 0;

    /* Enable TIMER1 output compare interrupt. */
    TIMSK1 = (1 << OCIE1A);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief Stop the timer.
*/
void
timer_stop(void)
{
    /* Disable TIMER1 output compare interrupt. */
    TIMSK1 &= ~(1 << OCIE1A);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This is the interrupt subroutine for the TIMER1 output match comparison.
*/
ISR
(TIMER1_COMPA_vect)
{
    /* Set the irq flag. */
    timer_flag = true;
}

/** \}   */
