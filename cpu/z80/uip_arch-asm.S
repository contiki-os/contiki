;;; 
;;; 
;;; uip_arch-asm.S
;;; 
;;; \file
;;; 	Z80 architecture-depend uip module
;;; 	for calculating checksums
;;; 
;;; \author
;;; 	Takahide Matsutsuka <markn@markn.org>
;;; 
	
	.globl _uip_add32
	.globl _uip_acc32
	
	.area _GSINIT

	.area _DATA	

	.area	_CODE

	;; ---------------------------------
	;; void uip_add32(u8_t *op32, u16_t op16);
	;; Stack; retl reth op32l op32h op16l op16h
	;; ABCDEHL____
	;; return void
	;; _uip_acc32 = op32 + op16
	;; ---------------------------------
_uip_add32_start::
_uip_add32:
	ld	hl,#2
	add	hl,sp
	;; HL indicates #_op32l
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	;; BC indicates op16
	ld	l,e
	ld	h,d
	;; HL indicates #_op32
	ld	de,#_uip_acc32
	;; DE indicates #_uip_acc32
	;; uip_acc32[0] = op32[0] + op16l;
	ld	a,(hl)
	add	a,c
	ld	(de),a
	inc	hl
	inc	de
	;; uip_acc32[1] = op32[1] + op16h + carry;
	ld	a,(hl)
	adc	a,b
	ld	(de),a
	inc	hl
	inc	de
	jr	nc,_uip_add32_exit
	ld	a,(hl)
	inc	a
	ld	(de),a
	inc	hl
	inc	de
	jr	nc,_uip_add32_exit
	ld	a,(hl)
	inc	a
	ld	(de),a
	ret
_uip_add32_exit:	
	ld	a,(hl)
	ld	(de),a
	inc	hl
	inc	de
	ld	a,(hl)
	ld	(de),a
	ret			
_uip_add32_end::		
	;; ---------------------------------
	;; u16_t uip_chksum(void);
	;; Stack; retl reth suml sumh datal datah lenl lenh
	;; ABCDEHL____
	;; return HL
	;; _uip_acc32 = op32 + op16
	;; ---------------------------------
