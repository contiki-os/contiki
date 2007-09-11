;;; 
;;; 
;;; ctk-conio_arch-asm.hS
;;; 
;;; Architecture depend ctk-conio implementation.
;;; This assembler source contributes basically saving for speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: ctk-conio_arch-asm.hS,v 1.1 2007/09/11 12:12:59 matsutsuka Exp $
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
	.globl	_cclear_arch
	.globl	_revers_arch
	.globl	_clip_arch
	.globl	_ctk_draw_init
	.globl	_ctk_draw_clear
	.globl	_ctk_draw_clear_window
	;; import symbols
	.globl	_info
.if 0
	.globl	_ctk_arch_window_x
	.globl	_ctk_arch_window_y
	.globl	_ctk_arch_window_h
	.globl	_ctk_arch_window_w
.endif
		
	.area	_DATA
_ctk_arch_reversed:
	.ds	1

	.area	_CODE
	
	;; ---------------------------------
	;; void cputc(char ch);
	;; Stack; ixl ixh retl reth ch
	;; AF__DEHL____
	;; return void
	;; ---------------------------------
_cputc_arch:
	push	ix
	ld	ix, #_info
	ld	a, CURSY(ix)
	cp	a, CY1(ix)		; cursy - cy1
	jr	c, _cputc_arch_ret		; cusy < cy1
	cp	a, CY2(ix)		; cursy - cy2
	jr	z, _cputc_arch_next1
	jr	nc, _cputc_arch_ret		; cursy > cy2
_cputc_arch_next1:
	cp	a, #SCREEN_HEIGHT	; cursy - SCREEN_HEIGHT
	jr	z, _cputc_arch_next2
	jr	nc, _cputc_arch_ret		; cursy > SCREEN_HEIGHT
_cputc_arch_next2:	
	ld	a, CURSX(ix)
	cp	a, #SCREEN_WIDTH	; cursx - SCREEN_WIDTH
	jr	nc, _cputc_arch_ret
	;; calculate offset
	ld	a, CURSY(ix)
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
	ld	e, CURSX(ix)
	add	hl, de
	;; putchar
	push	hl
	ld	hl, #6
	add	hl, sp
	ld	a, (hl)
	pop	hl
	ld	de, #VRAM_CHAR
	push	hl
	add	hl, de
	ld	(hl), a
	pop	hl
	;; putattr
	ld	de, #VRAM_ATTR
	add	hl, de
	ld	de, #_ctk_arch_reversed
	ld	a, (de)
	or	a
	jr	z, _cputc_arch_normal
	ld	(hl), #COLOR_REVERSED
	jr	_cputc_arch_ret
_cputc_arch_normal:
	ld	(hl), #COLOR_NORMAL
_cputc_arch_ret:
	inc	CURSX(ix)
	pop	ix
	ret
	
	;; ---------------------------------
	;; void cputs_arch(char *str);
	;; Stack; ixl ixh retl reth strl strh
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_cputs_arch:
	push	ix
	ld	hl, #4
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	b, #0		; max length
_cputs_arch_loop:
	ld	a, (de)
	or	a
	jr	z, _cputs_arch_ret
	push	bc
	push	de
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	pop	de
	pop	bc
	inc	de
	djnz	_cputs_arch_loop
_cputs_arch_ret:
	pop	ix
	ret

	;; ---------------------------------
	;; void cputsn(char *str, unsigned char length);
	;; Stack; ixl ixh retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_cputsn_arch:
	push	ix
	ld	ix, #4
	add	ix, sp
	ld	e, 0(ix)
	ld	d, 1(ix)
	ld	b, 2(ix)
	jr	_cputs_arch_loop	; take over B and DE

	;; ---------------------------------
	;; void chline(unsigned char length);
	;; Stack; ixl ixh retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_chline_arch:	
	push	ix
	ld	hl, #4
	add	hl, sp
	ld	b, (hl)
	ld	a, #CH_HOLILINE
_chline_arch_loop:
	push	bc
	push	af
	inc	sp
	call	_cputc_arch
	dec	sp
	pop	af
	pop	bc
	djnz	_chline_arch_loop
	pop	ix
	ret
	
	;; ---------------------------------
	;; void cvline(unsigned char length);
	;; Stack; ixl ixh retl reth length
	;; AFB_DEHL____
	;; return void
	;; ---------------------------------
_cvline_arch:
	push	ix
	ld	hl, #4		; subject to change
	add	hl, sp
	ld	b, (hl)	; length
	ld	a, #CH_VERTLINE
	ld	ix, #_info
_cvline_arch_loop:
	push	bc
	push	af
	inc	sp
	call	_cputc_arch
	dec	sp
	pop	af
	pop	bc
	inc	CURSY(ix)
	dec	CURSX(ix)
	djnz	_cvline_arch_loop
	pop	ix
	ret

	;; ---------------------------------
	;; void gotoxy(unsigned char x, unsigned char y)
	;; Stack; ixl ixh retl reth x y
	;; AF____HL____
	;; return void
	;; ---------------------------------
