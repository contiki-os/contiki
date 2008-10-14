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
 *      This file provides joystick operations. Combined with ADC functions.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include "key.h"
#include "uart.h"
#include "main.h"
#include "adc.h"

/**
 *  \addtogroup lcd
 *  \{
*/

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will intialize the joystick and the ADC for button readings.
*/
void
key_init(void)
{
    /* Disable digital input buffer for joystick signal */
    DIDR0 |= (1 << ADC1D);

    /* Enter is input w/pullup */
    ENTER_DDR &= ~(1<<ENTER_PIN);
    ENTER_PUR |= (1<<ENTER_PIN);

    /* Joystick is input wo/pullup (all though normal port function is overridden by ADC module when reading) */
    KEY_DDR &= ~(1<<KEY_PIN);
    KEY_PUR &= ~(1<<KEY_PIN);

    /* Get the ADC ready to use */
    adc_init(ADC_CHAN_ADC1, ADC_TRIG_FREE_RUN, ADC_REF_AVCC, ADC_PS_128);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will disable the ADC used for button readings.
*/
void
key_deinit(void)
{
    /* Turn off the ADC */
    adc_deinit();
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will poll run key_task() to determine if a button has been pressed.
 *
 *   \retval True if button is pressed
 *   \retval False if button is not pressed
*/
uint8_t
is_button(void)
{
    /* Return true if button has been pressed. */
    if (key_task() == KEY_NO_KEY){
        return false;
    }
    else{
        return true;
    }
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This function will wait for a user to press a button.
 *
 *   \return retval Returns the global button state.
*/
uint8_t
get_button(void)
{
    uint8_t retval;
    while (!is_button())
        ;

    retval = button;
    button = KEY_STATE_DONE;
    return retval;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will check the joystick state to return the current button status.
 *
 *   \return button Current button state.
*/
key_state_t
key_task(void)
{
    key_state_t key_state;

    /* Check joystick state. Post event if any change since last */
    key_state =  key_state_get();
    if (key_state == KEY_STATE_NO_KEY){
        if (button == KEY_STATE_DONE){
            button = KEY_STATE_NO_KEY;
        }
        return KEY_NO_KEY;
    }

    /* Key_state is button press code */
    if (button == KEY_STATE_DONE){
        /*
         * Button has already been used, don't return any more presses
         * until the button is released/re-pressed
         */
        return KEY_NO_KEY;
    }

    /* Button has been pressed for the first time */
    button = key_state;
    return button;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This function will start the ADC conversion and read the current
 *   converstion value to determine the button position.
 *
 *   \retval KEY_ENTER Enter button has been pressed.
 *   \retval KEY_UP Up Button has been pressed.
 *   \retval KEY_RIGHT Right Button has been pressed.
 *   \retval KEY_LEFT Left Button has been pressed.
 *   \retval KEY_DOWN Down Button has been pressed.
 *   \retval KEY_NO_KEY No Button has been pressed.
*/
key_state_t
key_state_get(void)
{
    int16_t reading;

    /* Start the A/D conversion */
    adc_conversion_start();

    /* Wait for conversion to finish */
    while ((reading = adc_result_get(ADC_ADJ_RIGHT)) == EOF )
        ;

    /* Determine which button (if any) is being pressed */
    if (!(ENTER_PORT & (1<<ENTER_PIN))){
        return KEY_ENTER;
    }
    if (reading < 0x00A0){
        return KEY_UP;
    }
    if (reading < 0x0180){
        return KEY_RIGHT;
    }
    if (reading < 0x0280){
        return KEY_LEFT;
    }
    if (reading < 0x0380){
        return KEY_DOWN;
    }
    return KEY_NO_KEY;
}

/** \}   */
