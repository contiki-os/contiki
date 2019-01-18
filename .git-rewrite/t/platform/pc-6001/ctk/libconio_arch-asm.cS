;;; 
;;; 
;;; libconio_arch-asm.hS
;;; 
;;; Architecture depend libconio implementation.
;;; This assembler source contributes basically saving for speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: libconio_arch-asm.cS,v 1.2 2007/09/29 04:10:00 matsutsuka Exp $
;;;

	;; uses preprocessor to enable definitions
#include "ctk_arch-def.h"

	;; export symbols
.if LIBCONIO_EXPORT
	.globl	_cputc_arch
	.globl	_cputs_arch
	.globl	_cputsn_arch
	.globl	_chline_arch
	.globl	_cvline_arch
	.globl	_gotoxy_arch
	.globl	_clearto_arch
	.globl	_revers_arch
	.globl	_clip_arch
.endif
	.globl	_wherex_arch
	.globl	_wherey_arch
	
	.area	_DATA
_screen_cursor:
	.ds	1		; x
	.ds	1		; y
_screen_clips:
	.ds	1		; clip y1
	.ds	1		; clip y2
_screen_reversed:
	.ds	1

	.area	_CODE
	
_libconio_arch_asm_start::
	;; ---------------------------------
	;; void cputc(char ch);
	;; Stack; retl reth ch
	;; AFBCDEHL____
	;; return A=1 if output is succeeded, A=0 if not
	;; ---------------------------------
.if LIBCONIO_EXPORT
_cputc_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
.endif				; LIBCONIO_EXPORT
	;; ------------
	;; A = char
	;; AFBCDEHL____
	;; ------------