_gotoxy_arch:
	push	ix
	ld	hl, #4
	add	hl, sp
	ld	ix, #_info
	ld	a, (hl)
	ld	CURSX(ix), a
	inc	hl
	ld	a, (hl)
	ld	CURSY(ix), a
	pop	ix
	ret
	
	;; ---------------------------------
	;; void clearTo(char x)
	;; Stack; ixl ixh retl reth x
	;; AF__DEHL____
	;; return void
	;; ---------------------------------
_cclear_arch:
	push	ix
	ld	ix, #_info
	ld	hl, #4
	add	hl, sp
_cclear_arch_loop:	
	ld	a, (hl)		; x
	cp	a, CURSX(ix)
	jr	z, _cclear_arch_ret
	jr	c, _cclear_arch_ret
	ld	a, #CH_SPACE
	push	hl
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	pop	hl
	jr	_cclear_arch_loop
_cclear_arch_ret:
	ld	a, (hl)
	ld	CURSX(ix), a
	pop	ix
	ret
	
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
	ld	hl, #_ctk_arch_reversed
	ld	(hl), a
	ret

	;; ---------------------------------
	;; void clip_arch(unsigned char clip1, unsigned char clip2)
	;; Stack; retl reth clip1 clip2
	;; A_____HL____
	;; return void
	;; ---------------------------------
_clip_arch:
	push	ix
	ld	ix, #_info
	ld	hl, #4
	add	hl, sp
	ld	a, (hl)
	ld	CY1(ix), a
	inc	hl
	ld	a, (hl)
	ld	CY2(ix), a
	pop	ix
	ret

	;; ---------------------------------
	;; void ctk_draw_init(void)
	;; Stack; retl reth
	;; _F____HL____ AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_init:
	push	ix
	ld	ix, #_info
	ld	h, #SCREEN_HEIGHT
	ld	l, #0
	ld	CY1(ix), l
	ld	CY2(ix), h
	push	hl
	call	_ctk_draw_clear
	pop	hl
	pop	ix
	ret

	;; ---------------------------------
	;; ctk_draw_clear(unsigned char clipy1, unsigned char clipy2);
 	;; Stack; retl reth clipy1 clipy2
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_clear:
	ld	hl, #2
	add	hl, sp
	ld	d, (hl)		; clipy1
	inc	hl
	ld	e, (hl)		; clipy2
	ld	c, #0
_ctk_draw_clear_loop:
	ld	a, d
	cp	a, e		; d - e (clipy1 - clipy2)
	ret	nc		; ret if clipy1 >= clipy2
	push	de
	ld	b, d
	push	bc
	call	_gotoxy_arch
	pop	bc
	ld	a, #SCREEN_WIDTH
	push	af
	inc	sp
	call	_cclear_arch
	inc	sp
	pop	de
	inc	d
	jr	_ctk_draw_clear_loop

.if 0
	;; ---------------------------------
	;; void ctk_draw_clear_window(struct ctk_window *window,
	;; 		      unsigned char focus,
	;; 		      unsigned clipy1,
	;; 		      unsigned clipy2);
	;; Stack; retl reth windowl windowh focus clipy1 clipy2
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_clear_get_value:
	;; param; de=window, hl=offset
	;; return a=value
	;; A___________
	push	de
	ld	a, (hl)
	add	a, e
	ld	e, a
	ld	a, d
	adc	#0
	ld	d, a
	ld	a, (de)
	pop	de
	ret
_ctk_draw_clear_window:
	push	ix
	ld	ix, #4
	add	ix, sp
	ld	l, 3(ix)
	ld	h, 4(ix)
	push	hl
	call	_clip_arch
	pop	hl
	ld	e, 0(ix)
	ld	d, 1(ix)	; #_window
	ld	hl, #_ctk_arch_window_y
	call	_ctk_draw_clear_get_value
	ld	b, a
	inc	b
	inc	b		; window->y + 2
	
	ld	hl, #_ctk_arch_window_h
	call	_ctk_draw_clear_get_value
	ld	c, a		; window->h
	
	ld	hl, #_ctk_arch_window_x
	call	_ctk_draw_clear_get_value
	ld	h, a
	inc	h		; window->x + 1
	
	push	hl
	ld	hl, #_ctk_arch_window_w
	call	_ctk_draw_clear_get_value
	pop	hl
	ld	l, a		; window->w
	ld	a, b		; i
_ctk_draw_clear_window_loop:
	cp	a, c		; i - h
	jr	nc, _ctk_draw_clear_window_ret
	ld	d, a
	ld	e, h		; d=i, e=window->x+1
	push	af
	push	hl
	push	de
	call	_gotoxy_arch
	pop	de
	pop	hl
	ld	a, h
	add	a, l		; window->x + 1+ window->w
	push	hl
	push	af
	inc	sp
	call	_cclear_arch
	inc	sp
	pop	af
	pop	hl
	inc	a
	jr	_ctk_draw_clear_window_loop
_ctk_draw_clear_window_ret:	
	pop	ix
	ret	
	
void
ctk_draw_clear_window(struct ctk_window *window,
		      unsigned char focus,
		      unsigned char ctk_arch_clipy1,
		      unsigned char ctk_arch_clipy2) {
  clip_arch(clipy1, clipy2);
  h = window->y + 2 + window->h;

  for(i = window->y + 2; i < h; ++i) {
      gotoxy(window->x + 1, i)
      cclear(window->x + 1 + window->w);
  }
.endif
