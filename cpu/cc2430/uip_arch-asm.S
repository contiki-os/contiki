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
	.module	uip_arch-asm

	;; export symbols
	.globl _uip_add32
	.globl _uip_arch_chksum
	.globl _uip_chksum
	
	;; import symbols
	.globl _uip_acc32
	.globl _uip_buf

	.area	_DATA	

	.area	_GSINIT
	
	.area	_CODE

	;; ---------------------------------
	;; void uip_add32(uint8_t *op32, uint16_t op16);
	;; Stack; retl reth op32l op32h op16l op16h
	;; ABCDEHL____
	;; return void
	;; _uip_acc32 = op32 + op16
	;; ---------------------------------
_uip_add32_start::
_uip_add32:
	;; HL = #_op32l
	ld	hl, #2
	add	hl, sp

	;; DE = #(_op32)
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl

	;; BC = op16
	ld	c, (hl)
	inc	hl
	ld	b, (hl)

	;; HL = #(_op32) + 3
	ld	hl, #3	
	add	hl, de
	
	;; DE = #_uip_acc32 + 3
	ld	de, #_uip_acc32 + 3

	;; uip_acc32[3] = op32[3] + op16l;
	ld	a, (hl)
	add	a, c
	ld	(de), a
	
	;; uip_acc32[2] = op32[2] + op16h + carry;
	dec	hl
	dec	de
	ld	a, (hl)
	adc	a, b
	ld	(de), a
	jr	nc, _uip_add32_nocarry1

	;; uip_acc32[1]
	dec	hl
	dec	de
	ld	a, (hl)
	inc	a
	ld	(de), a
	jr	nz, _uip_add32_nocarry0

	;; uip_acc32[0]
	dec	hl
	dec	de
	ld	a, (hl)
	inc	a
	ld	(de), a
	ret
_uip_add32_nocarry1:
	;; uip_acc32[1]
	dec	hl
	dec	de
	ld	a, (hl)
	ld	(de), a

_uip_add32_nocarry0:
	;; uip_acc32[0]
	dec	hl
	dec	de
	ld	a, (hl)
	ld	(de), a
	ret			
_uip_add32_end::		
	
	;; ---------------------------------
	;; static uint16_t chksum(uint16_t sum, const uint8_t *data, uint16_t len)
	;; Stack; retl reth suml sumh datal datah lenl lenh
	;; ABCDEHL____
	;; return HL
	;; ---------------------------------
_uip_arch_chksum_start::	
_uip_arch_chksum:
	push	ix
	;; IX = #_suml
	ld	ix, #4
	add	ix, sp
	;; BC = sum
	ld	c, 0(ix)
	ld	b, 1(ix)
	;; DE = #data
	ld	e, 2(ix)
	ld	d, 3(ix)
	
	;; (lenl, lenh) <- dataptr + len - 1 (last address)
	;; (len) + DE - 1 -> (len)
	ld	l, 4(ix)
	ld	h, 5(ix)
	add	hl, de
	dec	hl
	ld	4(ix), l
	ld	5(ix), h

_uip_arch_chksum_loop:
	;; compare HL(last address) and DE(dataptr)
	;; HL - DE
	;; if (HL < DE) C,NZ else if (HL = DE) NC,Z=1 otherwise NC,NZ
	;;  HL = last address, DE = current pointer
	ld	l, 4(ix)
	ld	h, 5(ix)
	
	ld      a, h
	sub     d
	jr	nz, _uip_arch_chksum_compared
	ld      a, l
	sub     e
	;; if (last address == dataptr) _uip_arch_chksum_loop_exit_add_trailing
	jr	z, _uip_arch_chksum_loop_exit_add_trailing
_uip_arch_chksum_compared:
	;; if (last address > dataptr) _uip_arch_chksum_loop_exit
	jr	c, _uip_arch_chksum_loop_exit
	;; bc = dataptr[0],dataptr[1] + bc
	ld	a, (de)
	ld	h, a
	inc	de
	ld	a, (de)
	ld	l, a
	push	hl
	add	hl, bc
	inc	de
	ld	b, h
	ld	c, l
	;; HL = t
	pop	hl
	;; BC - HL
	;; if (sumBC < tHL) sum++
	ld	a, b
	sub	h
	jr	nz, _uip_arch_chksum_compared_t
	ld	a, c
	sub	l
_uip_arch_chksum_compared_t:
	jr	nc, _uip_arch_chksum_nocarry_t
	inc	bc
_uip_arch_chksum_nocarry_t:
	jr	_uip_arch_chksum_loop
_uip_arch_chksum_loop_exit_add_trailing:
	;; HL = last address
	;; bc = bc + (last address)<<8
	ld	a, b
	add	a, (hl)
	ld	b, a
	jr	nc, _uip_arch_chksum_loop_exit
	inc	bc
_uip_arch_chksum_loop_exit:
	ld	l, c
	ld	h, b
	pop	ix
	ret
_uip_arch_chksum_end::	
	
	;; ---------------------------------
	;; uint16_t uip_chksum(void);
	;; Stack; retl reth datal datah lenl lenh
	;; ABCDEHL____
	;; return HL
	;; return htons(chksum(0, (uint8_t *)data, len));
	;; ---------------------------------
_uip_chksum_start::
_uip_chksum:
	ld	hl, #5
	add	hl, sp
	;; HL indicates #_lenh
	ld	b, #2
_uip_chksum_loop:
	ld	d, (hl)
	dec	hl
	ld	e, (hl)
	dec	hl
	push	de
	djnz	_uip_chksum_loop
	ld	bc, #0
	push	bc
	call	_uip_arch_chksum
	pop	af
	pop	af
	pop	af
	;; convert to BIG ENDIAN (htons)
	ld	a, l
	ld	l, h
	ld	h, a
	ret
_uip_chksum_end::
