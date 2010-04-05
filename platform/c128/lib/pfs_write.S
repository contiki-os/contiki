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
	.define F_IDE64 0	; support IDE64, 100 byte only

	.importzp    ptr1, ptr2
	.import	     pfs_rwcommon, pfs_rwsetflags, pfs_rwcommonend
	.if	F_IDE64
	.import      ide64_rwprepare, ide64_rwfinish
	.endif	
	.export	     _pfs_write
;---------------------------------------------------------------------
F_NBLK		= $40
ST		= $90	;status
CHKOUT		= $FFC9
CLRCHN		= $FFCC
CHROUT		= $FFD2
WRITE		= $DEF1
;---------------------------------------------------------------------
	.code

error5: jsr     CLRCHN		; clrchn

error2:	ldx	#255
	txa
	rts

.proc   _pfs_write
        jsr     pfs_rwcommon    ; pop params, check handle
        beq	error2		; not open

	.if	F_IDE64
	asl
        bmi     nblk		; no block operation

        jsr     CHKOUT
        bcs     error2

        ; check support
        jsr     ide64_rwprepare
        bcs     norm

        ; write
        jsr     WRITE
        bcs     nosup
        jmp     ide64_rwfinish

nosup:  lda     #F_NBLK
        jsr     pfs_rwsetflags
        .endif

        ; Valid lfn. Make it the output file
nblk:   jsr     CHKOUT
        bcs     error2

        ; Output the next character from the buffer
norm:	ldy	#0
@L3:    inc     ptr1
        bne     @L0
        inc     ptr1+1
        beq     @L2

@L0:    lda     (ptr2),y
        inc     ptr2
        bne     @L1
        inc     ptr2+1          ; A = *buf++;
@L1:    jsr     CHROUT
	lda	ST
	beq	@L3
        bne     error5          ; bail out on errors
@L2:

        ; Wrote all chars, close the output channel
        jsr     CLRCHN

        ; Return the number of chars written
        jmp     pfs_rwcommonend
.endproc
