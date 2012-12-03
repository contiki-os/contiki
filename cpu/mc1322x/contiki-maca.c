/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
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
 * This file is part of the Contiki OS.
 *
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* debug */
#define DEBUG DEBUG_ANNOTATE
#include "net/uip-debug.h"

/* contiki */
#include "radio.h"
#include "sys/process.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

#include "contiki-conf.h"

/* mc1322x */
#include "mc1322x.h"
#include "config.h"

#ifndef CONTIKI_MACA_PREPEND_BYTE
#define CONTIKI_MACA_PREPEND_BYTE 0xff
#endif

#ifndef BLOCKING_TX
#define BLOCKING_TX 1
#endif

unsigned short node_id = 0;

static volatile uint8_t tx_complete;
static volatile uint8_t tx_status;

/* contiki mac driver */

int contiki_maca_init(void);
int contiki_maca_on_request(void);
int contiki_maca_off_request(void);
int contiki_maca_read(void *buf, unsigned short bufsize);
int contiki_maca_prepare(const void *payload, unsigned short payload_len);
int contiki_maca_transmit(unsigned short transmit_len);
int contiki_maca_send(const void *payload, unsigned short payload_len);
int contiki_maca_channel_clear(void);
int contiki_maca_receiving_packet(void);
int contiki_maca_pending_packet(void);

const struct radio_driver contiki_maca_driver =
{
	.init = contiki_maca_init,
	.prepare = contiki_maca_prepare,
	.transmit = contiki_maca_transmit,
	.send = contiki_maca_send,
	.read = contiki_maca_read,
	.receiving_packet = contiki_maca_receiving_packet,
	.pending_packet = contiki_maca_pending_packet,
	.channel_clear = contiki_maca_channel_clear,
	.on = contiki_maca_on_request,
	.off = contiki_maca_off_request,
};

static volatile uint8_t contiki_maca_request_on = 0;
static volatile uint8_t contiki_maca_request_off = 0;

static process_event_t event_data_ready;

static volatile packet_t prepped_p;

void contiki_maca_set_mac_address(uint64_t eui) {
	rimeaddr_t addr;
	uint8_t i;

	/* setup mac address registers in maca hardware */
	*MACA_MACPANID = 0xcdab; /* this is the hardcoded contiki pan, register is PACKET order */
	*MACA_MAC16ADDR = 0xffff; /* short addressing isn't used, set this to 0xffff for now */

	*MACA_MAC64HI = (uint32_t) (eui >> 32);
	*MACA_MAC64LO = (uint32_t)  eui;

	ANNOTATE("setting panid 0x%04x\n\r", *MACA_MACPANID);
	ANNOTATE("setting short mac 0x%04x\n\r", *MACA_MAC16ADDR);
	ANNOTATE("setting long mac 0x%08x_%08x\n\r", *MACA_MAC64HI, *MACA_MAC64LO);

	/* setup mac addresses in Contiki (RIME) */
	rimeaddr_copy(&addr, &rimeaddr_null);

	for(i=0; i < RIMEADDR_CONF_SIZE; i++) {
		addr.u8[RIMEADDR_CONF_SIZE - 1 - i] = (mc1322x_config.eui >> (i * 8)) & 0xff;
	}

	node_id = (addr.u8[6] << 8 | addr.u8[7]);
	rimeaddr_set_node_addr(&addr);

#if DEBUG_ANNOTATE
	ANNOTATE("Rime configured with address ");
	for(i = 0; i < sizeof(addr.u8) - 1; i++) {
		ANNOTATE("%02X:", addr.u8[i]);
	}
	ANNOTATE("%02X\n", addr.u8[i]);
#endif
}

int contiki_maca_init(void) {
//	trim_xtal();
//	vreg_init();
//	contiki_maca_init();
//	set_channel(0); /* channel 11 */
//	set_power(0x12); /* 0x12 is the highest, not documented */
	return 1;
}

/* CCA not implemented */
int contiki_maca_channel_clear(void) {
	return 1;
}

/* not sure how to check if a reception is in progress */
int contiki_maca_receiving_packet(void) {
	return 0;
}

int contiki_maca_pending_packet(void) {
	if (rx_head != NULL) {
		return 1;
	} else {
		return 0;
	}
}

int contiki_maca_on_request(void) {
	contiki_maca_request_on = 1;
	contiki_maca_request_off = 0;
	return 1;
}

int contiki_maca_off_request(void) {
	contiki_maca_request_on = 0;
	contiki_maca_request_off = 1;
	return 1;
}

