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

#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>



/** \name Port definition for reading the joystick input. */
/** \{ */
#define KEY_PORT      PINF
#define KEY_PUR       PORTF
#define KEY_DDR       DDRF
#define KEY_PIN       PINF1
/** \} */

/** \name Port defininition for the joystick Enter button. */
/** \{ */
#define ENTER_PORT      PINE
#define ENTER_PUR       PORTE
#define ENTER_DDR       DDRE
#define ENTER_PIN       PINE2
/** \} */

/** \name Valid Key States. */
/** \{ */
#define KEY_STATE_UP        0x01
#define KEY_STATE_DOWN      0x02
#define KEY_STATE_LEFT      0x04
#define KEY_STATE_RIGHT     0x08
#define KEY_STATE_ENTER     0x10
#define KEY_STATE_NO_KEY    0x00
#define KEY_STATE_DONE      0x20
/** \} */

typedef enum {
    KEY_UP      = 0x01,
    KEY_DOWN    = 0x02,
    KEY_LEFT    = 0x04,
    KEY_RIGHT   = 0x08,
    KEY_ENTER   = 0x10,
    KEY_NO_KEY  = 0x00
}key_state_t;

extern key_state_t button;

/* Initialize the joystick */
void key_init(void);
void key_deinit(void);
key_state_t key_task(void);
key_state_t key_state_get(void);
uint8_t is_button(void);
uint8_t get_button(void);

#endif /* __KEY_H__ */
