;;; 
;;; 
;;; libconio_arch-asm.hS
;;; 
;;; Architecture depend libconio implementation.
;;; This assembler source contributes basically saving for speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: libconio_arch-asm.cS,v 1.1 2007/09/19 12:46:15 matsutsuka Exp $
;;;

	;; uses preprocessor to enable definitions
#include "ctk_arch-def.h"

	;; export symbols
	.globl	_cputc_arch
	.globl	_cputs_arch
	.globl	_cputsn_arch
	.globl	_chline_arch
	.globl	_cvline_arch
	.globl	_gotoxy_arch
	.globl	_clearto_arch
	.globl	_revers_arch
	.globl	_clip_arch
	.globl	_wherex_arch
	.globl	_drawbox_arch
	.globl	_drawicon_arch
	.globl	_clearbox_arch
	.globl	_newline_arch
	
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
.if 0
	;; input: B = y, C = x
	;; output: HL = offset
	;; ______HL____
_calc_offset:
	push	af
	push	de
	ld	a, b
	ld	hl, #0
	ld	de, #SCREEN_WIDTH
_calc_offset_loop:
	or	a
	jr	z, _calc_offset_calcx
	add	hl, de
	dec	a
	jr	_calc_offset_loop
_calc_offset_calcx:
	ld	d, #0
	ld	e, c
	add	hl, de
	pop	de
	pop	af
	ret
.endif	
	;; ---------------------------------
	;; void cputc(char ch);
	;; Stack; retl reth ch
	;; AFBCDEHL____
	;; return A=1 if output is succeeded, A=0 if not
	;; ---------------------------------
_cputc_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
	;; ------------
	;; A = char
	;; AFBCDEHL____
	;; ------------
_cputc_arch_asm:
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
.if 0	
	call	_calc_offset
.else	
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
.endif
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
_cputs_arch:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	b, #SCREEN_WIDTH		; max length
	;; ------------
	;; B = nchars
	;; DE = str
	;; AFB_DEHL____
	;; ------------
_cputs_arch_loop:
	ld	a, (de)
	or	a
	ret	z
	push	bc
	push	de
	call	_cputc_arch_asm
	pop	de
	pop	bc
	inc	de
	djnz	_cputs_arch_loop
	ret

	;; ---------------------------------
	;; void cputsn(char *str, unsigned char length);
	;; Stack; retl reth strl strh length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_cputsn_arch:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl
	ld	b, (hl)
	jr	_cputs_arch_loop	; take over B and DE
	
	;; ---------------------------------
	;; void chline(unsigned char length);
	;; Stack; retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_chline_arch:
	ld	hl, #2
	add	hl, sp
	ld	b, (hl)
	;; ------------
	;; B = length
	;; AFB_DEHL____
	;; ------------
_chline_arch_asm:
	ld	a, #CH_HOLILINE
_chline_arch_loop:
	push	bc
	push	af
	call	_cputc_arch_asm
	pop	af
	pop	bc
	djnz	_chline_arch_loop
	ret
		
	;; ---------------------------------
	;; void cvline(unsigned char length);
	;; Stack; retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_cvline_arch:
	ld	hl, #2
	add	hl, sp
	ld	b, (hl)		; length
	;; ------------
	;; B = length
	;; AFB_DEHL____
	;; ------------
