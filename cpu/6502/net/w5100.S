;
; Copyright (c) 2013, Oliver Schmidt
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
; Author: Oliver Schmidt <ol.sc@web.de>
;
;---------------------------------------------------------------------

	.macpack	module
	module_header	_w5100

	; Driver signature
	.byte	$65, $74, $68	; "eth"
	.byte	$01		; Ethernet driver API version number

	; Ethernet address
mac:	.byte	$00, $08, $DC	; OUI of WIZnet
	.byte	$11, $11, $11

	; Buffer attributes
bufaddr:.res	2		; Address
bufsize:.res	2		; Size

	; Jump table.
	jmp init
	jmp poll
	jmp send
	jmp exit

;---------------------------------------------------------------------

	.if DYN_DRV

	.zeropage
sp:	.res	2		; Stack pointer (Do not trash !)
reg:	.res	2		; Pointer Register content
ptr:	.res	2		; Indirect addressing pointer
len:	.res	2		; Data length
cnt:	.res	2		; Data length counter
adv:	.res	2		; Data pointer advancement
dir:	.res	1		; Transfer direction
bas:	.res	1		; Socket 0 Base Address (hibyte)
lim:	.res	1		; Socket 0 memory limit (hibyte)
tmp:	.res	1		; Temporary value

	.else

	.include "zeropage.inc"
reg	:=	ptr1		; Pointer Register content
ptr	:=	ptr2		; Indirect addressing pointer
len	:=	ptr3		; Data length
cnt	:=	ptr4		; Data length counter
adv	:=	sreg		; Data pointer advancement
dir	:=	tmp1		; Transfer direction
bas	:=	tmp2		; Socket 0 Base Address (hibyte)
lim	:=	tmp3		; Socket 0 memory limit (hibyte)
tmp	:=	tmp4		; Temporary value

	.endif

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
	.byte	fixup32-fixup31

fixups	= * - fixup

;---------------------------------------------------------------------

; 14 most significant bits are fixed up at runtime
mode		:= $FFFC|0
addr		:= $FFFC|1
data		:= $FFFC|3

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
:	lda (ptr),y
	and #$03
	ora reg
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

	; Indirect Bus I/F mode, Address Auto-Increment
:
fixup01:lda mode
	ora #$03
fixup02:sta mode

	; Retry Time-value Register: = 2000 ?
	ldx #$00		; Hibyte
	ldy #$17		; Lobyte
	jsr set_addr
	lda #$07^$D0
fixup03:eor data
fixup04:eor data
	beq :+
	sec
	rts

	; Check for W5100 shared access
	; RX Memory Size Register: Assign 4+2+1+1KB to socket 0 to 3 ?
:	; ldx #$00		; Hibyte
	ldy #$1A		; Lobyte
	jsr set_addr
fixup05:lda data
	cmp #$06
	beq :+++

	; S/W Reset
	lda #$80
fixup06:sta mode
:
fixup07:lda mode
	bmi :-

	; Indirect Bus I/F mode, Address Auto-Increment, Ping Block
	lda #$13
fixup08:sta mode

	; Source Hardware Address Register: MAC Address
	ldx #$00		; Hibyte
	ldy #$09		; Lobyte
	jsr set_addr
:	lda mac,x
fixup09:sta data
	inx
	cpx #$06
	bcc :-

	; RX Memory Size Register: Assign 4KB each to socket 0 and 1
	; TX Memory Size Register: Assign 4KB each to socket 0 and 1
	ldx #$00		; Hibyte
	ldy #$1A		; Lobyte
	jsr set_addr
	lda #$0A
fixup10:sta data
fixup11:sta data

	; MAC Address: Source Hardware Address Register
:	; ldx #$00		; Hibyte
	ldy #$09		; Lobyte
	jsr set_addr
:
fixup12:lda data
	sta mac,x
	inx
	cpx #$06
	bcc :-

	; Socket 0 Mode Register: MACRAW, MAC Filter
	; Socket 0 Command Register: OPEN
	ldy #$00
	jsr set_addrsocket0
	lda #$44
fixup13:sta data
	lda #$01
fixup14:sta data
	tya
	tax
	clc
	rts

;---------------------------------------------------------------------

poll:
	; Check for completion of previous command
	; Socket 0 Command Register: = 0 ?
	jsr set_addrcmdreg0
fixup15:lda data
	beq :++

	; No data available
	lda #$00
:	tax
	sec
	rts

	; Socket 0 RX Received Size Register: != 0 ?
:	ldy #$26		; Socket RX Received Size Register
	jsr set_addrsocket0
fixup16:lda data		; Hibyte
fixup17:ora data		; Lobyte
	beq :--

	; Process the incoming data
	; -------------------------

	; Set parameters for receiving data
	lda #>$6000		; Socket 0 RX Base Address
	ldx #$00		; Read
	jsr set_parameters

	; ldy #$28		; Socket RX Read Pointer Register
	; jsr set_addrsocket0

	; Calculate and set physical address
	jsr set_addrphysical

	; Move physical address shadow to $F000-$FFFF
	ora #>$F000
	tax

	; Read MAC raw 2byte packet size header
	jsr get_datacheckaddr	; Hibyte
	sta adv+1
	jsr get_datacheckaddr	; Lobyte
	sta adv

	; Subtract 2byte header and set length
	sec
	sbc #<$0002
	sta len
	sta cnt
	lda adv+1
	sbc #>$0002
	sta len+1
	sta cnt+1

	; Is bufsize < length ?
	lda bufsize
	cmp len
	lda bufsize+1
	sbc len+1
	bcs :+

	; Set data length = 0 and skip read
	lda #$00
	sta len
	sta len+1
	beq :++			; Always

	; Read data
