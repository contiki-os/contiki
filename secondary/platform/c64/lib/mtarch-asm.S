;---------------------------------------------------------------------       
	.importzp       ptr1
	.importzp	sp
	.import		__ZP_START__

	
	.import		_mtarch_asm_threadstack
	.import		_mtarch_asm_threadsp
	.import		_mtarch_asm_threadspreg
	.import		_mtarch_asm_threadzp	
	
	.export		_mtarch_asm_exec
	.export		_mtarch_yield
	.export		_mtarch_asm_start
	.export		_mtarch_pstart, _mtarch_pstop
	
;---------------------------------------------------------------------
.bss
kernelsp:	.res 2
kernelspreg:	.res 1		
	
oldirq:		.res 2

	zpsize = 32
.code
;---------------------------------------------------------------------	
	;; Switch to thread defined by threadsp, threadstack and threadspreg.
	;; The kernel stack is swapped onto the threadstack, and the
	;; sp and spreg are saved to the local variables "kernelsp" and
	;; "kernelspreg". Also, the zeropage variables are saved.
	
_mtarch_asm_exec:
	sei
	;; Save current stack pointer
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
:		
	lda <__ZP_START__,y
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

	ldy kernelspreg                  ; determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg              ; as we only need to swap the used part of the stack
        bcc :+
        ldy _mtarch_asm_threadspreg

:
	lda $0100,y
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

; 	jsr _mtarch_pstart	

	lda $0314
	sta oldirq
	lda $0315
	sta oldirq+1

	lda #<irq
 	sta $0314
	lda #>irq
	sta $0315

	
	pla
	tay
	pla
	tax
	pla
	
	rti

	;; Switch from thread defined by threadsp, threadstack and threadspreg.
	;; The kernel stack is swapped back from the threadstack, and the
	;; sp and spreg are restored from the local variables "kernelsp" and
	;; "kernelspreg".
yield:
	sei
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

	ldy kernelspreg                  ; determine the smallest of the two stack pointers,
        cpy _mtarch_asm_threadspreg              ; as we only need to swap the used part of the stack
        bcc :+
        ldy _mtarch_asm_threadspreg

:		
	lda <__ZP_START__,y
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
:
	lda $0100,y
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
	;; Simulate an IRQ by pushing CPU status and CPu registers
	;; onto the stack. Then call the yield function to yield the
	;; process.		
_mtarch_yield:	
	php
	pha
	txa
	pha
	tya
	pha

        tsx
         
                                ; the rts adds 1 to the PC
                                ; saved on the stack. We want
        lda $0105,x             ; the stack to look like is would
        clc                     ; do inside of an interrupt.
        adc #1                  ; (this is what the 'rts' does,
        sta $0105,x             ;  but not the 'rti')
        lda $0106,x
        adc #0
        sta $0106,x
 
	
	jmp yield		
;---------------------------------------------------------------------
_mtarch_asm_start:
	lda _mtarch_asm_threadzp
	sta ptr1
	lda _mtarch_asm_threadzp+1
	sta ptr1+1

	ldy #0
:		
	lda <__ZP_START__,y
	sta (ptr1),y	
	iny
	cpy #zpsize
	bne :-
	rts

;---------------------------------------------------------------------
irq:
	lda oldirq
	sta $0314
	lda oldirq+1
	sta $0315
	jmp yield
;---------------------------------------------------------------------	
	;; Setup preemption IRQ
_mtarch_pstart:
	sei
	lda $0314
	sta oldirq
	lda $0315
	sta oldirq+1

	lda #<irq
 	sta $0314
	lda #>irq
	sta $0315		
	cli
	rts
;---------------------------------------------------------------------
_mtarch_pstop:
	sei
	lda oldirq
	sta $0314
	lda oldirq+1
	sta $0315
	cli
	rts	
;---------------------------------------------------------------------