_cvline_arch_asm:
	ld	a, #CH_VERTLINE
	ld	hl, (#_screen_cursor) ; H=y, L=x
_cvline_arch_loop:
	push	hl
	push	bc
	push	af
	call	_cputc_arch_asm
	pop	af
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
_gotoxy_arch:
	ld	hl, #2
	add	hl, sp
	ld	d, (hl)		; x
	inc	hl
	ld	e, (hl)		; y
	;; ------------
	;; D = x, E = y
	;; ______HL____
	;; ------------
_gotoxy_arch_asm:
	ld	l, d		; L=x
	ld	h, e		; H=y
	ld	(#_screen_cursor), hl
	ret
	
	;; ---------------------------------
	;; void clearto_arch(unsigned char to)
	;; Stack; retl reth to
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_clearto_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
_clearto_arch_loop:
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
	jr	_clearto_arch_loop

	;; ---------------------------------
	;; void revers_arch(unsigned char reverse)
	;; Stack; retl reth reverse
	;; A_____HL____
	;; return void
	;; ---------------------------------
_revers_arch:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
	ld	(#_screen_reversed), a
	ret

	;; ---------------------------------
	;; void clip_arch(unsigned char clip1, unsigned char clip2)
	;; Stack; retl reth clip1 clip2
	;; ____________
	;; return void
	;; ---------------------------------
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

	;; ---------------------------------
	;; unsigned char wherex_arch()
	;; Stack; retl reth
	;; A_____HL____
	;; return void
	;; ---------------------------------
_wherex_arch:
	ld	a, (#_screen_cursor)
	ld	l, a
	ret
	
	;; ---------------------------------
	;; void drawbox_arch(unsigned char x, y, w, h)
	;; Stack; retl reth x y w h
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_drawbox_arch:
	ld	hl, #2
	add	hl, sp
	ld	d, (hl)		; x
	inc	hl
	ld	e, (hl)		; y
	inc	hl
	ld	c, (hl)		; w
	inc	hl
	ld	b, (hl)		; h
	inc	e
	call	_gotoxy_arch_asm	; (x, y + 1)
	dec	e
	push	de
	push	bc
	call	_cvline_arch_asm
	ld	a, #CH_LLCORNER
	call	_cputc_arch_asm
	pop	bc
	push	bc		; Stack; BC DE
	ld	b, c		; w
	call	_chline_arch_asm
	ld	a, #CH_LRCORNER
	call	_cputc_arch_asm
	pop	bc
	pop	de
	call	_gotoxy_arch_asm	; (x, y)
	push	de
	push	bc		; Stack; BC DE
	ld	a, #CH_ULCORNER
	call	_cputc_arch_asm
	pop	bc
	push	bc		; Stack; BC DE
	ld	b, c
	call	_chline_arch_asm
	ld	a, #CH_URCORNER
	call	_cputc_arch_asm
	pop	bc		; B = h
	pop	de
	ld	a, d
	inc	a
	add	a, c
	ld	d, a		; D = x + 1 + w
	inc	e		; E = y + 1
	call	_gotoxy_arch_asm
	call	_cvline_arch_asm
	ret

.if CTK_CONF_ICONS
	;; ---------------------------------
	;; void drawicon_arch(char* textmap, char *title);
	;; Stack; retl reth textmapl textmaph titlel titleh
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_drawicon_arch:
	ld	hl, #_screen_cursor
	ld	b, (hl)		; x
	inc	hl
	ld	c, (hl)		; y
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)		; DE = textmap
	ld	a, e
	or	d
	ret	z		; if (textmap == NULL) return;
	ld	a, #3		; loop counter
_drawicon_arch_loop:
	push	af
	push	de
	ld	d, b
	ld	e, c
	call	_gotoxy_arch_asm
	pop	de
	push	bc
	ld	b, #3
	call	_cputs_arch_loop ; DE = DE + 3
	pop	bc
	pop	af
	inc	c		; y++
	dec	a
	jr	z, _drawicon_arch_title
	jr	_drawicon_arch_loop
	;; B = x, C = y
_drawicon_arch_title:
	ld	hl, #4
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)		; DE = title
	ld	h, d
	ld	l, e
	xor	a
	push	de
	ld	d, #0
_drawicon_arch_title_strlen:
	ld	a, (hl)
	or	a
	jr	z, _drawicon_arch_title_setpos
	inc	hl
	inc	d
	jr	_drawicon_arch_title_strlen
_drawicon_arch_title_setpos:
	ld	h, d		; H = len
	ld	a, d
	pop	de
	add	a, b
	cp	a, #SCREEN_WIDTH ;  x + len - SCREEN_WIDTH
	jr	c, _drawicon_arch_title_show
	ld	a, #SCREEN_WIDTH
	sub	h
	ld	b, a
	;; B = x, C = y, DE = title, H = len
_drawicon_arch_title_show:
	push	de
	ld	d, b
	ld	e, c
	call	_gotoxy_arch_asm
	pop	de
	ld	b, #SCREEN_WIDTH
	call	_cputs_arch_loop
	ret
.endif

	;; ---------------------------------
	;; void clearbox_arch(unsigned char x, y, w, h)
	;; Stack; retl reth x y w h
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_clearbox_arch:
	ld	hl, #2
	add	hl, sp
	ld	d, (hl)		; x
	inc	hl
	ld	e, (hl)		; y
	inc	hl
	ld	c, (hl)		; w
	inc	hl
	ld	b, (hl)		; h
	ld	a, d
	add	a, c
	ld	c, a		; to_x
_clearbox_arch_loop:
	call	_gotoxy_arch_asm	; (x, y + 1)
	push	de
	push	bc
	ld	a, c
	call	_clearto_arch_loop
	pop	bc
	pop	de
	inc	e
	djnz	_clearbox_arch_loop
	ret

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
	
_libconio_arch_asm_end::
