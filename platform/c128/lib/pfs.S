;
; Copyright (c) 2010, Kajtar Zsolt <soci@c64.rulez.org>
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
; Author: Kajtar Zsolt <soci@c64.rulez.org>
;
;---------------------------------------------------------------------
	.define F_IDE64 0	; support IDE64, not on C128

	.constructor init_pfs
        .destructor  done_pfs
	.importzp    ptr1, ptr2, ptr3, sp
	.import	     curunit, __filetype, popax, addysp, subysp
	.export	     pfs_rwcommon, pfs_rwsetflags, pfs_rwcommonend
	.if	F_IDE64
	.export      ide64_rwprepare, ide64_rwfinish
	.endif	
	.export	     _pfs_open, _pfs_read, _pfs_close
;---------------------------------------------------------------------
F_EOF		= $80
F_NBLK		= $40
F_OPEN		= $20
F_MAXLEN	= 80	;max filename length
ST		= $90	;status
FN		= $BB	;filename
FNL		= $B7	;filenamelength
LF		= $B8	;logical file number
SA		= $B9	;secondary address
OPEN		= $FFC0
CLOSE		= $FFC3
CHKIN		= $FFC6
CHKOUT		= $FFC9
CLRCHN		= $FFCC
CHRIN		= $FFCF
CHROUT		= $FFD2
SETLFS		= $FFBA
SETNAM		= $FFBD
CLALL		= $FFE7
WRITE		= $DEF1
READ		= $DEF4
;---------------------------------------------------------------------
	.bss

cmdc:	.res	1
flags:	.res	10
;---------------------------------------------------------------------
	.data

illchr:	.byte	$3A, $2A, $3F, $3D	;illegal chars
pw:	.byte	$2C, $50, $2C, $57	;,p,w
;---------------------------------------------------------------------
	.segment	"INIT"

init_pfs:
	ldy	#F_MAXLEN+8
	jsr	subysp	;allocate
	lda	#0
	sta	FNL	;no name
	ldy	#15-1
	jsr	open2	;open command channel
	sta	cmdc
	rts
;---------------------------------------------------------------------
	.code

error3: jmp error

_pfs_open:
	sta	ptr2
	; Pop and store name
	jsr	popax
	sta	FN
	stx	FN+1	;filename (kernal)
	ldy	#F_MAXLEN+8
	jsr	subysp	;allocate name
	ldy	#255
	sty	ptr3
	sty	ptr1
@L10:	iny
	cpy	#F_MAXLEN
	bcs	error3	;too long...
	ldx	#4	;4+1 (comma)
@L12:	cmp	illchr,x
	beq	error3	;illegal char?
	dex
	bpl	@L12
	cmp	#$2F
	bne 	@L11
	sty	ptr1	;last slash
@L11:	lda	(FN),y
	bne	@L10
	sty	FNL

	tay
	tax
	lda	#$30	;this partition
	sta	(sp),y
	iny
	inc	ptr1
	beq	nopath
	lda	#$2F
@L13:	sta	(sp),y
	iny
	lda	(FN,x)
	inc	FN
	bne	@L14
	inc	FN+1
@L14:	cpy	ptr1
	bcc	@L13
	lda	#$2F
	sta	(sp),y
	iny
nopath: lda	#$3A
@L16:	sta	(sp),y
	iny
	lda	(FN,x)
	inc	FN
	bne	@L15
	inc	FN+1
@L15:	ora	#0
	bne	@L16
	lsr	ptr2
	bcs	ro	;read only
	lda	__filetype
	sta	pw+1	;set filetype
	ldx	#252
@L20:	lda	pw-252,x
	sta	(sp),y	;write
	iny
	inx
	bne	@L20
ro:	tya		;name length (kernal)
	ldx	sp
	ldy	sp+1
	jsr	SETNAM

	lda	#0	;file number
	tay		;secondary address
open2:	sta	ptr2
	sty	ptr2+1

next:	inc	ptr2	;next file number
	ldx	ptr2	;file number
	cpx	#11
	bcs	error	;no more files
	lda	flags-1,x
	bne	next	;already used
	lda	ptr2+1
	bne	nextsa
	inx
	stx	ptr2+1
nextsa:	inc	ptr2+1	;next channel
retr:	lda	ptr2	;file number
	ldx	curunit
	ldy	ptr2+1	;secondary address
	jsr	SETLFS
	jsr	OPEN	;open
	bcs	oerr
	ldx	cmdc
	beq	opok	;error channel open
	jsr	CHKIN
	bcs	error
	jsr	CHRIN
	pha
	jsr	CHRIN
	sta	ptr1
@L4:	jsr	CHRIN
	lda	ST
	beq	@L4
	jsr	CLRCHN
	pla
	tax
	lsr
	cmp	#$18	;no serious error
	beq	opok
	txa
	pha
	lda	ptr2
	jsr	CLOSE	;close
	pla
	ldx	ptr1
	cmp	#$37	;no channel?
	bne	nnoc
	cpx	#$30
	bne	error	;not no channel
	lda	ptr2+1
	cmp	#14
	bcc	nextsa	;try next channel
	bcs	error	;give up

