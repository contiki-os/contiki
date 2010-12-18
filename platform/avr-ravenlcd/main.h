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
 *      This is the main file for the Raven LCD application. Contains binary
 *      command definitions.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef __MAIN_H__
#define __MAIN_H__

/** \name Logical defines */
/** \{ */
#define BOOL    char
#define FALSE   0
#define TRUE    (!false)

#define PING_ATTEMPTS       (4)
/** \} */

/** \name These are GUI to Radio Binary commands. */
/** \{ */
#define NULL_CMD                      (0)
#define SEND_TEMP                     (0x80)
#define SEND_PING                     (0x81)
#define SEND_ADC2                     (0x82)
#define SEND_SLEEP                    (0x83)
#define SEND_WAKE                     (0x84)
/** \} */

/** \name These are the Radio to GUI binary commands. */
/** \{ */
#define REPORT_PING                   (0xC0)
#define REPORT_PING_BEEP              (0xC1)
#define REPORT_TEXT_MSG               (0xC2)
#define REPORT_WAKE                   (0xC3)
/** \} */


#endif /* __MAIN_H__ */
