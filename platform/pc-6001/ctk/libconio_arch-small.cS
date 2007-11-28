;;; 
;;; 
;;; libconio_arch-small.hS
;;; 
;;; Architecture depend libconio implementation.
;;; This assembler source contributes basically saving for speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: libconio_arch-small.cS,v 1.2 2007/11/28 09:38:21 matsutsuka Exp $
;;;

	;; uses preprocessor to enable definitions
#include "ctk_arch-def.h"

	;; export symbols
	.globl	_clrscr_arch
	.globl	_libputc_arch
	.globl	_libputs_arch
	
	.area	_DATA
_screen_offset::
	.ds	2

	.area	_CODE
	
_libconio_arch_small_start::

_clrscr_arch:
	ld	hl, #0
	ld	(#_screen_offset), hl
	ld	hl, #VRAM_CHAR
	ld	bc, #VRAM_CHAR + #SCREEN_HEIGHT * #SCREEN_WIDTH
_clrscr_arch_loop1:
	ld	(hl), #CH_SPACE
	inc	hl
	ld	a, h
	cp	b
	jr	nz, _clrscr_arch_loop1
	ld	a, l
	cp	c
	jr	nz, _clrscr_arch_loop1
	ld	hl, #VRAM_ATTR
	ld	bc, #VRAM_ATTR + #SCREEN_HEIGHT * #SCREEN_WIDTH
_clrscr_arch_loop2:
	ld	(hl), #COLOR_NORMAL
	inc	hl
	ld	a, h
	cp	b
	jr	nz, _clrscr_arch_loop2
	ld	a, l
	cp	c
	jr	nz, _clrscr_arch_loop2
	ret

_libputc_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
_libputc_asm::
	push	af
	ld	hl, (#_screen_offset)
	ld	bc, #SCREEN_HEIGHT * #SCREEN_WIDTH
	ld	a, b
	cp	h
	jr	nz, _libputc_arch_putc
	ld	a, c
	cp	l
	jr	z, _libputc_arch_scroll
_libputc_arch_putc:
	ld	bc, #VRAM_CHAR
	add	hl, bc
	pop	af
	ld	(hl), a
	ld	hl, #_screen_offset
	inc	(hl)
	ret	nz
	inc	hl
	inc	(hl)
	ret
_libputc_arch_scroll:
	push	de
	ld	hl, #VRAM_CHAR + #SCREEN_WIDTH
	ld	de, #VRAM_CHAR
	ld	bc, #SCREEN_HEIGHT * #SCREEN_WIDTH - #SCREEN_WIDTH
	push	bc
	push	de	
	ldir
	pop	de
	pop	hl
	push	hl
	ld	(#_screen_offset), hl
	ld	b, #SCREEN_WIDTH
	add	hl, de
_libputc_arch_scroll_loop:
	ld	(hl), #CH_SPACE
	inc	hl
	djnz	_libputc_arch_scroll_loop		
	pop	hl
	pop	de
	jr	_libputc_arch_putc

_libputs_arch:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
_libputs_arch_loop:
	ld	a, (hl)
	or	a
	ret	z
	call	_libputc_asm
	jr	_libputs_arch_loop	
_libconio_arch_small_end::