opok:	ldx	ptr2
	lda	#F_OPEN
	sta	flags-1,x
	txa		;ok, return file number
	ldx	#0
ret:	ldy	#F_MAXLEN+8	; free filename
	jmp	addysp

oerr:	dec	ptr2+1
	cmp	#2	;already open,
	beq	next	;retry with next

error:	lda	#$FF
	tax		;failed
	bne	ret

nnoc:	inc	ptr3
	bne	error	;no retry
	cmp	#$36
	bne	error	;no exists
	cpx	#$33
	bne	error
	ldx	cmdc
	jsr	CHKOUT
	bcs	error
	lda	FNL
	sec
	sbc	#5
	tax
	lda	#$53	;scratch
	jsr	CHROUT
	ldy	#1
@L4:	lda	(FN),y
	iny
	jsr	CHROUT
	dex
	bne	@L4
	lda	#$3D
	jsr	CHROUT
	iny
	lda	(FN),y
	jsr	CHROUT
	lda	#$0d
	jsr	CHROUT
	jsr	CLRCHN
	jmp	retr

.proc   _pfs_read
        jsr     pfs_rwcommon    ; pop params, check handle
	beq	error2		; not open

	bmi	eof
	.if	F_IDE64
	asl
        bmi     nblk		; no block operation

        jsr     CHKIN
        bcs     error2
        
        ; check support
        jsr     ide64_rwprepare
        bcs     norm
        
        ; read
        jsr     READ
        bcs     nosup
        jmp     ide64_rwfinish

nosup:  lda     #F_NBLK
	jsr	pfs_rwsetflags
	.endif

        ; Valid lfn. Make it the input file
nblk:   jsr     CHKIN
        bcs     error2

        ; Decrement the count
norm:	ldy	#0
@L3:    inc     ptr1
        bne     @L0
        inc     ptr1+1
        beq     done            ; branch always

        ; Read the next byte
@L0:    jsr     CHRIN
        tax                     ; save the input byte

        lda	ST              ; read the IEEE status
        cmp     #1              ; save it
        and     #%10111111      ; check anything but the EOI bit
        bne     error5          ; assume device not present

        ; Store the byte just read
        txa
        sta     (ptr2),y
        inc     ptr2
        bne     @L1
        inc     ptr2+1          ; *buf++ = A;

        ; Get the status again and check the EOI bit
@L1:    bcc     @L3             ; loop if no end of file

        ; Set the EOI flag and bail out
        lda     #F_EOF
        jsr     pfs_rwsetflags

        ; Read done, close the input channel
done:   jsr     CLRCHN		; clrchn

        ; Return the number of chars read
eof:    jmp     pfs_rwcommonend
.endproc

        ; Error entry, file is not open
done_pfs:
	ldx	#10
@L2:	ldy	flags-1,x	; file open?
	beq	@L1
	txa
	jsr     _pfs_close
@L1:	dex
	bne @L2
	rts

error5: jsr     CLRCHN		; clrchn

error2:	ldx	#255
	txa
	rts

_pfs_close:
	pha
	jsr	CLOSE	;close
	pla
	tax
	lda	#0
	sta	flags-1,x
	rts

.proc   pfs_rwcommon
        eor     #$FF
        sta     ptr1
        txa
        eor     #$FF
        sta     ptr1+1          ; remember -count-1

        jsr     popax           ; get buf
        sta     ptr2
        stx     ptr2+1

        sta     ptr3
        stx     ptr3+1          ; for length

        jsr     popax           ; get the handle
        sta	LF
        lda	#0
        beq	pfs_rwsetflags
.endproc

	.if	F_IDE64
.proc   ide64_rwprepare
        sec
        lda     ptr1+1
        eor     #255
        beq 	small		; too small, not worth it
        tay
        lda     ptr1            ; setup registers
        eor     #255
        tax
        lda     $031B
        eor     #$DE
        bne     noide           ; open vector set?
        lda     $DE60
        eor     #$49
        bne     noide           ; check identification
        lda     $DE61
        eor     #$44
        bne     noide
        lda     $DE62
        eor     #$45
        bne     noide
        clc
        lda     #ptr2
small:  rts
        
noide:  lda     #F_NBLK
	bne	pfs_rwsetflags
.endproc
	.endif

.proc	pfs_rwsetflags
	ldx 	LF
	ora 	flags-1,x
	sta 	flags-1,x
	rts
.endproc

	.if	F_IDE64
.proc   ide64_rwfinish
        txa
        pha
        tya
        pha
        jsr     CLRCHN
        pla
        tax
        pla
        rts
.endproc
	.endif

.proc   pfs_rwcommonend
        lda     ptr2
        sec
        sbc     ptr3
        pha
        lda     ptr2+1
        sbc     ptr3+1
        tax
        pla
        rts
.endproc
