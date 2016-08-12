;
; Copyright (c) 2008, Swedish Institute of Computer Science.
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
	.constructor init_pfs
	.destructor  done_pfs
	.importzp    ptr1
	.import      popax, _uip_aligned_buf
	.export      _pfs_open, _pfs_read, _pfs_close
;---------------------------------------------------------------------
pathname	:= $0280
mli		:= $BF00
level		:= $BF94

OPEN_CALL	= $C8
READ_CALL	= $CA
CLOSE_CALL	= $CC
;---------------------------------------------------------------------
	.bss

prefix:	.res	1
;---------------------------------------------------------------------
	.data

open_param:	.byte	$03		    ;PARAM_COUNT
		.addr	pathname	    ;PATHNAME
		.byte	$00		    ;IO_BUFFER (Lo)
		.byte	>_uip_aligned_buf+1 ;IO_BUFFER (Hi)
open_fd:	.byte	$00		    ;REF_NUM

read_param:	.byte	$04		    ;PARAM_COUNT
read_fd:	.byte	$00		    ;REF_NUM
read_buffer:	.addr	$0000		    ;DATA_BUFFER
read_count_in:	.word	$0000		    ;REQUEST_COUNT
read_count_out:	.word	$0000		    ;TRANS_COUNT

close_param:	.byte	$01		    ;PARAM_COUNT
close_fd:	.byte	$00		    ;REF_NUM
;---------------------------------------------------------------------
	.segment	"ONCE"

init_pfs:
	; Allow exec() to keep file open
	inc level

	; Get prefix len of path used to load binary
	ldx pathname
:	lda pathname,x
	cmp #'/'
	beq :+
	dex
	bne :-
:	stx prefix
	rts
;---------------------------------------------------------------------
	.code

done_pfs:
	; Close all file
	lda #$00
	jsr _pfs_close

	; Allow exec() to keep file open
	dec level
	rts

_pfs_open:
	; Pop and store name
	jsr popax
	sta ptr1
	stx ptr1+1
	
	; Append name to prefix
	ldy #$00
	ldx prefix
:	lda (ptr1),y
	beq :+
	sta pathname+1,x
	iny
	inx
	bne :-
:	stx pathname

	jsr mli
	.byte OPEN_CALL
	.addr open_param
	bcs error

	; Return fd
	lda open_fd
	ldx #$00
	rts

_pfs_read:
	; Store len requested
	sta read_count_in
	stx read_count_in+1

	; Pop and store buf
	jsr popax
	sta read_buffer
	stx read_buffer+1

	; Pop and store fd
	jsr popax
	sta read_fd

	jsr mli
	.byte READ_CALL
	.addr read_param
	bcs error

	; Return len read
	lda read_count_out
	ldx read_count_out+1
	rts

_pfs_close:
	; Store fd
	sta close_fd

	jsr mli
	.byte CLOSE_CALL
	.addr close_param
	bcs error
	rts

error:
	; Return -1
	lda #$FF
	tax
	rts
;---------------------------------------------------------------------
