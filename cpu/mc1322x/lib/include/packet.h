/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details. 
 *
 *
 */

#ifndef PACKET_H
#define PACKET_H

/* does not include 2 byte FCS checksum */
#ifndef MAX_PAYLOAD_SIZE
#define MAX_PAYLOAD_SIZE 125
#endif

#define PACKET_STATS 0

struct packet {
        uint8_t length; /* does not include FCS checksum */
	volatile struct packet * left;
	volatile struct packet * right;
        /* offset into data for first byte of the packet payload */
	/* On TX this should be 0 */
	/* On RX this should be 1 since the maca puts the length as the first byte*/
	uint8_t offset; 	
	uint8_t lqi;
	uint8_t status;
	uint32_t rx_time;
	#if PACKET_STATS
	uint8_t seen; 
	uint8_t post_tx;
	uint8_t get_free;
	uint8_t rxd;
	#endif
	uint8_t data[MAX_PAYLOAD_SIZE+2+1]; /* +2 for FCS; + 1 since maca returns the length as the first byte */
};
typedef struct packet packet_t;

#endif
