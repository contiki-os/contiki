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
; Author: Greg King <gregdk@users.sf.net>
;
;---------------------------------------------------------------------
.define	F_IDE64		0	; C128 doesn't have IDE64

	.constructor	init_pfs
	.destructor	done_pfs
	.importzp	sp, ptr1, ptr2, ptr3, tmp1
	.import		curunit, __filetype, popax, addysp, subysp
	.export		pfs_rwcommon, pfs_rwsetflags, pfs_rwcommonend
.if	F_IDE64
	.export		ide64_rwprepare, ide64_rwfinish
.endif
	.export		cmdc, flags
	.export		pfs_makename, pfs_scratch

	.export		_pfs_open, _pfs_read, _pfs_close

;---------------------------------------------------------------------
MAXLEN	=	80		;maximum filename length

; Flag bits
F_EOF	=	%10000000	;end of file
F_NBLK	=	%01000000	;block read/write not available
F_OPEN	=	%00100000

; Kernal variables
ST	:=	$90		;status
FN	:=	$BB		;filename
FNL	:=	$B7		;filename length
LF	:=	$B8		;logical file number

OPNVec	:=	$031A		;address vector to OPEN function's code

; IDEDOS function
READ	:=	$DEF4

; Kernal functions
SETLFS	:=	$FFBA
SETNAM	:=	$FFBD
OPEN	:=	$FFC0
CLOSE	:=	$FFC3
CHKIN	:=	$FFC6
CHKOUT	:=	$FFC9
CLRCHN	:=	$FFCC
CHRIN	:=	$FFCF
CHROUT	:=	$FFD2
;---------------------------------------------------------------------
	.data

; illchr and sw must stay together because the comma, also, is illegal in names.
illchr:	.byte	"*?:="		;illegal chars
sw:	.byte	",s,w"
cmdc:	.byte	0
flags:	.res	10		;(Kernal allows only ten open files)
;---------------------------------------------------------------------
	.segment	"ONCE"

init_pfs:
	ldy	#MAXLEN + 8
	jsr	subysp		;allocate because open2 will free it
	lda	#0		;no name, file number 1
	sta	FNL
	ldy	#15 - 1		;secondary address 15
	jsr	open2		;open command channel
	sta	cmdc
	rts
;---------------------------------------------------------------------
	.code

_pfs_open:
	sta	ptr2		;save open-mode flags

	; Get and store name
	jsr	popax
	jsr	pfs_makename
	lda	FNL
	beq	error		;must have a filename

	; A directory listing needs a special secondary address
	ldy	tmp1
	bne	open1		;branch if not directory
	lda	#2 - 1
	dey			;.Y = 0 - 1
	bne	open2		;branch always

open1:	lda	#2 - 1		;file number
	tay			;secondary address
open2:	sta	ptr2
	sty	ptr2 + 1

next:	inc	ptr2		;next file number
	ldx	ptr2		;file number
	cpx	#.sizeof(flags) + 1
	bcs	error		;no more files
	lda	flags - 1,x
	bne	next		;already used
nextsa:	inc	ptr2 + 1	;next channel
retr:	lda	ptr2		;file number
	ldx	curunit
	ldy	ptr2 + 1	;secondary address (channel number)
	jsr	SETLFS
	jsr	OPEN		;open a pair of files (in computer and drive)
	bcs	oerr		;branch if could not open computer file
	ldx	cmdc
	beq	opok		;branch if error channel just was openned
	jsr	CHKIN
	bcs	error
	jsr	CHRIN
	pha			;first digit of error code
	jsr	CHRIN
	sta	ptr1		;second digit
@L4:	jsr	CHRIN		;flush status message
	lda	ST
	beq	@L4
	jsr	CLRCHN
	pla
	cmp	#'2'
	bcc	opok		;no serious error
	pha
	lda	ptr2
	jsr	CLOSE		;close computer file
	pla
	ldx	ptr1
	cmp	#'7'		;no channel?
	bne	nnoc
	cpx	#'0'
	bne	error		;not "no channel"
	lda	ptr2 + 1
	cmp	#14
	bcc	nextsa		;try next channel
	bcs	error		;give up

opok:	ldx	ptr2
	lda	#F_OPEN
	sta	flags - 1,x
	txa			;OK, return file number
ret0:	ldx	#>$0000
ret:	ldy	#MAXLEN + 8	;free filename space
	jmp	addysp

oerr:	dec	ptr2 + 1
	cmp	#2		;already open,
	beq	next		;retry with next

error:	lda	#<-1
	tax			;failed
	bne	ret

nnoc:	inc	ptr3
	bne	error		;no retry
	cmp	#'6'
	bne	error		;not "file exists"
	cpx	#'3'
	bne	error
	jsr	pfs_scratch
	bcc	retr
	bcs	error		;branch always

pfs_scratch:
	ldx	cmdc
	jsr	CHKOUT
	bcs	@L5
	ldy	#1
	lda	#'s'		;scratch
@L4:	jsr	CHROUT
	lda	(FN),y
	iny
	cmp	#','
	bne	@L4
	lda	#$0D		;carriage return
	jsr	CHROUT
	jsr	CLRCHN
	clc			;carry = 0: OK
@L5:	rts			;carry = 1: error

