;
; Copyright (c) 2003-2007, Adam Dunkels, Josef Soucek and Oliver Schmidt
; All rights reserved. 
;
; Redistribution and use in source and binary forms, with or without 
; modification, are permitted provided that the following conditions 
; are met: 
; 1. Redistributions of source code must retain the above copyright 
;    notice, this list of conditions and the following disclaimer. 
; 2. Redistributions in binary form must reproduce the above copyright 
;    notice, this list of conditions and the following disclaimer in the 
;    documentation and/or other materials provided with the distribution. 
; 3. Neither the name of the Institute nor the names of its contributors 
;    may be used to endorse or promote products derived from this software 
;    without specific prior written permission. 
;
; THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
; ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
; OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
; HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
; LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
; OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
; SUCH DAMAGE. 
;
; This file is part of the Contiki operating system.
; 
; Author: Adam Dunkels <adam@sics.se>, Josef Soucek <josef.soucek@ide64.org>,
;         Oliver Schmidt <ol.sc@web.de>
;
;---------------------------------------------------------------------

	.segment	"JUMPTABLE"

	; Driver signature
	.byte	$65, $74, $68	; "eth"
	.byte	$01		; Ethernet driver API version number

	; Ethernet address
mac:	.byte	$00, $80, $0F	; OUI of Standard Microsystems
	.byte	$11, $11, $11

	; Buffer attributes
bufaddr:.res	2		; Address
bufsize:.res	2		; Size

	; Jump table.
	.addr	init
	.addr	poll
	.addr	send
	.addr	exit

;---------------------------------------------------------------------

	.zeropage

sp:	.res   	2		; Stack pointer (Do not trash !)
reg:	.res	2		; Address of register base
ptr:	.res	2		; Indirect addressing pointer
len:	.res	2		; Frame length

;---------------------------------------------------------------------

	.rodata

fixup:	.byte	fixup02-fixup01, fixup03-fixup02, fixup04-fixup03
	.byte	fixup05-fixup04, fixup06-fixup05, fixup07-fixup06
	.byte	fixup08-fixup07, fixup09-fixup08, fixup10-fixup09
	.byte	fixup11-fixup10, fixup12-fixup11, fixup13-fixup12
	.byte	fixup14-fixup13, fixup15-fixup14, fixup16-fixup15
	.byte	fixup17-fixup16, fixup18-fixup17, fixup19-fixup18
	.byte	fixup20-fixup19, fixup21-fixup20, fixup22-fixup21
	.byte	fixup23-fixup22, fixup24-fixup23, fixup25-fixup24
	.byte	fixup26-fixup25, fixup27-fixup26, fixup28-fixup27
	.byte	fixup29-fixup28, fixup30-fixup29, fixup31-fixup30
	.byte	fixup32-fixup31, fixup33-fixup32, fixup34-fixup33
	.byte	fixup35-fixup34, fixup36-fixup35, fixup37-fixup36
	.byte	fixup38-fixup37

fixups	= * - fixup

;---------------------------------------------------------------------

ethbsr		:= $FF0E	; Bank select register             R/W (2B)

; Register bank 0
ethtcr		:= $FF00	; Transmition control register     R/W (2B)
ethephsr	:= $FF02	; EPH status register              R/O (2B)
ethrcr		:= $FF04	; Receive control register         R/W (2B)
ethecr		:= $FF06	; Counter register                 R/O (2B)
ethmir		:= $FF08	; Memory information register      R/O (2B)
ethmcr		:= $FF0A	; Memory Config. reg.    +0 R/W +1 R/O (2B)

; Register bank 1
ethcr		:= $FF00	; Configuration register           R/W (2B)
ethbar		:= $FF02	; Base address register            R/W (2B)
ethiar		:= $FF04	; Individual address register      R/W (6B)
ethgpr		:= $FF0A	; General address register         R/W (2B)
ethctr		:= $FF0C	; Control register                 R/W (2B)

; Register bank 2
ethmmucr	:= $FF00	; MMU command register             W/O (1B)
ethautotx	:= $FF01	; AUTO TX start register           R/W (1B)
ethpnr		:= $FF02	; Packet number register           R/W (1B)
etharr		:= $FF03	; Allocation result register       R/O (1B)
ethfifo		:= $FF04	; FIFO ports register              R/O (2B)
ethptr		:= $FF06	; Pointer register                 R/W (2B)
ethdata		:= $FF08	; Data register                    R/W (4B)
ethist		:= $FF0C	; Interrupt status register        R/O (1B)
ethack		:= $FF0C	; Interrupt acknowledge register   W/O (1B)
ethmsk		:= $FF0D	; Interrupt mask register          R/W (1B)

; Register bank 3
ethmt		:= $FF00	; Multicast table                  R/W (8B)
ethmgmt		:= $FF08	; Management interface             R/W (2B)
ethrev		:= $FF0A	; Revision register                R/W (2B)
ethercv		:= $FF0C	; Early RCV register               R/W (2B)

	.data

;---------------------------------------------------------------------

init:
	; Save address of register base
	sta reg
	stx reg+1

	; Start with first fixup location
	lda #<(fixup01+1)
	ldx #>(fixup01+1)
	sta ptr
	stx ptr+1
	ldx #$FF
	ldy #$00

	; Fixup address at location
