;
; Copyright (c) 2004, Adam Dunkels.
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
; Author: Adam Dunkels <adam@sics.se>
;
; $Id: mtarch-asm.S,v 1.1 2007/12/15 00:14:19 oliverschmidt Exp $
;
;---------------------------------------------------------------------       
	.import		__ZP_START__
	
	.import		_mtarch_asm_threadspreg
	.import		_mtarch_asm_threadzp	
	.import		_mtarch_asm_threadstack
	
	.export		_mtarch_asm_start
	.export		_mtarch_asm_yield
	.export		_mtarch_asm_exec
	
;---------------------------------------------------------------------
.bss
kernelspreg:	.res 1
zpsave:		.res 2
	
zpspace	= 26	; see <cc65 source>/asminc/zeropage.inc

;---------------------------------------------------------------------	
.code
; Switch to thread defined by threadzp, threadstack and threadspreg.
; The kernel CPU stack is swapped onto the threadstack, and the
; CPU stack pointer is saved to the local variable "kernelspreg".
; Also, the zeropage variables are saved.
_mtarch_asm_exec:
	sei
	
	; Save kernel CPU stack pointer
	tsx
	stx kernelspreg

	; Save zp locations FE/FF
	lda $FE
	sta zpsave
	lda $FF
	sta zpsave+1

	; Get zero page buffer addr
	lda _mtarch_asm_threadzp
	sta $FE
	lda _mtarch_asm_threadzp+1
	sta $FF

	; Swap zero page content with buffer content
	ldy #0
:	lda <__ZP_START__,y
	tax
	lda ($FE),y
	sta <__ZP_START__,y
	txa
	sta ($FE),y	
	iny
	cpy #zpspace
	bne :-
	
	; Get CPU stack buffer addr
	lda _mtarch_asm_threadstack
	sta $FE
	lda _mtarch_asm_threadstack+1
	sta $FF

	; Get CPU stack size
	ldy kernelspreg             ; Determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg ; as we only need to swap the used part of the stack.
        bcc :+
        ldy _mtarch_asm_threadspreg

	; Swap CPU stack content with buffer content
:	lda $0100,y
	tax	
	lda ($FE),y
	sta $0100,y
	txa
	sta ($FE),y
	iny
	bne :-

	; Restore zp locations FE/FF
	lda zpsave
	sta $FE
	lda zpsave+1
	sta $FF

	; Set thread CPU stack pointer
	ldx _mtarch_asm_threadspreg
	txs

	pla
	tay
	pla
	tax
	pla
	rti

;---------------------------------------------------------------------
; Switch from thread defined by threadzp, threadstack and threadspreg.
; The kernel CPU stack is swapped back from the threadstack, and the
; CPU stack pointer is restored from the local variable "kernelspreg".
; Also, the zeropage variables are restored.
_mtarch_asm_yield:
	php
	pha
	txa
	pha
	tya
	pha

	sei
        tsx                     ; The rts adds 1 to the PC
                                ; saved on the stack. We want
        lda $0105,x             ; the stack to look like is would
        clc                     ; do inside of an interrupt
        adc #1                  ; (this is what the 'rts' does,
        sta $0105,x             ;  but not the 'rti').
        lda $0106,x
        adc #0
        sta $0106,x
 	
	; Save thread CPU stack pointer
	tsx
	stx _mtarch_asm_threadspreg

	; Save zp locations FE/FF
	lda $FE
	sta zpsave
	lda $FF
	sta zpsave+1

	; Get zero page buffer addr
	lda _mtarch_asm_threadzp
	sta $FE
	lda _mtarch_asm_threadzp+1
	sta $FF

	; Swap zero page content with buffer content
	ldy #0
:	lda <__ZP_START__,y
	tax
	lda ($FE),y
	sta <__ZP_START__,y
	txa
	sta ($FE),y	
	iny
	cpy #zpspace
	bne :-

	; Get CPU stack buffer addr
	lda _mtarch_asm_threadstack
	sta $FE
	lda _mtarch_asm_threadstack+1
	sta $FF

	; Get CPU stack size
	ldy kernelspreg             ; Determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg ; as we only need to swap the used part of the stack.
        bcc :+
        ldy _mtarch_asm_threadspreg

	; Swap CPU stack content with buffer content
:	lda $0100,y
	tax	
	lda ($FE),y
	sta $0100,y
	txa
	sta ($FE),y
	iny
	bne :-

	; Restore zp locations FE/FF
	lda zpsave
	sta $FE
	lda zpsave+1
	sta $FF

	; Set kernel CPU stack pointer
	ldx kernelspreg
	txs

	cli	
	rts
;---------------------------------------------------------------------
_mtarch_asm_start:
	; Save zp locations FE/FF
	lda $FE
	sta zpsave
	lda $FF
	sta zpsave+1

	; Get zero page buffer addr
	lda _mtarch_asm_threadzp
	sta $FE
	lda _mtarch_asm_threadzp+1
	sta $FF

	; Copy zero page content to buffer
	ldy #$00
:	lda <__ZP_START__,y
	sta ($FE),y	
	iny
	cpy #zpspace
	bne :-

	; Restore zp locations FE/FF
	lda zpsave
	sta $FE
	lda zpsave+1
	sta $FF

	rts
;---------------------------------------------------------------------
