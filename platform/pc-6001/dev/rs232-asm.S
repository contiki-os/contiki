;;; 
;;; 
;;; rs232-asm.S
;;; 
;;; PC-6001 Architecture depend RS-232C implementation,
;;; uses a bunch of BIOS functions.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: rs232-asm.S,v 1.2 2007/09/19 12:47:19 matsutsuka Exp $
;;; 
	
	;; definitions of PC-6001 BIOS routines
_rs232c_initialize	= 0x19bf
_rs232c_char_output	= 0x19e8
_char_input_sub		= 0x2642
_rs232c_interrupt_sub	= 0x0f64
	
	;; export symbols
	.globl	_rs232_arch_writeb
	.globl	_rs232_arch_poll
	.globl	_rs232_arch_init

	.area	_CODE

	;; ---------------------------------
	;; void rs232_writeb(u8_t c)
	;; Stack; retl reth c
	;; AFBC__HL____
	;; return void
	;; ---------------------------------
_rs232_arch_writeb:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
	call	_rs232c_char_output
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
	call	#_char_input_sub	; read from buffer
	ei
	jr	nz, _rs232_arch_poll_ret
	xor	a			; we have no data in the buffer
	ld	(de), a
_rs232_arch_poll_ret:
	ld	l, a
	ret

	;; ---------------------------------
	;; void rs232_init(unsigned long ubr);
	;; Stack; retl reth ubr1 ubr2 ubr3 ubr4
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_rs232_arch_init:
	ld	b, #0x4f
	call	_rs232c_initialize
	ret
