/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author:   Adam Dunkels <adam@sics.se>
 *           Simon Barner <barner@in.tum.de>
 *
 */

#ifndef __RS232_H__
#define __RS232_H__

#include <avr/pgmspace.h>
#include "contiki-conf.h"

#if defined (__AVR_ATmega128__)
#include "dev/rs232_atmega128.h"
#elif defined (__AVR_ATmega1281__)
#include "dev/rs232_atmega1281.h"
#elif defined (__AVR_ATmega1284P__)
#include "dev/rs232_atmega1284.h"
#elif defined (__AVR_AT90USB1287__)
#include "dev/rs232_at90usb1287.h"
#elif defined (__AVR_ATmega128RFA1__)
#include "dev/rs232_atmega128rfa1.h"
#elif defined (__AVR_ATmega644__) || defined (__AVR_ATmega328P__)
#include "dev/rs232_atmega644.h"
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega8515__) \
   || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
#include "dev/rs232_atmega32.h"
#else
#error "Please implement a rs232 header for your MCU (or set the MCU type \
in contiki-conf.h)."
#endif

/******************************************************************************/
/***   Baud rates                                                             */
/******************************************************************************/
#define BAUD_RATE(x) (F_CPU/16/x-1)

/**
 * \brief      Initialize the RS232 module
 *
 *             This function is called from the boot up code to
 *             initalize the RS232 module.
 * \param port The RS232 port to be used.
 * \param bd   The baud rate of the connection.
 * \param ffmt The frame format of the connection, i.e. parity mode,
 *             number of stop and data bits, ...
 */
void
rs232_init (uint8_t port, uint8_t bd, uint8_t ffmt);

/**
 * \brief      Set an input handler for incoming RS232 data
 * \param port The RS232 port to be used.
 * \param f    A pointer to a byte input handler
 *
 *             This function sets the input handler for incoming RS232
 *             data. The input handler function is called for every
 *             incoming data byte. The function is called from the
 *             RS232 interrupt handler, so care must be taken when
 *             implementing the input handler to avoid race
 *             conditions.
 *
 *             The return value of the input handler affects the sleep
 *             mode of the CPU: if the input handler returns non-zero
 *             (true), the CPU is awakened to let other processing
 *             take place. If the input handler returns zero, the CPU
 *             is kept sleeping.
 */
void
rs232_set_input(uint8_t port, int (* f)(unsigned char));


/**
 * \brief      Print a text string from program memory on RS232
 * \param port The RS232 port to be used.
 * \param buf  A pointer to the string that is to be printed
 *
 *             This function prints a string from program memory to
 *             RS232. The string must be terminated by a null
 *             byte. The RS232 module must be correctly initalized and
 *             configured for this function to work.
 */
void
rs232_print(uint8_t port, char *buf);

/**
 * \brief      Print a formated string on RS232
 * \param port The RS232 port to be used.
 * \param fmt  The format string that is used to construct the string
 *             from a variable number of arguments.
 *
 *             This function prints a formated string to RS232. Note
 *             that this function used snprintf internally and thus cuts
 *             the resulting string after RS232_PRINTF_BUFFER_LENGTH - 1
 *             bytes. You can override this buffer lenght with the
 *             RS232_CONF_PRINTF_BUFFER_LENGTH define. The RS232 module
 *             must becorrectly initalized and configured for this
 *             function to work.
 */
void
rs232_printf(uint8_t port, const char *fmt, ...);

/**
 * \brief      Print a character on RS232
 * \param port The RS232 port to be used.
 * \param c    The character to be printed
 *
 *             This function prints a character to RS232. The RS232
 *             module must be correctly initalized and configured for
 *             this function to work.
 */
void
rs232_send(uint8_t port, unsigned char c);

/**
 * \brief      Redirects stdout to a given RS232 port
 * \param port The RS232 port to be used.
 *
 *             This function redirects the stdout channel to a given
 *             RS232 port. Note that this modfies the global variable
 *             stdout. If you want to restore the previous behaviour, it
 *             is your responsibility to backup to old value. The RS232
 *             module must be correctly initalized and configured for
 *             the redirection to work.
 */
void
rs232_redirect_stdout (uint8_t port);

#endif /* __RS232_H__ */
