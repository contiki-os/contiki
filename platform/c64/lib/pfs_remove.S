;
; Copyright (c) 2016, Greg King
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions, and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions, and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
; 3. Neither the name of the author nor the names of contributors
;    may be used to endorse or promote products derived from this software
;    without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS"; AND,
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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
; Author: Greg King <gregdk@users.sf.net>
;
;---------------------------------------------------------------------
	.importzp	ptr2
	.import		addysp
	.import		pfs_makename, pfs_scratch

	.export		_pfs_remove
;---------------------------------------------------------------------
MAXLEN	=	80		; max. filename length

FNL	:=	$B7		; filename length
;---------------------------------------------------------------------
.proc	_pfs_remove
	asl	ptr2		; force pfs_makename to format for pfs_scratch
	jsr	pfs_makename
	ldx	FNL
	beq	error		; no name

	jsr	pfs_scratch
	ldx	#>$0000
	bcs	error
ret:	txa
	ldy	#MAXLEN + 8	; free filename space
	jmp	addysp

error:	dex			;(ldx #>-1)
	bne	ret
.endproc
