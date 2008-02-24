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
 * @(#)$Id: cc2420.h,v 1.1 2008/02/24 22:29:08 adamdunkels Exp $
 */

#ifndef CC2420_H
#define CC2420_H

#define MAX_PACKET_LEN      127
#define ACK_PACKET_LEN        5

/*  fc0 */
#define FC0_TYPE_MASK       0x07 /* bit0-2 */
#define FC0_TYPE_BEACON     0x00 /* bit0-2 */
#define FC0_TYPE_DATA       0x01 /* bit0-2 */
#define FC0_TYPE_ACK        0x02 /* bit0-2 */
#define FC0_TYPE_MAC_CMD    0x03 /* bit0-2 */
/*      reserved            0x04-0x07 */

#define FC0_SECURE          0x08 /* bit3 */
#define FC0_PENDING         0x10 /* bit4 */
#define FC0_REQ_ACK         0x20 /* bit5 */
#define FC0_INTRA_PAN       0x40 /* bit6 */
/*      reserved            0x80    bit7 */

/* fc1 */
/*      reserved            0x80    bit8-9 */
#define FC1_DST_MASK        0x0c /* bit10-11 */
#define FC1_DST_0           0x00 /* bit10-11 */
#define FC1_DST_16          0x08 /* bit10-11 */
#define FC1_DST_64          0x0c /* bit10-11 */

/*      reserved            0x80    bit12-13 */
#define FC1_SRC_MASK        0xc0 /* bit14-15 */
#define FC1_SRC_0           0x00 /* bit14-15 */
#define FC1_SRC_16          0x80 /* bit14-15 */
#define FC1_SRC_64          0xc0 /* bit14-15 */

/* footer[0] == RSSI, footer[1] == ... */
#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

/* The hdr_802_15::len byte does not count!!! */
#define MAC_HDR_LEN   (2 + 1 + 2 + 2 + 2)

struct hdr_802_15 {
  u8_t len;			/* Not part of MAC header!!! */
  u8_t fc0;
  u8_t fc1;
  u8_t seq;
  u16_t dst_pan;		/* also u8_t ack_footer[2] */
  u16_t dst;
  /* optional src_pan */
  u16_t src;
  /* u8_t payload[uip_len] */
  /* u8_t footer[2] */
  /* no more */
};

struct hdr_802_15_ack {
  u8_t len;
  u8_t fc0;
  u8_t fc1;
  u8_t seq;
  u8_t footer[2];
  /* no more */
};

struct cc2420_neigbour {
  unsigned mac:16, nretrans:4, expire:4; /* expiration time */
#if 0
  unsigned rssi:6, correlation:6;
#endif
};

#define NNEIGBOURS 16
extern struct cc2420_neigbour neigbours[NNEIGBOURS];

PROCESS_NAME(cc2420_process);

extern struct uip_fw_netif cc2420if;

extern signed char cc2420_last_rssi;
extern u8_t cc2420_last_correlation;
extern u8_t cc2420_is_input;
extern volatile u8_t cc2420_ack_received;

void cc2420_init(void);
void cc2420_set_chan_pan_addr(unsigned channel, unsigned pan,
			      unsigned addr, const u8_t *ieee_addr);
void cc2420_on(void);
void cc2420_off(void);

u8_t cc2420_send_ip(void);
u8_t cc2420_send_uaodv(void);
int  cc2420_send(struct hdr_802_15 *hdr, u8_t hdr_len,
		 const u8_t *p, u8_t p_len);

/* Called at poll priority. */
void cc2420_input(const struct hdr_802_15 *hdr, u8_t hdr_len,
		  const u8_t *payload, u8_t payload_len);

/* Is mac far away? remote=1, local=0, don't know=-1. */
enum { REMOTE_MAYBE = -1, REMOTE_NO = 0, REMOTE_YES = 1 };
int cc2420_check_remote(u16_t mac);
void cc2420_recv_ok(uip_ipaddr_t *from);

/*
 * Machine dependent initialization function and an interrupt service
 * routine must be provided externally. Call cc2420_intr from the
 * interrupt service routine.
 */
void __cc2420_arch_init(void);
int  __cc2420_intr(void);

#endif /* CC2420_H */
