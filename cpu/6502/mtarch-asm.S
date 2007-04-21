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
; $Id: mtarch-asm.S,v 1.1 2007/04/21 22:15:45 oliverschmidt Exp $
;
;---------------------------------------------------------------------       
	.importzp       ptr1
	.importzp	sp
	.import		__ZP_START__
	
	.import		_mtarch_asm_threadspreg
	.import		_mtarch_asm_threadsp
	.import		_mtarch_asm_threadzp	
	.import		_mtarch_asm_threadstack
	
	.export		_mtarch_asm_start
	.export		_mtarch_asm_yield
	.export		_mtarch_asm_exec
	
;---------------------------------------------------------------------
.bss
kernelsp:	.res 2
kernelspreg:	.res 1		
	
zpsize	= 32

;---------------------------------------------------------------------	
.code
; Switch to thread defined by threadsp, threadstack and threadspreg.
; The kernel stack is swapped onto the threadstack, and the
; sp and spreg are saved to the local variables "kernelsp" and
; "kernelspreg". Also, the zeropage variables are saved.
_mtarch_asm_exec:
	sei
	; Save current stack pointer
	lda sp
	sta kernelsp
	lda sp+1
	sta kernelsp+1
	
	tsx
	stx kernelspreg
	
	lda _mtarch_asm_threadzp
	sta ptr1
	lda _mtarch_asm_threadzp+1
	sta ptr1+1

	ldy #0
:	lda <__ZP_START__,y
	tax
	lda (ptr1),y
	sta <__ZP_START__,y
	txa
	sta (ptr1),y	
	iny
	cpy #zpsize
	bne :-
	
	lda _mtarch_asm_threadstack
	sta ptr1
	lda _mtarch_asm_threadstack+1
	sta ptr1+1

	ldy kernelspreg             ; Determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg ; as we only need to swap the used part of the stack.
        bcc :+
        ldy _mtarch_asm_threadspreg

:	lda $0100,y
	tax	
	lda (ptr1),y
	sta $0100,y
	txa
	sta (ptr1),y
	iny
	bne :-

	lda _mtarch_asm_threadsp
	sta sp
	lda _mtarch_asm_threadsp+1
	sta sp+1
	
	ldx _mtarch_asm_threadspreg
	txs

	pla
	tay
	pla
	tax
	pla
	rti

;---------------------------------------------------------------------
; Switch from thread defined by threadsp, threadstack and threadspreg.
; The kernel stack is swapped back from the threadstack, and the
; sp and spreg are restored from the local variables "kernelsp" and
; "kernelspreg".
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
 	
	lda sp
	sta _mtarch_asm_threadsp
	lda sp+1
	sta _mtarch_asm_threadsp+1

	tsx
	stx _mtarch_asm_threadspreg

	lda _mtarch_asm_threadzp
	sta ptr1
	lda _mtarch_asm_threadzp+1
	sta ptr1+1

	ldy kernelspreg             ; Determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg ; as we only need to swap the used part of the stack.
        bcc :+
        ldy _mtarch_asm_threadspreg

:	lda <__ZP_START__,y
	tax
	lda (ptr1),y
	sta <__ZP_START__,y
	txa
	sta (ptr1),y	
	iny
	cpy #zpsize
	bne :-

	lda _mtarch_asm_threadstack
	sta ptr1
	lda _mtarch_asm_threadstack+1
	sta ptr1+1

	ldy #0
:	lda $0100,y
	tax	
	lda (ptr1),y
	sta $0100,y
	txa
	sta (ptr1),y
	iny
	bne :-

	lda kernelsp
	sta sp
	lda kernelsp+1
	sta sp+1
	
	ldx kernelspreg
	txs

	cli	
	rts
;---------------------------------------------------------------------
_mtarch_asm_start:
	lda _mtarch_asm_threadzp
	sta ptr1
	lda _mtarch_asm_threadzp+1
	sta ptr1+1

	ldy #0
:	lda <__ZP_START__,y
	sta (ptr1),y	
	iny
	cpy #zpsize
	bne :-
	rts
;---------------------------------------------------------------------
