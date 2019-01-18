;;; 
;;; 
;;; isprint-arch.S
;;; 
;;; Architecture depend isprint function.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: isprint_arch.S,v 1.1 2007/09/11 12:12:59 matsutsuka Exp $
;;; 
	.globl _isprint_arch
	.area _GSINIT

	;; unsigned long clock_time();
	;; return long value as (DE, BC)

	;; this function is identify if a given
	;; character is printable on PC-6001
	.area _CODE
_isprint_arch:
	;; ---------------------------------
	;; char isprint (unsigned char ch);
	;; Stack; retl reth ch
	;; A_____HL____
	;; return l
	;; ---------------------------------
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
	or	a
	jr	z, #_isprint_arch_ret_false
	cp	#0xfe
	jr	nc, #_isprint_arch_ret_false
	ld	l, #1
	ret
_isprint_arch_ret_false:
	ld	l, #0
	ret
