/* -*- C -*- */
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
 * @(#)$Id: slip.h,v 1.5 2008/02/24 21:00:53 adamdunkels Exp $
 */

#ifndef __SLIP_H__
#define __SLIP_H__

#include "contiki.h"

PROCESS_NAME(slip_process);

/**
 * Send an IP packet from the uIP buffer with SLIP.
 */
uint8_t slip_send(void);

/**
 * Input a SLIP byte.
 *
 * This function is called by the RS232/SIO device driver to pass
 * incoming bytes to the SLIP driver. The function can be called from
 * an interrupt context.
 *
 * For systems using low-power CPU modes, the return value of the
 * function can be used to determine if the CPU should be woken up or
 * not. If the function returns non-zero, the CPU should be powered
 * up. If the function returns zero, the CPU can continue to be
 * powered down.
 *
 * \param c The data that is to be passed to the SLIP driver
 *
 * \return Non-zero if the CPU should be powered up, zero otherwise.
 */
int slip_input_byte(unsigned char c);

uint8_t slip_write(const void *ptr, int len);

/* Did we receive any bytes lately? */
extern uint8_t slip_active;

/* Statistics. */
extern uint16_t slip_rubbish, slip_twopackets, slip_overflow, slip_ip_drop;

/**
 * Set a function to be called when there is activity on the SLIP
 * interface; used for detecting if a node is a gateway node.
 */
void slip_set_input_callback(void (*callback)(void));

/*
 * These machine dependent functions and an interrupt service routine
 * must be provided externally (slip_arch.c).
 */
void slip_arch_init(unsigned long ubr);
void slip_arch_writeb(unsigned char c);

#endif /* __SLIP_H__ */