/* it appears that the mc1332x radio cannot */
/* receive packets where the last three bits of the first byte */
/* is equal to 2 --- even in promiscuous mode */
int contiki_maca_read(void *buf, unsigned short bufsize) {
	volatile uint32_t i;
	volatile packet_t *p;

	if((p = rx_packet())) {
		PRINTF("maca read");
#if CONTIKI_MACA_RAW_MODE
		/* offset + 1 and size - 1 to strip the raw mode prepended byte */
		/* work around since maca can't receive acks bigger than five bytes */
		PRINTF(" raw mode");
		p->length -= 1;
		p->offset += 1;
#endif
		PRINTF(": p->length 0x%0x bufsize 0x%0x \n\r", p->length, bufsize);
		if((p->length) < bufsize) bufsize = (p->length);
		memcpy(buf, (uint8_t *)(p->data + p->offset), bufsize);
		packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY,p->lqi);
		packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP,p->rx_time);
#if CONTIKI_MACA_DEBUG
		for( i = p->offset ; i < (bufsize + p->offset) ; i++) {
			PRINTF(" %02x",p->data[i]);
		}
#endif
		PRINTF("\n\r");
		free_packet(p);
		return bufsize;
	} else {
		return 0;
	}
}

/* copies a payload into the prepped packet */
/* transmit sends the prepped packet everytime it is called */
/* Contiki may call prepare once and then transmit several times to send */
/* the same packet repeatedly */
int contiki_maca_prepare(const void *payload, unsigned short payload_len) {
	volatile int i;

	PRINTF("contiki maca prepare");
#if CONTIKI_MACA_RAW_MODE
	prepped_p.offset = 1;
	prepped_p.length = payload_len + 1;
#else
	prepped_p.offset = 0;
	prepped_p.length = payload_len;
#endif
	if(payload_len > MAX_PACKET_SIZE)  return RADIO_TX_ERR;
	memcpy((uint8_t *)(prepped_p.data + prepped_p.offset), payload, payload_len);
#if CONTIKI_MACA_RAW_MODE
	prepped_p.offset = 0;
	prepped_p.data[0] = CONTIKI_MACA_PREPEND_BYTE;
	PRINTF(" raw mode");
#endif
#if CONTIKI_MACA_DEBUG
	PRINTF(": sending %d bytes\n\r", payload_len);
	for(i = prepped_p.offset ; i < (prepped_p.length + prepped_p.offset); i++) {
		PRINTF(" %02x",prepped_p.data[i]);
	}
	PRINTF("\n\r");
#endif

	return RADIO_TX_OK;

}

/* gets a packet from the radio (if available), */
/* copies the prepared packet prepped_p */
/* and transmits it */
int contiki_maca_transmit(unsigned short transmit_len) {
	volatile packet_t *p;

	PRINTF("contiki maca transmit\n\r");
#if BLOCKING_TX
	tx_complete = 0;
#endif
	if(p = get_free_packet()) {
		p->offset = prepped_p.offset;
		p->length = prepped_p.length;
		memcpy((uint8_t *)(p->data + p->offset),
		       (const uint8_t *)(prepped_p.data + prepped_p.offset),
		       prepped_p.length);
		tx_packet(p);
	} else {
		PRINTF("couldn't get free packet for transmit\n\r");
		return RADIO_TX_ERR;
	}

#if BLOCKING_TX
	/* block until tx_complete, set by contiki_maca_tx_callback */
 	while(!tx_complete && (tx_head != 0));
#endif
}

int contiki_maca_send(const void *payload, unsigned short payload_len) {
	contiki_maca_prepare(payload, payload_len);
	contiki_maca_transmit(payload_len);
	switch(tx_status) {
	case SUCCESS:
	case CRC_FAILED: /* CRC_FAILED is usually an ack */
		PRINTF("TXOK\n\r");
		return RADIO_TX_OK;
	case NO_ACK:
		PRINTF("NOACK\n\r");
		return RADIO_TX_NOACK;
	default:
		PRINTF("TXERR\n\r");
		return RADIO_TX_ERR;
	}
}

PROCESS(contiki_maca_process, "maca process");
PROCESS_THREAD(contiki_maca_process, ev, data)
{
 	volatile uint32_t i;
	int len;

 	PROCESS_BEGIN();

	while (1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

		/* check if there is a request to turn the radio on or off */
		if(contiki_maca_request_on == 1) {
			contiki_maca_request_on = 0;
//			maca_on();
 		}

		if(contiki_maca_request_off == 1) {
			contiki_maca_request_off = 0;
//			maca_off();
 		}

		if (rx_head != NULL) {
			packetbuf_clear();
			len = contiki_maca_read(packetbuf_dataptr(), PACKETBUF_SIZE);
			if(len > 0) {
				packetbuf_set_datalen(len);
				NETSTACK_RDC.input();
			}
		}
                /* Call ourself again to handle remaining packets in the queue */
		if (rx_head != NULL) {
			process_poll(&contiki_maca_process);
		}

 	};

 	PROCESS_END();
}

void maca_rx_callback(volatile packet_t *p __attribute((unused))) {
	process_poll(&contiki_maca_process);
}


#if BLOCKING_TX
void maca_tx_callback(volatile packet_t *p __attribute((unused))) {
	tx_complete = 1;
	tx_status = p->status;
}
#endif
