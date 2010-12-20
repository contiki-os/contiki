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
 *      This file implements a beep function to emit a beep sound from Raven's
 *      speaker. Also polyphonic ringtones.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *      David Kopf dak664@embarqmail.com
 *
 */


#include "beep.h"
#include "key.h"
#include <stdlib.h>
#include "util/delay.h"

/**
 *  \addtogroup lcd
 *  \{
*/
/*---------------------------------------------------------------------------*/

/**
 *  \brief Emits a beep from the Raven's buzzer.  This routine simply toggles a port pin
 *  at an audio frequency, which causes a tone to be emitted from the Raven's speaker.
 *  The beep consists of two tones at two different frequencies.  At the end, the beeper
 *  port pin is kept low, which causes the speaker amplifier to shut down.
*/
void
beep(void)
{
    uint8_t i;
    volatile uint8_t j;

    /* Turn on the power to the speaker. */
    ENTER_DDR |= (1 << PE7);
    ENTER_PUR |= (1 << PE7);

    /* Setup port pin */
    BEEP_DDR |= (1 << BEEP_BIT);

    for (i=0;i<100;i++){
        /* Toggle port pin */
        BEEP_PIN |= (1 << BEEP_BIT);
        /* Delay loop */
        for (j=0;j<0xff;j++)
            ;
    }

    for (i=0;i<100;i++){
        /* Toggle port pin */
        BEEP_PIN |= (1 << BEEP_BIT);
        /* Delay loop */
        for (j=0;j<0xa0;j++)
            ;
    }

    /* Turn off speaker */
    BEEP_PORT &= ~(1 << BEEP_BIT);
    ENTER_PUR &= ~(1 << PE7);
}

/*---------------------------------------------------------------------------*/
/**
 *  \brief Play polyphonic tune using buzzer.  An 8 bit timer generates interrupts
 *  that flip the BEEP pin at the selected frequencies.
 *  On exit the beeper port pin is set low to shut down the speaker amplifier.
*/
/* Base interrupts per second, and interrupts for each note */
//#define BASE 31250
#define BASE 31250/2  //up one octave
#define NONE 0
#define C4   BASE/262
#define CS4  BASE/277
#define D4   BASE/294
#define DS4  BASE/311
#define E4   BASE/330
#define F4   BASE/349
#define FS4  BASE/370
#define G4   BASE/392
#define GS4  BASE/415
#define A4   BASE/440
#define AS4  BASE/466
#define B4   BASE/494
#define C5   BASE/523
#define CS5  BASE/554
#define D5   BASE/587
#define DS5  BASE/622
#define E5   BASE/659
#define F5   BASE/698
#define FS5  BASE/740
#define G5   BASE/784
#define GS5  BASE/831
#define A5   BASE/880
#define AS5  BASE/932

/* Tone reference length and internote gap, milliseconds */
#define TONE_LENGTH 60
#define TONE_GAP    20


static uint8_t tuneindex=0;
static uint8_t pictures[] PROGMEM = {G4,4, F4,4, AS4,4, C5,2, F5,2, D5,4, C5,2, F5,2, D5,4, AS4,4, C5,4, G4,4, F4,4, 0xff};
static uint8_t     axel[] PROGMEM = {FS4,2, NONE,2, A4,3, FS4,2, FS4,1, B4,2, FS4,2, E4,2, FS4,2, NONE,2, CS5,3, FS4,2, FS4,1, D5,2, CS5,2, A4,2, FS4,2, CS5,2, FS5,2, FS4,1, E4,2, E4,1, CS4,2, GS4,2, FS4,6, 0xff};
static uint8_t sandman1[] PROGMEM = {F4,2, G4,2, B4,4, A4,10, B4,2, B4,2, A4,2, B4,12, 0xff};
static uint8_t sandman2[] PROGMEM = {C4,2, E4,2, G4,2, B4,2, A4,2, G4,2, E4,2, C4,2, D4,2, F4,2, A4,2, C5,2, B4,8, 0xff};
static uint8_t furelise[] PROGMEM = {E5,1, DS5,1, E5,1, DS5,1, E5,1, B4,1, D5,1, E5,1, A4,2, NONE,1, C4,1, E4,1, A4,1, B4,2, NONE,1, E4,1, GS4,1, B4,1, C5,2, 0xff};

static volatile uint8_t icnt,tone;

#include <avr/interrupt.h>
ISR(TIMER0_OVF_vect)
{
    if (tone == NONE) icnt = 0;
    else if (icnt++ >= tone)
    {
        BEEP_PIN |= (1 << BEEP_BIT);
        icnt = 0;
    }
}

void play_ringtone(void)
{
uint8_t i,*noteptr;

    /* What's next on the playlist? */
    switch (tuneindex++) {
    case 1 :beep();return;
    case 2 :noteptr=sandman1;break;
    case 3 :noteptr=furelise;break;
    case 4 :noteptr=sandman2;break;
    case 5 :noteptr=axel;break;
    default:noteptr=pictures;tuneindex=1;break;
    }
    
    /* Turn on the power to the speaker. */
    ENTER_DDR |= (1 << PE7);
    ENTER_PUR |= (1 << PE7);

    /* Setup port pin */
    BEEP_DDR |= (1 << BEEP_BIT);

    /* Start with no tone */
    icnt = 0;
    tone = NONE;
    
    /* Clock 8 bit timer at maximum frequency (CS0=1), interrupt overflow */
    /* 8MHz / 256 = 31250 interrupts per second */
    TCCR0A |= _BV(CS00);
    TCNT0 = 0;
    TIMSK0 |= _BV(TOIE0);

    /* Play all the notes */
    for (;;) {
        tone=pgm_read_byte(noteptr++);
        if (tone==0xff) break;
        for (i = pgm_read_byte(noteptr++);i > 0; i--) _delay_us(1000UL*TONE_LENGTH);
        tone = NONE;_delay_us(1000UL*TONE_GAP);
    }
    
    /* Turn off interrupts and speaker */
    TIMSK0 &= ~_BV(TOIE0);
    BEEP_PORT &= ~(1 << BEEP_BIT);
    ENTER_PUR &= ~(1 << PE7);
}
/** \}   */