:	lda reg
	ora (ptr),y
	sta (ptr),y
	iny
	lda reg+1
	sta (ptr),y
	dey

	; Advance to next fixup location
	inx
	cpx #fixups
	bcs :+
	lda ptr
	clc
	adc fixup,x
	sta ptr
	bcc :-
	inc ptr+1
	bcs :-			; Always

	; Reset ETH card
:	lda #$00		; Bank 0
fixup01:sta ethbsr

	lda #%10000000		; Software reset
fixup02:sta ethrcr+1

	ldy #$00
fixup03:sty ethrcr
fixup04:sty ethrcr+1

	; Delay
:	cmp ($FF,x)		; 6 cycles
	cmp ($FF,x)		; 6 cycles
	iny			; 2 cycles
	bne :-			; 3 cycles
				; 17 * 256 = 4352 -> 4,4 ms

	; Enable transmit and receive
	lda #%10000001		; Enable transmit TXENA, PAD_EN
	ldx #%00000011		; Enable receive, strip CRC ???
fixup05:sta ethtcr
fixup06:stx ethrcr+1

	lda #$01		; Bank 1
fixup07:sta ethbsr

fixup08:lda ethcr+1
	ora #%00010000		; No wait (IOCHRDY)
fixup09:sta ethcr+1

	lda #%00001001		; Auto release
fixup10:sta ethctr+1

	; Set MAC address
	ldy #$00
:	lda mac,y
fixup11:sta ethiar,y
	iny
	cpy #$06
	bcc :-

	; Set interrupt mask
	lda #$02		; Bank 2
fixup12:sta ethbsr

	lda #%00000000		; No interrupts
fixup13:sta ethmsk
	rts

;---------------------------------------------------------------------

poll:
fixup14:lda ethist
	and #%00000001		; RCV INT
	bne :+

	; No packet available
	tax
	rts

	; Process the incoming packet
	; ---------------------------
	
:	lda #$00
	ldx #%11100000		; RCV, AUTO INCR., READ
fixup15:sta ethptr
fixup16:stx ethptr+1

	; Last word contains 'last data byte' and $60 or 'fill byte' and $40
fixup17:lda ethdata		; Status word
fixup18:lda ethdata		; Need high byte only

	; Move ODDFRM bit into carry:
	; - Even packet length -> carry clear -> subtract 6 bytes
	; - Odd packet length  -> carry set   -> subtract 5 bytes
	lsr
	lsr
	lsr
	lsr
	lsr

	; The packet contains 3 extra words
fixup19:lda ethdata		; Total number of bytes
	sbc #$05		; Actually 5 or 6 depending on carry
	sta len
fixup20:lda ethdata
	sbc #$00
	sta len+1

	; Is bufsize < len ?
	lda bufsize
	cmp len
	lda bufsize+1
	sbc len+1
	bcs :+

	; Yes, skip packet
	; Remove and release RX packet from the FIFO
	lda #%10000000
fixup21:sta ethmmucr

	; No packet available
	lda #$00
	tax
	rts

	; Read bytes into buffer
:	jsr adjustptr
:
fixup22:lda ethdata
	sta (ptr),y
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-

	; Remove and release RX packet from the FIFO
	lda #%10000000
fixup23:sta ethmmucr

	; Return packet length
	lda len
	ldx len+1
	rts

;---------------------------------------------------------------------

send:
	; Save packet length
	sta len
	stx len+1

	; Allocate memory for TX
	txa
	ora #%00100000
fixup24:sta ethmmucr

	; 8 retries
	ldy #$08

	; Wait for allocation ready
:
fixup25:lda ethist
	and #%00001000		; ALLOC INT
	bne :+

	; Shouldn't we do something here to actively free memory,
	; maybe removing and releasing an RX packet from the FIFO ???

	; And try again
	dey
	bne :-
	rts

	; Acknowledge interrupt, is it necessary ???
:	lda #%00001000
fixup26:sta ethack

	; Set packet address
fixup27:lda etharr
fixup28:sta ethpnr

	lda #$00
	ldx #%01000000		; AUTO INCR.
fixup29:sta ethptr
fixup30:stx ethptr+1

	; Status written by CSMA
	lda #$00
fixup31:sta ethdata
fixup32:sta ethdata

	; Check packet length parity:
	; - Even packet length -> carry set   -> add 6 bytes
	; - Odd packet length  -> carry clear -> add 5 bytes
	lda len
	eor #$01
	lsr
	
	; The packet contains 3 extra words
	lda len
	adc #$05		; Actually 5 or 6 depending on carry
fixup33:sta ethdata
	lda len+1
	adc #$00
fixup34:sta ethdata

	; Send the packet
	; ---------------

	; Write bytes from buffer
	jsr adjustptr
:	lda (ptr),y
fixup35:sta ethdata
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-

	; Odd packet length ?
	lda len
	lsr
	bcc :+

	; Yes
	lda #%00100000		; ODD
	bne :++			; Always

	; No
:	lda #$00
fixup36:sta ethdata		; Fill byte
:	
fixup37:sta ethdata		; Control byte

	; Add packet to FIFO
	lda #%11000000		; ENQUEUE PACKET - transmit packet
fixup38:sta ethmmucr
	rts

;---------------------------------------------------------------------

exit:
	rts

;---------------------------------------------------------------------

adjustptr:
	lda len
	ldx len+1
	eor #$FF		; Two's complement part 1
	tay
	iny			; Two's complement part 2
	sty reg
	sec
	lda bufaddr
	sbc reg
	sta ptr
	lda bufaddr+1
	sbc #$00
	sta ptr+1
	rts

;---------------------------------------------------------------------
