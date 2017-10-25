/*
 * Copyright (c) 2017, Peter Sjodin, KTH Royal Institute of Technology
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
 *
 * Author  : Peter Sjodin psj@kth.se
 * Created : 2017-01-06
 */

#ifndef PMS5003_ARCH_H
#define PMS5003_ARCH_H

/**
 * \file
 *	Architecture-specific definitions for the Plantower PMS X003 dust sensors for avr-rss2
 */

/* AVR configuration for controlling dust sensor */
#define SET_PMS_DDR  DDRB   /* Data Direction Register: Port B */
#define SET_PMS_PORT PORTB  /* Serial Peripheral Interface */
#define PMS_SET      2      /* PD1: OW2_PIN, Chip Select */

/* Duty cycle mode -- STANDBY_MODE_OFF means device is active, etc. */
#define STANDBY_MODE_OFF	0
#define STANDBY_MODE_ON		1

extern void pms5003_set_standby_mode(uint8_t mode);
extern uint8_t pms5003_get_standby_mode(void);
extern uint8_t pms5003_i2c_probe(void);
#endif /* PMS5003_ARCH_H */
