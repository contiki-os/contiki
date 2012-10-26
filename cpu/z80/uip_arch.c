/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */
 /*
  * \file
  * 	Z80 architecture-depend uip module
  * 	for calculating checksums
  * \author
  * 	Takahide Matsutsuka <markn@markn.org>
  */

#include <stddef.h>
#include "uip_arch.h"

static const uint16_t sizeof_uip_ipaddr_t = sizeof(uip_ipaddr_t);
static const uint16_t offset_tcpip_hdr_len = offsetof(struct uip_tcpip_hdr, len);
static const uint16_t offset_tcpip_hdr_srcipaddr = offsetof(struct uip_tcpip_hdr, srcipaddr);

/*--------------------------------------------------------------------------*/
static void upper_layer_chksum() {
__asm
	;; ---------------------------------
	;; static uint16_t upper_layer_chksum(uint8_t proto);
	;; Stack; retl reth
	;; @param C proto
	;; ABCDEHL____
	;; ---------------------------------
	;; HL = BUF = &uip_buf[UIP_LLH_LEN]
	ld	hl, #_uip_buf
	ld	de, #UIP_LLH_LEN
	add	hl, de
	push	hl
	
	;; HL = BUF->len[0]
	push	ix
	ld	ix, #_offset_tcpip_hdr_len
	ld	e, 0(ix)
	ld	d, 1(ix)
	add	hl, de
	pop	ix
	
	;; DE = upper layer length
	ld	d, (hl)
	inc	hl
	ld	e, (hl)
#if UIP_CONF_IPV6
#else
	ld	a, e
	sub	a, #UIP_IPH_LEN
	ld	e, a
	jr	nc, _upper_layer_chksum_setlen2
	dec	d
_upper_layer_chksum_setlen2:
#endif
	;; bc = upper_leyer_len + proto
	ld	b, d
	ld	a, e
	add	a, c
	ld	c, a
	jr	nc, _upper_layer_chksum_setlen3
	inc	b
_upper_layer_chksum_setlen3:
	pop	hl		; BUF
	push	de
	push	ix
	ld	ix, #_offset_tcpip_hdr_srcipaddr
	ld	e, 0(ix)
	ld	d, 1(ix)
	add	hl, de
	ld	e, l
	ld	d, h
	ld	ix, #_sizeof_uip_ipaddr_t
	ld	l, 0(ix)
	ld	h, 1(ix)
	pop	ix
	sla	l
	rl	h
	push	hl
	push	de
	push	bc
	call	_uip_arch_chksum		; hl = sum
	pop	af
	pop	af
	pop	af
	;; de is still stacked

	ld	b, h
	ld	c, l
	ld	hl, #_uip_buf
	ld	de, #UIP_IPH_LEN	
	add	hl, de
_upper_layer_chksum_call:
	ld	de, #UIP_LLH_LEN
	add	hl, de
	push	hl
	push	bc
	call	_uip_arch_chksum
	pop	af
	pop	af
	pop	af

	ld	a, h
	or	a, l
	jr	nz, _upper_layer_uip_htons
	ld	hl, #0xffff
	jr	_upper_layer_ret
_upper_layer_uip_htons:
	ld	a, l
	ld	l, h
	ld	h, a
_upper_layer_ret:
__endasm;
}

/*--------------------------------------------------------------------------*/
uint16_t
uip_ipchksum(void)
{
__asm
	;; ---------------------------------
	;; uint16_t uip_ipchksum(void);
	;; Stack; retl reth
	;; ABCDEHL____
	;; return HL
	;; ---------------------------------
 	ld	hl, #UIP_IPH_LEN
 	push	hl
	;; HL = BUF = &uip_buf[UIP_LLH_LEN]
	ld	hl, #_uip_buf
	;; BC = sum = 0
	ld	bc, #0
	jp	_upper_layer_chksum_call
__endasm;
}

/*--------------------------------------------------------------------------*/
#if UIP_CONF_IPV6
uint16_t
uip_icmp6chksum(void)
{
__asm
	;; ---------------------------------
	;; uint16_t uip_icmp6chksum(void);
	;; Stack; retl reth
	;; ABCDEHL____
	;; return HL
	;; ---------------------------------
	ld	c, #UIP_PROTO_ICMP6
	jp	_upper_layer_chksum
__endasm;
}
#endif /* UIP_CONF_IPV6 */

/*--------------------------------------------------------------------------*/
uint16_t
uip_tcpchksum(void)
{
__asm
	;; ---------------------------------
	;; uint16_t uip_tcpchksum(void);
	;; Stack; retl reth
	;; ABCDEHL____
	;; return HL
	;; ---------------------------------
	ld	c, #UIP_PROTO_TCP
	jp	_upper_layer_chksum
__endasm;
}

/*--------------------------------------------------------------------------*/
#if UIP_UDP_CHKSUMS
uint16_t
uip_udpchksum(void)
{
__asm
	;; ---------------------------------
	;; uint16_t uip_udpchksum(void);
	;; Stack; retl reth
	;; ABCDEHL____
	;; return HL
	;; ---------------------------------
	ld	c, #UIP_PROTO_UDP
	jp	_upper_layer_chksum
__endasm;
}
#endif /* UIP_UDP_CHKSUMS */
/*--------------------------------------------------------------------------*/
