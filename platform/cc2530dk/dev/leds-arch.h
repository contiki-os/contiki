/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *         Header file for platform-specific led functionality
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */


#ifndef LEDS_ARCH_H_
#define LEDS_ARCH_H_

#include "dev/port.h"

/* Led 4 on the SmartRF05EB is multiplexed with Button 1 on P0_1 */
#define LED4_READ()           PORT_READ(LED4_PORT, LED4_PIN)
#define LED4_WRITE(v)         PORT_WRITE(LED4_PORT, LED4_PIN, v)
#define LED4_FUNC_GPIO()      PORT_FUNC_GPIO(LED4_PORT, LED4_PIN)
#define LED4_DIR_INPUT()      PORT_DIR_INPUT(LED4_PORT, LED4_PIN)
#define LED4_DIR_OUTPUT()     PORT_DIR_OUTPUT(LED4_PORT, LED4_PIN)

#endif /* LEDS_ARCH_H_ */
