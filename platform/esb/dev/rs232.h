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
 */

/** \addtogroup esb
 * @{ */

/**
 * \defgroup esbrs232 ESB RS232
 *
 * @{
 */

/**
 * \file
 * Header file for MSP430 RS232 driver.
 * \author Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __RS232_H__
#define __RS232_H__


#define RS232_19200 1
#define RS232_38400 2
#define RS232_57600 3
#define RS232_115200 4

/**
 * \brief      Initialize the RS232 module
 *
 *             This function is called from the boot up code to
 *             initalize the RS232 module.
 */
void rs232_init(void);

/**
 * \brief      Set an input handler for incoming RS232 data
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
void rs232_set_input(int (* f)(unsigned char));

/**
 * \brief      Configure the speed of the RS232 hardware
 * \param speed The speed
 *
 *             This function configures the speed of the RS232
 *             hardware. The allowed parameters are RS232_19200,
 *             RS232_38400, RS232_57600, and RS232_115200.
 */
void rs232_set_speed(unsigned char speed);

/**
 * \brief      Print a text string on RS232
 * \param str  A pointer to the string that is to be printed
 *
 *             This function prints a string to RS232. The string must
 *             be terminated by a null byte. The RS232 module must be
 *             correctly initalized and configured for this function
 *             to work.
 */
void rs232_print(char *str);

/**
 * \brief      Print a character on RS232
 * \param c    The character to be printed
 *
 *             This function prints a character to RS232. The RS232
 *             module must be correctly initalized and configured for
 *             this function to work.
 */
void rs232_send(char c);

#endif /* __RS232_H__ */

/** @} */
/** @} */
