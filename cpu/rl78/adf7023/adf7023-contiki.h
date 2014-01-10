/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Ian Martin <martini@redwirellc.com>
 */

#ifndef __ADF7023_CONTIKI_H__
#define __ADF7023_CONTIKI_H__

int adf7023_init(void);

/* Prepare the radio with a packet to be sent. */
int adf7023_prepare(const void *payload, unsigned short payload_len);

/* Send the packet that has previously been prepared. */
int adf7023_transmit(unsigned short transmit_len);

/* Prepare & transmit a packet. */
int adf7023_send(const void *payload, unsigned short payload_len);

/* Prepare & transmit a packet. */
int adf7023_read(void *buf, unsigned short buf_len);

/* Perform a Clear-Channel Assessment (CCA) to find out if there is a packet in the air or not. */
int adf7023_channel_clear(void);

/* Check if the radio driver is currently receiving a packet. */
int adf7023_receiving_packet(void);

/* Check if the radio driver has just received a packet. */
int adf7023_pending_packet(void);

/* Turn the radio on. */
int adf7023_on(void);

/* Turn the radio off. */
int adf7023_off(void);

#endif /* __ADF7023_CONTIKI_H__ */
