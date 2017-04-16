.ifdef __dsPIC33F
.include "p33fxxxx.inc"
.endif

.ifdef __dsPIC33E
.include "p33exxxx.inc"
.endif
	
.ifdef __PIC24H
.include "p24hxxxx.inc"
.endif

	.global		__DefaultInterrupt
	.section	.text

	;; Stack will be (after pushes) currently is
	;; PC[15:0]			W15 - 34
	;; SR[0:7]:IPL3:PC[22:16]	W15 - 32
	;; RCOUNT			W15 - 30
	;; W0				W15 - 28
	;; W1				W15 - 26
	;; W2				W15 - 24
	;; W3				W15 - 22
	;; W4				W15 - 20
	;; W5				W15 - 18
	;; W6				W15 - 16
	;; W7				W15 - 14
	;; W8				W15 - 12
	;; W9				W15 - 10
	;; W10				W15 - 8
	;; W11				W15 - 6
	;; W12				W15 - 4
	;; W13				W15 - 2
	;;                		W15
__DefaultInterrupt:
	push	_RCOUNT
	mov.d	w0, [w15++]
	mov.d	w2, [w15++]
	mov.d	w4, [w15++]
	mov.d	w6, [w15++]
	mov.d	w8, [w15++]
	mov.d	w10, [w15++]
	mov.d	w12, [w15++]

	mov.w	w14, w0
	;; Now get the beginning of the stack frame to pass to the trap handler
	mov.w	#34, w1
	sub.w	w15, w1, w1
	call	_handle_trap

	mov.d	[--w15], w12
	mov.d	[--w15], w10
	mov.d	[--w15], w8
	mov.d	[--w15], w6
	mov.d	[--w15], w4
	mov.d	[--w15], w2
	mov.d	[--w15], w0
	pop	_RCOUNT
	retfie
	
