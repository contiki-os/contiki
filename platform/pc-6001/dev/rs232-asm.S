;;; 
;;; 
;;; rs232-asm.S
;;; 
;;; PC-6001 Architecture depend RS-232C implementation,
;;; uses a bunch of BIOS functions.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: rs232-asm.S,v 1.3 2007/11/28 09:39:30 matsutsuka Exp $
;;; 
	
	;; definitions of PC-6001 BIOS routines
_rs232c_initialize	= 0x19bf
_buffer_out		= 0x2627
_char_input_sub		= 0x2642
_count_buffer		= 0x26a2
_rs232c_intr_vector	= 0xfa04
_subcpu_handshake_out	= 0x0e8f	
LOG = 0
CTK = 0
	;; export symbols
	.globl	_rs232_arch_writeb
	.globl	_rs232_arch_poll
	.globl	_rs232_arch_init

	.area	_CODE

.if LOG
_hex:
	.ascii	"0123456789ABCDEF"
.if CTK
 	.globl	_cputc_arch_asm
.else
	.globl	_libputc_asm
	.globl	_screen_offset
.endif
_rs232_arch_putc:		; a = data
 	ld	hl, #0
 	ld	(#_screen_offset), hl
	push	af
	srl	a
	srl	a
	srl	a
	srl	a
	call	_rs232_arch_putc_next1
	pop	af
	and	a, #0x0f
_rs232_arch_putc_next1:
	ld	hl, #_hex
_rs232_arch_putc_loop:
	or	a
	jr	z, _rs232_arch_putc_next2
	inc	hl
	dec	a
	jr	_rs232_arch_putc_loop
_rs232_arch_putc_next2:
	ld	a, (hl)
.if CTK
 	call	_cputc_arch_asm
.else
	call	_libputc_asm
.endif
	ret
.endif
	
	;; ---------------------------------
	;; void rs232_writeb(uint8_t c)
	;; Stack; retl reth c
	;; AF__D_HL____
	;; return void
	;; ---------------------------------
_rs232_arch_writeb:
	ld	hl, #2
	add	hl, sp
	ld	d, (hl)
.if LOG
	push	de
 	ld	a, #0x77	; 'w'
.if CTK	
 	call	_cputc_arch_asm
.else	
	call	_libputc_asm
.endif
	pop	de
	;; 	ld	a, d
	;; 	push	de
	;;  	call	_rs232_arch_putc
	;; 	pop	de
.endif
_rs232_arch_writeb_loop:
	in	a, (#0x81)
	and	a, #0x01
	cp	#0x01		; TxRDY? 0x01/0x81/0x85
	jr	nz, _rs232_arch_writeb_loop
	ld	a, d
	out	(#0x80), a
	ret

	;; RS-232C interrupt routine
	;; receive a byte and put it into the buffer
_rs232_arch_intr:
	push	bc
	push	af
	push	de
	push	hl
	in	a, (#0x81)
	and	a, #0x02	; RxRDY?
	jr	z, _rs232_arch_intr_next	; no input
	in	a, (#0x80)
	ld	e, a
	ld	a, #0x01
	call	_buffer_out		; buffer output A=bufno, e=data
_rs232_arch_intr_next:
	ld	a, #0x0c
	call	_subcpu_handshake_out	; sub CPU handshake output A
	call	_rs232_arch_bufchk
	pop	hl
	pop	de
	pop	af
	pop	bc
	ei
	ret
	
_rs232_arch_bufchk:
	ld      a, #0x01
	call    _count_buffer	; count buffer available bytes -> A
	cp      #0x02		; A >= 2?
	ld      a, #0x37	; Rx enable
	jr      nc, _rs232_arch_bufchk_next	; buffer available bytes >= 2
	ld      a, #0x33	; Rx disable
_rs232_arch_bufchk_next:
	out     (#0x81),a	; buf available=0x37, buf full=0x33
	ret

	;; ---------------------------------
	;; unsigned char rs232_arch_poll(unsigned char *stat);
	;; Stack; retl reth statl stath
	;; AFBCDEHL____
	;; return input byte (*stat == 0 if no input)
	;; ---------------------------------
_rs232_arch_poll:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	a, #0x01
	ld	(de), a
	di
	call	_char_input_sub		; read from buffer
	ei
	push	af
	call	_rs232_arch_bufchk	; buffer check
	pop	af
	jr	nz, _rs232_arch_poll_ret
	xor	a			; we have no data in the buffer
	ld	(de), a
	jr	_rs232_arch_poll_ret2
_rs232_arch_poll_ret:
.if LOG
	call	_libputc_asm
	;;  	call	_rs232_arch_putc
.endif
_rs232_arch_poll_ret2:
	pop	af
	ld	l, a
	ret

	;; ---------------------------------
	;; void rs232_init(unsigned long ubr);
	;; Stack; retl reth ubr1 ubr2 ubr3 ubr4
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_rs232_arch_init:
	ld	hl, #_rs232_arch_intr
	di
	ld	(#_rs232c_intr_vector), hl
	ei
	;; S2 S1 PE P  L2 L1 B2 B1
	;;  0  1  0  0  1  1  1  1
	;; 8-N-1 64x clock
	ld	b, #0x4f
	call	_rs232c_initialize
	ld	a, #0x37
	out	(#0x81), a
	ret
