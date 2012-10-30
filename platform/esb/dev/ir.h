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
#ifndef __IR_H__
#define __IR_H__

#include "contiki.h"
#include "contiki-net.h"

/**
 * Initialize the IR driver.
 *
 * This function should be called from the main boot-up procedure in
 * order to initialize the IR device driver.
 *
 */
void ir_init(void);

void ir_send(unsigned short low12bits);

uint8_t ir_poll(void);
#define IR_NODATA 0
#define IR_DATA   1

uint16_t ir_data(void);

void ir_irq(void);

#define IR_STANDBY 0x0c
#define IR_VOLUME_UP 0x10
#define IR_VOLUME_DOWN 0x11
#define IR_VCR_PLAY 0x35
#define IR_VCR_STOP 0x36
#define IR_VCR_RECORD 0x37
#define IR_VCR_REWIND 0x32
#define IR_VCR_WIND 0x34

extern process_event_t ir_event_received;

PROCESS_NAME(ir_process);

#endif /* __IR_H__ */
