/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */
/**
 * \file  platform-conf.h
 * \brief Platform configuration file for the SEEDEYE port.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-06-06
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#ifndef SEEDEYE_ID
#define SEEDEYE_ID 1
#endif /* SEEDEYE_ID */

#if SEEDEYE_ID == 1
#define MRF24J40_PAN_COORDINATOR
#endif /* SEEDEYE_ID == 1 */

#define UART_DEBUG_BAUDRATE                     115200
#define UART_SLIP_BAUDRATE                      115200

#define PLATFORM_HAS_BATTERY                    1
#define PLATFORM_HAS_BUTTON                     1
#define PLATFORM_HAS_LEDS                       1
#define PLATFORM_HAS_RADIO                      1

#define CLOCK_CONF_SECOND                       1024

#endif /* __PLATFORM_CONF_H__ */
