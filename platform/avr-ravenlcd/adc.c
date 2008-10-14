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
 *      Functions to control the ADC of the MCU. This is used to read the
 *      joystick.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include "adc.h"

/**
 *  \addtogroup lcd
 *  \{
*/

static bool adc_initialized;
static bool adc_conversion_started;

/*---------------------------------------------------------------------------*/

/**
 *   \brief This function will init the ADC with the following parameters.
 *
 *   \param chan Determines the ADC channel to open.
 *   \param trig Sets what type of trigger is needed.
 *   \param ref Sets the proper reference voltage.
 *   \param prescale Sets the prescale to be used against the XTAL choice.
 *
 *   \return 0
*/
int
adc_init(adc_chan_t chan, adc_trig_t trig, adc_ref_t ref, adc_ps_t prescale)
{
    /* Enable ADC module */
    PRR &= ~(1 << PRADC);

    /* Configure */
    ADCSRA = (1<<ADEN)|prescale;
    ADMUX = (uint8_t)ref|(uint8_t)chan;
    ADCSRB = trig;

    adc_initialized = true;
    adc_conversion_started = false;

    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will disable the adc.
*/
void
adc_deinit(void)
{
    /* Disable ADC */
    ADCSRA &= ~(1<<ADEN);
    PRR |= (1 << PRADC);

    adc_initialized = false;
    adc_conversion_started = false;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will start an ADC conversion
 *
 *   \return 0
*/
int
adc_conversion_start(void)
{
    if (adc_initialized == false){
        return EOF;
    }
    adc_conversion_started = true;
    ADCSRA |= (1<<ADSC);
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will read the ADC result during the ADC conversion and return
 *   the raw ADC conversion result.
 *
 *   \param adjust This will Left or Right Adjust the ADC conversion result.
 *
 *   \return ADC raw 16-byte ADC conversion result.
*/
int16_t
adc_result_get(adc_adj_t adjust)
{
    if (adc_conversion_started == false){
        return EOF;
    }
    if (ADCSRA & (1<<ADSC)){
        return EOF;
    }
    adc_conversion_started = false;
    ADMUX |= (adjust<<ADLAR);
    return (int16_t)ADC;
}

/** \}   */
