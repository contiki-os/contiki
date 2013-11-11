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

	.segment	"JUMPTABLE"

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
	.addr	init
	.addr	poll
	.addr	send
	.addr	exit

;---------------------------------------------------------------------

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
	.byte	fixup26-fixup25, fixup27-fixup26

fixups	= * - fixup

;---------------------------------------------------------------------

mode		:= $FF00	; High byte patched at runtime
addr		:= $FF01	; High byte patched at runtime
data		:= $FF03	; High byte patched at runtime

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

	; S/W Reset
:	lda #$80
fixup01:sta mode
:
fixup02:lda mode
	bmi :-

	; Indirect Bus I/F mode, Address Auto-Increment, Ping Block
	lda #$13
fixup03:sta mode

	; Source Hardware Address Register: MAC Address
	ldx #$00		; Hibyte
	ldy #$09		; Lobyte
	jsr set_addr
:	lda mac,x
fixup04:sta data
	inx
	cpx #$06
	bcc :-

	; RX Memory Size Register: Assign 8KB to socket 0
	; TX Memory Size Register: Assign 8KB to socket 0
	ldx #$00		; Hibyte
	ldy #$1A		; Lobyte
	jsr set_addr
	lda #$03
fixup05:sta data
fixup06:sta data

	; Socket 0 Mode Register: MACRAW, MAC Filter
	; Socket 0 Command Register: OPEN
	ldy #$00
	jsr set_addrsocket0
	lda #$44
fixup07:sta data
	lda #$01
fixup08:sta data
	rts

;---------------------------------------------------------------------

poll:
	; Check for completion of previous command
	; Socket 0 Command Register: = 0 ?
	jsr set_addrcmdreg0
fixup09:lda data
	beq :++

	; No data available
	lda #$00
:	tax
	rts

	; Socket RX Received Size Register: != 0 ?
:	ldy #$26		; Socket RX Received Size Register
	jsr set_addrsocket0
fixup10:lda data		; Hibyte
fixup11:ora data		; Lobyte
	beq :--

	; Process the incoming data
	; -------------------------

	; Set parameters for receiving data
	lda #>$6000		; Socket 0 RX Base Address
	ldx #$00		; Read
	jsr set_parameters

	; ldy #$28		; Socket RX Read Pointer Register
	; jsr set_addrsocket0

	; Calculate and set pyhsical address
	jsr set_addrphysical

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
	lda reg
	clc
	adc adv
	tax
	lda reg+1
	adc adv+1
fixup12:sta data		; Hibyte
fixup13:stx data		; Lobyte

	; Set command register
	tya			; Restore command
	jsr set_addrcmdreg0
fixup14:sta data

	; Return data length (will be ignored for send)
	lda len
	ldx len+1
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
fixup15:lda data
	bne :-

	; Socket 0 TX Free Size Register: < length ?
:	ldy #$20
	jsr set_addrsocket0
fixup16:lda data		; Hibyte
fixup17:ldx data		; Lobyte
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
fixup18:lda data		; Hibyte
fixup19:ldy data		; Lobyte
	sta reg+1
	sty reg
	and #>$1FFF		; Socket Mask Address (hibyte)
	ora bas			; Socket Base Address (hibyte)
	tax
set_addr:
fixup20:stx addr		; Hibyte
fixup21:sty addr+1		; Lobyte
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
fixup22:lda data
fixup23:ldx addr		; Hibyte
	cpx lim			; Socket memory limit (hibyte)
	bcs set_addrbase
	rts

;---------------------------------------------------------------------

set_parameters:
	; Setup variables in zero page
	sta bas			; Socket Base Address
	clc
	adc #>$2000		; Socket memory size
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
fixup24:sbc addr+1		; Lobyte
	tay
	txa
fixup25:sbc addr		; Hibyte
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
fixup26:lda data
	sta (ptr),y
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-
	rts

	; Write data
:	lda (ptr),y
fixup27:sta data
	iny
	bne :-
	inc ptr+1
	dex
	bpl :-
	rts

;---------------------------------------------------------------------