:	jsr mov_data

	; Set parameters for common code
:	lda #$40		; RECV
	ldy #$28		; Socket 0 RX Read Pointer Register

	; Advance pointer register
common: jsr set_addrsocket0
	tay			; Save command
	clc
	lda reg
	adc adv
	tax
	lda reg+1
	adc adv+1
fixup18:sta data		; Hibyte
fixup19:stx data		; Lobyte

	; Set command register
	tya			; Restore command
	jsr set_addrcmdreg0
fixup20:sta data

	; Return data length (will be ignored for send)
	lda len
	ldx len+1
	clc
	rts

;---------------------------------------------------------------------

send:
	; Save data length
	sta len
	stx len+1
	sta cnt
	stx cnt+1
	sta adv
	stx adv+1

	; Set parameters for transmitting data
	lda #>$4000		; Socket 0 TX Base Address
	ldx #$01		; Write
	jsr set_parameters

	; Wait for completion of previous command
	; Socket 0 Command Register: = 0 ?
:	jsr set_addrcmdreg0
fixup21:lda data
	bne :-

	; Socket 0 TX Free Size Register: < length ?
:	ldy #$20		; Socket TX Free Size Register
	jsr set_addrsocket0
fixup22:lda data		; Hibyte
fixup23:ldx data		; Lobyte
	cpx len
	sbc len+1
	bcc :-

	; Send the data
	; -------------

	ldy #$24		; Socket TX Write Pointer Register
	jsr set_addrsocket0

	; Calculate and set pyhsical address
	jsr set_addrphysical

	; Write data
	jsr mov_data

	; Set parameters for common code
	lda #$20		; SEND
	ldy #$24		; Socket TX Write Pointer Register
	bne common		; Always

;---------------------------------------------------------------------

exit:
	rts

;---------------------------------------------------------------------

set_addrphysical:
fixup24:lda data		; Hibyte
fixup25:ldy data		; Lobyte
	sty reg
	sta reg+1
	and #>$0FFF		; Socket Mask Address (hibyte)
	ora bas			; Socket Base Address (hibyte)
	tax
set_addr:
fixup26:stx addr		; Hibyte
fixup27:sty addr+1		; Lobyte
	rts

set_addrcmdreg0:
	ldy #$01		; Socket Command Register
set_addrsocket0:
	ldx #>$0400		; Socket 0 register base address
	bne set_addr		; Always

set_addrbase:
	ldx bas			; Socket Base Address (hibyte)
	ldy #<$0000		; Socket Base Address (lobyte)
	beq set_addr		; Always

get_datacheckaddr:
fixup28:lda data
	iny			; Physical address shadow (lobyte)
	bne :+
	inx			; Physical address shadow (hibyte)
	beq set_addrbase
:	rts

;---------------------------------------------------------------------

set_parameters:
	; Setup variables in zero page
	sta bas			; Socket Base Address
	clc
	adc #>$1000		; Socket memory size
	sta lim			; Socket memory limit
	stx dir			; Transfer direction

	; Set indirect addressing pointer
	lda bufaddr
	ldx bufaddr+1
	sta ptr
	stx ptr+1
	rts

;---------------------------------------------------------------------

mov_data:
	; Calculate highest R/W address allowing
	; to R/W without address wraparound
	sec
	lda #<$0000		; Socket memory limit (lobyte)
	sbc len
	tay
	lda lim			; Socket memory limit (hibyte)
	sbc len+1
	tax
	tya

	; R/W without address wraparound possible because 
	; highest R/W address > actual R/W address ?
	; sec
fixup29:sbc addr+1		; Lobyte
	tay
	txa
fixup30:sbc addr		; Hibyte
	tax
	tya
	bcs :+

	; Calculate length of first chunk
	; clc
	adc len
	sta cnt
	tay
	txa
	adc len+1
	sta cnt+1
	tax
	tya

	; R/W first chunk
	jsr rw_data

	; Wraparound R/W address
	jsr set_addrbase

	; Set buffer pointer for second chunk
	clc
	lda bufaddr
	adc cnt
	sta ptr
	lda bufaddr+1
	adc cnt+1
	sta ptr+1

	; Calculate length of second chunk
	sec
	lda len
	sbc cnt
	sta cnt
	lda len+1
	sbc cnt+1
	sta cnt+1

	; Get length of (second) chunk
:	lda cnt
	ldx cnt+1

	; R/W (second) chunk
rw_data:eor #$FF		; Two's complement part 1
	tay
	iny			; Two's complement part 2
	sty tmp
	sec
	lda ptr
	sbc tmp
	sta ptr
	lda ptr+1
	sbc #$00
	sta ptr+1
	lda dir			; Transfer direction
	bne :++

	; Read data
:
fixup31:lda data
	sta (ptr),y
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-
	rts

	; Write data
:	lda (ptr),y
fixup32:sta data
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-
	rts

;---------------------------------------------------------------------