pfs_makename:
	sta	FN
	stx	FN+1		;Kernal filename pointer
	ldy	#MAXLEN + 8
	jsr	subysp		;allocate name space

	; Validate the name; and, find its length
	ldy	#<-1
	sty	ptr3		;init. the count of retries
	sty	ptr1
@L10:	iny
	cpy	#MAXLEN
	bcs	badchr		;too long
	lda	(FN),y
	ldx	#.sizeof(illchr); 4 + 1 (includes comma)
@L12:	cmp	illchr,x
	beq	badchr		;illegal char
	dex
	bpl	@L12
	cmp	#'/'
	bne 	@L11
	sty	ptr1		;last slash
@L11:	tax			;test for '\0'
	bne	@L10
	cpy	#0
	beq	badchr		;no name
	sty	FNL		;save original length

	tay			;zero index reg.
	lda	(FN),y
	sta	tmp1		;preset not-directory flag to true
	cmp	#'$'
	bne	@L9
	sty	tmp1		;set not-directory flag to false
	rts

@L9:	lda	#'0'		;drive 0 or current partition
	sta	(sp),y
	iny
	inc	ptr1
	beq	nopath
	lda	#'/'
@L13:	sta	(sp),y
	iny
	lda	(FN,x)
	inc	FN
	bne	@L14
	inc	FN+1
@L14:	cpy	ptr1
	bcc	@L13
	;lda	#'/'		; (.A already has a slash)
	sta	(sp),y
	iny
nopath: lda	#':'
@L16:	sta	(sp),y
	iny
	lda	(FN,x)
	inc	FN
	bne	@L15
	inc	FN+1
@L15:	ora	#$00		;test for '\0'
	bne	@L16
	lsr	ptr2
	bcs	ro		;read-only (read-write not supported)
	lda	__filetype
	sta	sw + 1		;set filetype
	lsr	ptr2
	lda	#'w'		;write-only
	lsr	ptr2
	bcc	write
	lda	#'a'		;append
write:	sta	sw + 3		;set mode
	ldx	#$0100 - .sizeof(sw)
@L20:	lda	sw - ($0100 - .sizeof(sw)),x
	sta	(sp),y
	iny
	inx
	bne	@L20
ro:	tya			;pathname length
	ldx	sp
	ldy	sp+1
namset:	jmp	SETNAM

badchr:	lda	#0
	beq	namset

.proc   _pfs_read
        jsr     pfs_rwcommon    ; pop params, check handle
	beq	error2		; not open

	bmi	eof

.if	F_IDE64
	asl	a
        bmi     nblk		; no block operation

        jsr     CHKIN
        bcs     error2

        ; check support
        jsr     ide64_rwprepare
        bcs     norm

        ; do a block read
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
        beq     done

        ; Read the next byte
@L0:    jsr     CHRIN
        tax                     ; save the input byte

        lda	ST              ; read the file status
        cmp     #$01            ; save it
        and     #%10111111      ; check anything but the EOF bit
        bne     error5          ; assume device not present

        ; Store the byte just read
        txa
        sta     (ptr2),y
        inc     ptr2
        bne     @L1
        inc     ptr2+1          ; *buf++ = A;

        ; Get the status again; and, check the EOF bit
@L1:    bcc     @L3             ; loop if not end of file

        ; Set the EOF flag; and, bail out
        lda     #F_EOF
        jsr     pfs_rwsetflags

        ; Read done; cancel the input channel
done:   jsr     CLRCHN

        ; Return the number of chars read
eof:
;       jmp     pfs_rwcommonend ; (fall through)
.endproc

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

done_pfs:
	ldx	#.sizeof(flags)
@L2:	ldy	flags - 1,x	; file open?
	beq	@L1
	txa
	jsr     close1
@L1:	dex
	bne	@L2
	rts

error5: jsr     CLRCHN

        ; Error entry, file is not open
error2:	ldx	#>-1
	txa
	rts

_pfs_close:
	cmp	#.sizeof(flags) + 1
	bcs	close0		; don't close if not valid file number
close1:	pha
	jsr	CLOSE
	pla
	tax
	lda	#$00
	sta	flags - 1,x
close0:	rts			; .X = file number

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
        lda	#$00
;       beq	pfs_rwsetflags  ; (fall through)
.endproc

.proc	pfs_rwsetflags
	ldx 	LF
	ora 	flags - 1,x
	sta 	flags - 1,x
	rts
.endproc

.if	F_IDE64
.proc   ide64_rwprepare
        sec                     ; assume it won't use IDEDOS
        lda     ptr1+1
        eor     #$FF		; convert -count-1 back to count
        beq 	small		; too small, not worth it
        tay
        lda     ptr1            ; set up registers
        eor     #$FF
        tax
        lda     OPNVec+1
        eor     #>$DE00
        bne     noide           ; is OPEN vector set to IDEDOS?
        lda     $DE60
        eor     #'i'
        bne     noide           ; check identification
        lda     $DE61
        eor     #'d'
        bne     noide
        lda     $DE62
        eor     #'e'
        bne     noide
        clc                     ; it will use IDEDOS
        lda     #ptr2
small:  rts

noide:  lda     #F_NBLK
	bne	pfs_rwsetflags
.endproc

.proc   ide64_rwfinish
        txa                     ; push .YX
        pha
        tya
        pha
        jsr     CLRCHN
        pla                     ; pull .XA
        tax
        pla
        rts
.endproc
.endif