_cputc_arch_asm::
	push	af	
	ld	bc, (#_screen_cursor)	;B=y, C=x
	ld	de, (#_screen_clips)	;D=cy2, E=cy1
	ld	a, b
	cp	a, e
	jr	c, _cputc_arch_ret_false	; if (cursy < cy1)
	cp	a, d
	jr	z, _cputc_arch_next1
	jr	nc, _cputc_arch_ret_false	; if (cursy > cy2)
_cputc_arch_next1:
	cp	a, #SCREEN_HEIGHT		; cursy - SCREEN_HEIGHT
	jr	z, _cputc_arch_next2
	jr	nc, _cputc_arch_ret_false	; if (cursy > SCREEN_HEIGHT)
_cputc_arch_next2:	
	ld	a, c
	cp	a, #SCREEN_WIDTH		; cursx - SCREEN_WIDTH
	jr	nc, _cputc_arch_ret_false	; if (cursx >= SCREEN_WIDTH)
	;; calculate offset
	ld	a, b			; A=y
	ld	hl, #0
	ld	de, #SCREEN_WIDTH
_cputc_arch_loop:
	or	a
	jr	z, _cputc_arch_calcx
	add	hl, de
	dec	a
	jr	_cputc_arch_loop
_cputc_arch_calcx:
	ld	d, #0
	ld	e, c
	add	hl, de

	;; putchar
	pop	af
	ld	de, #VRAM_CHAR
	push	hl
	add	hl, de
	ld	(hl), a
	pop	hl
	;; putattr
	ld	de, #VRAM_ATTR
	add	hl, de
	ld	a, (#_screen_reversed)
	or	a
	jr	z, _cputc_arch_normal
	ld	(hl), #COLOR_REVERSED
	jr	_cputc_arch_ret
_cputc_arch_normal:
	ld	(hl), #COLOR_NORMAL
	ld	a, #0x01
_cputc_arch_ret:
	ld	hl, #_screen_cursor
	inc	(hl)
	ret
_cputc_arch_ret_false:
	pop	af
	xor	a
	jr	_cputc_arch_ret
	
	;; ---------------------------------
	;; void cputs_arch(char *str);
	;; Stack; retl reth strl strh
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
.if 1
_cputs_arch::
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
.endif				; LIBCONIO_EXPORT
	
	;; ------------
	;; DE = str
	;; AFB_DEHL____
	;; ------------
_cputs_arch_asm::
	ld	b, #SCREEN_WIDTH		; max length
	jr	_cputsn_arch_asm	; take over B and DE

	;; ---------------------------------
	;; void cputsn(char *str, unsigned char length);
	;; Stack; retl reth strl strh length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_cputsn_arch:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl
	ld	b, (hl)
.endif				; LIBCONIO_EXPORT
	
	;; ------------
	;; B = nchars
	;; DE = str
	;; AFB_DEHL____
	;; ------------
_cputsn_arch_asm::
	ld	a, (de)
	or	a
	ret	z
	push	bc
	push	de
	call	_cputc_arch_asm
	pop	de
	pop	bc
	inc	de
	djnz	_cputsn_arch_asm
	ret
	
	;; ---------------------------------
	;; void chline(unsigned char length);
	;; Stack; retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_chline_arch:
	ld	hl, #2
	add	hl, sp
	ld	b, (hl)
	;; ------------
	;; B = length
	;; AFB_DEHL____
	;; ------------
.endif
_chline_arch_asm::
	ld	a, #CH_HOLILINE
	push	bc
	call	_cputc_arch_asm
	pop	bc
	djnz	_chline_arch_asm
	ret
		
	;; ---------------------------------
	;; void cvline(unsigned char length);
	;; Stack; retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_cvline_arch:
	ld	hl, #2
	add	hl, sp
	ld	b, (hl)		; length
.endif				; LIBCONIO_EXPORT
	;; ------------
	;; B = length
	;; AFB_DEHL____
	;; ------------
_cvline_arch_asm::
	ld	hl, (#_screen_cursor) ; H=y, L=x
_cvline_arch_loop:
	ld	a, #CH_VERTLINE
	push	hl
	push	bc
	call	_cputc_arch_asm
	pop	bc
	pop	hl
	inc	h
	ld	(#_screen_cursor), hl
	djnz	_cvline_arch_loop
	ret

	;; ---------------------------------
	;; void gotoxy(unsigned char x, unsigned char y)
	;; Stack; retl reth x y
	;; _F__DEHL____
	;; return void
	;; ---------------------------------
.if 1
_gotoxy_arch::
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)		; y
	inc	hl
	ld	d, (hl)		; x
	ld	l, e		; L=x
	ld	h, d		; H=y
.endif				; LIBCONIO_EXPORT

	;; ------------
	;; H = x, L = y
	;; ____________
	;; ------------
_gotoxy_arch_asm::
	ld	(#_screen_cursor), hl
	ret
	
	;; ---------------------------------
	;; void clearto_arch(unsigned char to)
	;; Stack; retl reth to
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_clearto_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
.endif				; LIBCONIO_EXPORT
	;; ------------
	;; A = to
	;; AFBCDEHL____
	;; ------------
_clearto_arch_asm::
	ld	hl, #_screen_cursor
	cp	(hl)		; to - cursx
	ret	z
	ret	c
	push	af
	ld	a, #CH_SPACE
	call	_cputc_arch_asm
	or	a
	pop	af
	ret	z
	jr	_clearto_arch_asm

	;; ---------------------------------
	;; void revers_arch(unsigned char reverse)
	;; Stack; retl reth reverse
	;; A_____HL____
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_revers_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
.endif				; LIBCONIO_EXPORT
	;; A = reverse
	;; ____________
_revers_arch_asm::
	ld	(#_screen_reversed), a
	ret

	;; ---------------------------------
	;; void clip_arch(unsigned char clip1, unsigned char clip2)
	;; Stack; retl reth clip1 clip2
	;; ____________
	;; return void
	;; ---------------------------------
.if LIBCONIO_EXPORT
_clip_arch:
	push	af
	push	hl
	push	de
	ld	hl, #0x08
	add	hl, sp
	ld	de, #_screen_clips
	ld	a, (hl)
	ld	(de), a
	inc	hl
	inc	de
	ld	a, (hl)
	ld	(de), a
	pop	de
	pop	hl
	pop	af
	ret	
.endif	
	;; ---------------------------------
	;; L = clipy1, H = clipy2
	;; ____________
	;; ---------------------------------
_clip_arch_asm::
	ld	(#_screen_clips), hl
	ret	

	;; ---------------------------------
	;; unsigned char wherex_arch()
	;; Stack; retl reth
	;; A______L____
	;; return x
	;; ---------------------------------
_wherex_arch:
	ld	a, (#_screen_cursor)
	ld	l, a
	ret
	
	;; ---------------------------------
	;; unsigned char wherey_arch()
	;; Stack; retl reth
	;; A______L____
	;; return y
	;; ---------------------------------
_wherey_arch:
	ld	a, (#_screen_cursor + 1)
	ld	l, a
	ret
	
.if 0
_newline_arch:
	ld	hl, #_screen_cursor
	xor	a
	ld	(hl), a
	inc	hl
	ld	a, (hl)
	inc	a
	cp	a, #SCREEN_HEIGHT
	jr	nc, _newline_arch_scroll
	ld	(hl), a
	ret
_newline_arch_scroll:
	;; TODO: attr and specific impl
	dec	a
	ld	(hl), a
	ld	hl, #VRAM_CHAR + #SCREEN_WIDTH
	ld	de, #VRAM_CHAR
	ld	bc, #SCREEN_HEIGHT * #SCREEN_WIDTH - #SCREEN_WIDTH
	ldir
	ret
.endif
	
_libconio_arch_asm_end::
