;;; 
;;; 
;;; ctk-conio_arch-asm.hS
;;; 
;;; Architecture depend ctk-conio implementation.
;;; This assembler source contributes saving speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: ctk-conio_arch-asm.cS,v 1.1 2007/09/19 12:46:15 matsutsuka Exp $
;;;

	;; uses preprocessor to enable definitions
#include "ctk_arch-def.h"

	;; export symbols
	.globl	_ctk_draw_init
	.globl	_ctk_draw_clear
	.globl	_ctk_draw_clear_window
	.globl	_ctk_draw_window
	.globl	_ctk_draw_dialog
	.globl	_ctk_draw_widget
	.globl	_ctk_draw_windowtitle_height
	.globl	_ctk_draw_menus
	.globl	_ctk_draw_width
	.globl	_ctk_draw_height

	;; import symbols
	.globl	_cputc_arch
	.globl	_cputs_arch
	.globl	_cputsn_arch
	.globl	_gotoxy_arch
	.globl	_clip_arch
	.globl	_clearto_arch
	.globl	_wherex_arch
	.globl	_clearbox_arch
	.globl	_drawbox_arch
.if CTK_CONF_ICONS
	.globl	_drawicon_arch
.endif				; CTK_CONF_ICONS
	.globl	_revers_arch
	.globl	_chline_arch
	
	;; offsets
	.globl	_off_window_x
	.globl	_off_window_y
	.globl	_off_window_h
	.globl	_off_window_w
	.globl	_off_window_inactive
	.globl	_off_window_active
	.globl	_off_window_next
	.globl	_off_window_focused

	.globl	_off_widget_x
	.globl	_off_widget_y
	.globl	_off_widget_w
	.globl	_off_widget_h
	.globl	_off_widget_type
	.globl	_off_widget_window
	.globl	_off_widget_label_text
	.globl	_off_widget_button_text
	.globl	_off_widget_textentry_text
	.globl	_off_widget_textentry_xpos
	.globl	_off_widget_textentry_ypos
	.globl	_off_widget_textentry_state
.if CTK_CONF_ICONS
	.globl	_off_widget_icon_title
	.globl	_off_widget_icon_textmap
.endif				; CTK_CONF_ICONS
.if CTK_CONF_MENUS
	.globl	_off_menu_title
	.globl	_off_menu_active
	.globl	_off_menu_nitems
	.globl	_off_menu_items
	.globl	_off_menu_next
	.globl	_off_menuitem_title
	.globl	_size_menuitem
	.globl	_off_menus_open
	.globl	_off_menus_menus
	.globl	_off_menus_desktopmenu
.endif				; CTK_CONF_MENUS

	.area	_DATA

	.area	_CODE
_ctk_draw_windowtitle_height:
	.db	1
	
_ctk_conio_arch_asm_start::
	;; ---------------------------------
	;; void ctk_draw_init(void)
	;; Stack; retl reth
	;; _F____HL____ AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_init:
	ld	d, #SCREEN_HEIGHT
	ld	e, #0
	jr	_ctk_draw_clear_asm

	;; ---------------------------------
	;; ctk_draw_clear(unsigned char clipy1, unsigned char clipy2);
 	;; Stack; retl reth clipy1 clipy2
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_clear:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)		; clipy1
	inc	hl
	ld	d, (hl)		; clipy2
	;; E = clip1, D = clip2
_ctk_draw_clear_asm:
	push	de
	call	_clip_arch
	pop	de
	ld	a, d
	sub	e
	ld	h, a			; height
	ld	l, #SCREEN_WIDTH	; width
	push	hl
	ld	l, #0		; x
	ld	h, e		; y
	push	hl
	call	_clearbox_arch
	pop	hl
	pop	hl
	ret

_get_offset:
	;; BC = offset address
	;; HL = base address
	;; return DE = data
	;; ___DE______
	push	af
	push	hl
	ld	a, (bc)
	add	a, l
	ld	l, a
	ld	a, h
	adc	#0
	ld	h, a
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	pop	hl
	pop	af
	ret

	;; ---------------------------------
	;; ctk_draw_clear_window(struct ctk_window *window,
	;; 		      unsigned char focus,
	;; 		      unsigned char clipy1,
	;; 		      unsigned char clipy2)
 	;; Stack; retl reth winl winh focus clipy1 clipy2
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_clear_window:
	;;   clip_arch(clipy1, clipy2);
	ld	hl, #5
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	push	de
	call	_clip_arch
	pop	de
	;;   clearbox_arch(window->x + 1, window->y + 2, window->w, window->h);
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	h, d
	ld	l, e		; HL = window

	ld	bc, #_off_window_h
	call	_get_offset	; E = window->h
	ld	a, e
	ld	bc, #_off_window_w
	call	_get_offset	; E = window->w
	ld	d, a
	push	de
	ld	bc, #_off_window_y
	call	_get_offset	; E = window->y
	ld	a, e
	inc	a
	inc	a
	ld	bc, #_off_window_x
	call	_get_offset	; E = window->x
	inc	e
	ld	d, a		; D = window->y + 2, E = window->x + 1
	push	de
	call	_clearbox_arch
	pop	de
	pop	de
	ret
	
	;; ---------------------------------
	;; void ctk_draw_dialog(struct ctk_window *dialog)
 	;; Stack; retl reth dialogl dialogh
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_dialog:
	;;   ctk_draw_clear_window(dialog, 0, 0, SCREEN_HEIGHT)
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	h, #SCREEN_HEIGHT
	ld	l, #0
	push	hl
	xor	a
	push	af
	inc	sp
	push	de
	call	_ctk_draw_clear_window
	pop	de
	inc	sp
	pop	hl

	;;   draw_window_asm(dialog, CTK_FOCUS_DIALOG)
	ld	a, #_CTK_FOCUS_DIALOG
	call	_draw_window_asm
	ret
	
	;; ---------------------------------
	;; void ctk_draw_window(struct ctk_window *window, unsigned char focus,
	;; 		     unsigned char clipy1, unsigned char clipy2,
	;; 		     unsigned char draw_borders)
 	;; Stack; retl reth winl winh focus cy1 cy2 borders
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_window:
	;; clip_arch(clipy1, clipy2);
	ld	hl, #5
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	push	de
	call	_clip_arch
	pop	de
	
	;;   DE = window
	;;   A = focus = focus & CTK_FOCUS_WINDOW
	;;   draw_window_asm(window, focus);
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl
	ld	a, (hl)
	and	a, #_CTK_FOCUS_WINDOW
	call	_draw_window_asm
	ret

	;; ---------------------------------
	;; void draw_window_asm
	;;	(struct ctk_window *window, unsigned char focus)
	;; DE = window, A = focus
 	;; Stack; retl reth
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_draw_window_asm:
	;;  drawbox_arch(window->x, window->y + 1, window->w, window->h);
	push	af		; focus
	ld	h, d
	ld	l, e		; HL = window
	push	hl		; window
	ld	bc, #_off_window_h
	call	_get_offset
	ld	a, e
	ld	bc, #_off_window_w
	call	_get_offset
	ld	d, a
 	push	de		; D = window->h, E = window->w
	ld	bc, #_off_window_y
	call	_get_offset
	ld	a, e
	inc	a
	ld	bc, #_off_window_x
	call	_get_offset
	ld	d, a
	push	de		; D = window->y + 1, E = window->x
	call	_drawbox_arch
	pop	de		; pop yx
	pop	bc		; pop hw, Stack; window focus
	pop	hl		; HL = window; Stack; focus
	inc	d		; D = x = window->y + 2
	inc	e		; E = y = window->x + 1
	push	de		; Stack; yx focus
	
	;; for(w = window->inactive; w != NULL; w = w->next) {
	ld	bc, #_off_window_inactive
	call	_get_offset	; DE = w = window->inactive
	pop	bc		; yx
	pop	af		; focus
	push	hl
	;; de = widget, bc = yx, a = focus, hl = window; Stack; window
_ctk_draw_window_asm_loop1:	; draw inactive widgets
	;; HL = e4cd
	push	af		; Stack; focus window
	ld	a, d
	or	e
	jr	z, _draw_window_asm_next1 ; if (DE == NULL)
	pop	af		; A = focus, Stack; window
	;; bc = yx, de = w, a = focus
	;; draw_widget(w, x, y, focus);
	call	_draw_widget_asm
	push	bc		; Stack; BC=xy HL=window
	ld	h, d
	ld	l, e		; HL = w
	ld	bc, #_off_window_next
	call	_get_offset	; DE = window->next
	pop	bc		; bc = yx, Stack; window
	jr	_ctk_draw_window_asm_loop1
	;; for(w = window->active; w != NULL; w = w->next)
	;; HL = window, BC = xy, A = focus; Stack; focus
_draw_window_asm_next1:		; Stack; focus window
	pop	af		; A = focus
	pop	hl		; HL = window; Stack null
	push	bc		; Stack yx
	;; calc window->focused
	ld	bc, #_off_window_focused	
	call	_get_offset	; DE = window->focused
	push	de		; Stack focused yx
	ld	bc, #_off_window_active
	call	_get_offset	; DE = window->active
	pop	hl		; HL = window->focused, Stack; yx
	pop	bc		; BC = yx, Stack; null
	push	af		; Stack; focus
	push	hl		; Stack; focused focus

	;; BC = yx, DE = widget, Stack; focused focus
_ctk_draw_window_asm_loop2:	; draw active widgets
	ld	a, d		; DE = w
	or	e
	jr	z, _draw_window_asm_next2
	;; if (w == window->focused) focus |= CTK_FOCUS_WIDGET;
	pop	hl		; HL = window->focused; Stack focus
	ld	a, h
	cp	d
	jr	nz, _draw_window_asm_nofocus
	ld	a, l
	cp	e
	jr	nz, _draw_window_asm_nofocus
	pop	af
	push	af
	or	a, #_CTK_FOCUS_WIDGET
	jr	_draw_window_asm_draw
_draw_window_asm_nofocus:
	pop	af
	push	af
	;; A = wfocus, BC = yx de = widget hl = focused
_draw_window_asm_draw:		; Stack; focus
	;; draw_widget(w, posx, posy, wfocus);
	call	_draw_widget_asm
	
	push	hl		; Stack; focused focus
	push	bc		; Stack; yx focused focus
	ld	h, d
	ld	l, e	
	ld	bc, #_off_window_next
	call	_get_offset	; DE = w->next
	pop	bc		; BC = yx, Stack; focused focus
	jr	_ctk_draw_window_asm_loop2
_draw_window_asm_next2:		; Stack; focused focus
	pop	hl
	pop	af
	ret

	;; ---------------------------------
	;; ctk_draw_widget(struct ctk_widget *w,
	;; 		unsigned char focus,
	;; 		unsigned char clipy1,
	;; 		unsigned char clipy2);
	;; Stack; retl reth wl wh focus clipy1 clipy2
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_ctk_draw_widget:
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)		; DE = widget
	inc	hl
	ld	a, (hl)		; A = focus
	inc	hl
	ld	c, (hl)		; clipy1
	inc	hl
	ld	b, (hl)		; clipy2
	push	af		; focus
	push	bc
	call	_clip_arch
	pop	bc
	push	de		; Stack; widget focus
	ld	h, d
	ld	l, e
	ld	bc, #_off_widget_window
	call	_get_offset
	ld	h, d
	ld	l, e		; HL = window
	ld	bc, #_off_window_focused
	call	_get_offset	; DE = focused
	pop	bc		; pop widget, Stack focus
	ld	a, b		; compare DE(fucosed) and BC(widget)
	sub	d
	jr	nz, _ctk_draw_widget_nofocus
	ld	a, c
	sub	e
	jr	nz, _ctk_draw_widget_nofocus
	pop	af
	or	a, #_CTK_FOCUS_WIDGET
	push	af
_ctk_draw_widget_nofocus:
	push	bc		; widget, Stack widget focus
	ld	bc, #_off_window_x
	call	_get_offset
	inc	e
	ld	c, e
	push	bc
	ld	bc, #_off_window_y
	call	_get_offset
	inc	e
	inc	e
	pop	bc
	ld	b, e		; yx
	pop	de		; widget
	pop	af		; focus
	;; FALL THROUGH

	;; ---------------------------------
	;; void draw_widget_asm(struct ctk_widget *w,
	;; 			unsigned char x, unsigned char y,
	;; 			unsigned char focus)
 	;; Stack; retl reth
	;; in; a = focus, bc = yx, de = widget
	;; ____________
	;; return void
	;; ---------------------------------
_draw_widget_asm:
	push	hl
	push	de
	push	bc
	push	af
	
	push	de
	ld	h, d
	ld	l, e
	push	af
	push	bc		; Stack; xy, focus, w
	ld	bc, #_off_widget_x
	call	_get_offset
	ld	a, e
	pop	bc		; Stack; focus w
	add	c
	ld	c, a
	push	bc
	ld	bc, #_off_widget_y
	call	_get_offset
	ld	a, e
	pop	bc		; Stack; focus, w
	add	b
	ld	b, a		; B = ypos, C = xpos
	pop	af
	and	#_CTK_FOCUS_WIDGET
	push	af
	inc	sp
	call	_revers_arch
	inc	sp		; Stack; w
	push	bc		; Stack; yx, w
	call	_gotoxy_arch
	pop	bc
	pop	hl		; Stack; null
	push	hl
	push	bc	
	ld	bc, #_off_widget_type
	call	_get_offset
	ld	a, e		; A = type
	ld	bc, #_off_widget_w
	call	_get_offset	; E = w->w;
	pop	bc		; B = ypos, C = xpos
	pop	hl		; HL = w
	cp	a, #_CTK_WIDGET_SEPARATOR
	jp	z, _draw_widget_separator
	cp	a, #_CTK_WIDGET_LABEL
	jp	z, _draw_widget_label
	cp	a, #_CTK_WIDGET_BUTTON
	jp	z, _draw_widget_button
	cp	a, #_CTK_WIDGET_HYPERLINK
	jp	z, _draw_widget_hyperlink
	cp	a, #_CTK_WIDGET_TEXTENTRY
	jp	z, _draw_widget_textentry
.if CTK_CONF_ICONS
	cp	a, #_CTK_WIDGET_ICON
	jp	z, _draw_widget_icon
.endif				;  CTK_CONF_ICONS
_draw_widget_ret:
	xor	a
	push	af
	inc	sp
	call	_revers_arch
	inc	sp

	pop	af
	pop	bc
	pop	de
	pop	hl
	ret
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_separator:
	push	de
	inc	sp
	call	_chline_arch
	inc	sp
	jr	_draw_widget_ret
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_label:
	ld	a, e
	push	bc
	push	hl
	ld	bc, #_off_widget_h
	call	_get_offset	; E = h
	ld	h, e
	ld	l, a		; H = h, L = w->w
	ex	(sp), hl	; Stack; H = h, L = w->w; HL = w
	ld	bc, #_off_widget_label_text
	call	_get_offset	; DE =  text
	pop	hl		; Stack; yx; H = h, L = w->w
	pop	bc		; BC = yx
_draw_widget_label_loop:
	ld	a, h
	or	a
	jr	z, _draw_widget_ret

	push	de
	push	hl
	push	bc
	call	_gotoxy_arch
	pop	bc
	pop	hl
	pop	de
	;; BC = ypos, xpos, H = h, L = w->w, DE = text
	;;       cputsn_arch(text, w->w);
	ld	a, l
	push	hl
	push	bc
	push	af
	inc	sp
	push	de
	call	_cputsn_arch
	pop	de
	inc	sp
	pop	bc
	pop	hl
	;;       clearto_arch(xpos + w->w);
	ld	a, c
	add	a, l
	push	hl
	push	de
	push	bc
	push	af
	inc	sp
	call	_clearto_arch
	inc	sp
	pop	bc
	pop	de
	pop	hl
	;;       ++ypos;
	inc	b
	;;       text += w->w;
	ld	a, e
	add	a, l
	ld	e, a
	ld	a, d
	adc	a, #0
	ld	d, a
	
	dec	h
	jr	_draw_widget_label_loop
	
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_button:
	push	hl		; w
	push	de		; w->w
	ld	a, #0x5b	; '['
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	pop	de
	pop	hl
	push	de
	ld	bc, #_off_widget_button_text
	call	_get_offset
	push	de
	call	_cputsn_arch
	pop	de
	pop	de
	ld	a, #0x5d	; ']'
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	jp	_draw_widget_ret
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_hyperlink:
	push	de
	ld	bc, #_off_widget_button_text
	call	_get_offset
	push	de
	call	_cputsn_arch
	pop	de
	pop	de
	jp	_draw_widget_ret
	
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_textentry:		; 0xc5dc sp=0xf97e
	;; stable variables;
	;;   w->w, w->h, w->widget.textentry.{state,ypos,xpos), xscroll
	push	ix
	ld	ix, #-6
	add	ix, sp
	ld	sp, ix
	push	bc
	
	ld	0(ix), e	; w->w
	ld	bc, #_off_widget_h
	call	_get_offset
	ld	1(ix), e	; w->h
	ld	bc, #_off_widget_textentry_state
	call	_get_offset
	ld	a, e
	sub	a, #_CTK_TEXTENTRY_EDIT
	ld	2(ix), a	; w->widget.textentry.state
	ld	bc, #_off_widget_textentry_ypos
	call	_get_offset
	ld	3(ix), e	; w->widget.textentry.ypos
	ld	bc, #_off_widget_textentry_xpos
	call	_get_offset
	ld	4(ix), e	; w->widget.textentry.xpos

	ld	bc, #_off_widget_textentry_xpos
	call	_get_offset
	ld	a, e
	inc	a
	sub	0(ix)	; w->w
	jr	nc, _draw_widget_textentry_next
	xor	a
_draw_widget_textentry_next:	; A = xscroll, Stack; yx - IX
	ld	5(ix), a	; w->widget.textentry.xscroll

	ld	bc, #_off_widget_textentry_text
	call	_get_offset	; DE = text

	ld	l, #0		; loop counter
	pop	bc
	;; L = counter, IX = sp, DE = text, BC = yx, Stack; null
_draw_widget_textentry_loop1:	; 0xc629
	;;       gotoxy_arch(xpos, ypos);
	push	de
	push	hl		; Stack; yx count text
	push	bc
	call	_gotoxy_arch

	;;       if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	;; 	    w->widget.textentry.ypos == j)
	pop	bc
	pop	hl		; Stack; text
	pop	de		; Stack; null
	push	bc		; Stack; yx
	ld	a, l
	cp	a, 1(ix)	; j - w->h
	jp	nc, _draw_widget_textentry_ret
	ld	a, 2(ix)	; state
	or	a
	jr	nz, _draw_widget_textentry_else
	ld	a, 3(ix)	; ypos
	cp	a, l
	jr	nz, _draw_widget_textentry_else
	;; 	revers_arch(0);
	push	hl		; Stack count yx
	push	de		; Stack text count yx
	xor	a
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	;; 	cputc_arch('>');
	ld	a, #0x3e	; '>'
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	
	pop	de		; Stack count yx
	push	de		; Stack text count yx
	ld	a, 5(ix)	; xscroll
	add	a, e
	ld	e, a
	ld	a, d
	adc	a, #0
	ld	d, a		; DE = text[xscroll]

	ld	h, #0		; loop counter
	;; 	for(i = 0; i < w->w; ++i)
_draw_widget_textentry_loop2:	; 0xc666 Stack text count yx
	ld	a, h
	cp	a, 0(ix)	; i - w->w
	jr	nc, _draw_widget_textentry_loop2exit
	;; 	  revers_arch(i == w->widget.textentry.xpos - xscroll);
	push	hl		; Stack count2 text count1 yx
	ld	a, 4(ix)	; w->w.widget.textentry.xpos
	sub	5(ix)		; xscroll
	sub	h		; textentry.xpos - xscroll- i
	jr	z, _draw_widget_textentry_revers1
	xor	a
	jr	_draw_widget_textentry_revers0
_draw_widget_textentry_revers1:
	ld	a, #01
_draw_widget_textentry_revers0:
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	
	ld	a, (de)		; ch
	or	a
	jr	nz, _draw_widget_textentry_nospace
	ld	a, #CH_SPACE
_draw_widget_textentry_nospace:
	push	de
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	pop	de
	pop	hl		; Stack text count1 yx
	inc	h
	inc	de
	jr	_draw_widget_textentry_loop2

_draw_widget_textentry_loop2exit:	; Stack text count yx
	xor	a
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	ld	a, #0x3c	; '<'
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	jr	_draw_widget_textentry_endif

_draw_widget_textentry_else: ; 0xc68a DE = text, L = count1, Stack yx
	;; 	cputc_arch(CH_VERTLINE);
	ld	a, #CH_VERTLINE
	push	hl
	push	de
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
	pop	de
	;; 	cputsn_arch(text, w->w);
	ld	a, 0(ix)	; w->w
	push	af
	inc	sp
	push	de
	call	_cputsn_arch
	pop	de
	inc	sp		; Stack j yx
	;; 	clearto_arch(xpos + w->w + 1);
	pop	hl
	pop	bc
	push	bc
	push	hl
	push	de		; Stack text count yx
	ld	a, 0(ix)	; w->w
	add	a, c
	inc	a		; xpos + w->w + 1
	push	af
	inc	sp
	call	_clearto_arch
	inc	sp
	;; 	cputc_arch(CH_VERTLINE);
	ld	a, #CH_VERTLINE
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp
_draw_widget_textentry_endif:	; Stack text count yx
	;; text += w->w
	pop	de
	ld	a, e
	add	a, 0(ix)
	ld	e, a
	ld	a, d
	adc	a, #0
	ld	d, a

	pop	hl
	;;	i++
	inc	l
	pop	bc
	;;       ++ypos;
	inc	b
	jp	_draw_widget_textentry_loop1
_draw_widget_textentry_ret:
	pop	bc
	ld	hl, #6
	add	hl, sp
	ld	sp, hl
	pop	ix
	jp	_draw_widget_ret	

.if CTK_CONF_ICONS
_draw_widget_icon:
	ld	bc, #_off_widget_icon_title
	call	_get_offset
	push	de
	ld	bc, #_off_widget_icon_textmap
	call	_get_offset
	push	de
	call	_drawicon_arch
	pop	de
	pop	de
	jp	_draw_widget_ret
.endif				; CTK_CONF_ICONS

.if CTK_CONF_MENUS
	;; ---------------------------------
	;; static void draw_menu(struct ctk_menu *m, struct ctk_menu *open)
	;; in; HL = menu, BC = open
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
_draw_menu_asm:	
	;;   x = wherex_arch();
	push	hl
	call	_wherex_arch	; A = x
	pop	hl
	push	af
	;;  cputs_arch(m->title);
	push	bc
	ld	bc, #_off_menu_title
	call	_get_offset	; DE = m->title
	push	hl
	push	de
	call	_cputs_arch
	pop	de		; Stack; menu open x
	;;   cputc_arch(CH_SPACE);
	ld	a, #CH_SPACE
	push	af
	inc	sp
	call	_cputc_arch
	inc	sp

	pop	hl		; menu
	pop	bc		; open, Stack; x
	ld	a, h
	sub	b
	jp	nz, _draw_menu_ret2
	ld	a, l
	sub	c
	jp	nz, _draw_menu_ret2
	;;  x2 = wherex_arch();
	;; 0xc473
	push	hl
	call	_wherex_arch	; A = x2
	ld	c, l		; C = x2
	pop	hl		; Stack; x
	;; HL = menu
	push	bc
	ld	bc, #_off_menu_active
	call	_get_offset
	pop	bc		; E = m->active
	ld	d, #0		; D = y
	pop	af		; A = x, Stack; null
	ld	b, a
	add	a, #CTK_CONF_MENUWIDTH
	sub	#SCREEN_WIDTH
	jr	c, _draw_menu_loop1
	jr	z, _draw_menu_loop1
	ld	a, #SCREEN_WIDTH
	sub	#CTK_CONF_MENUWIDTH
	ld	b, a
	;; B = x, C = x2, D = y, E = m->active, HL = menu
_draw_menu_loop1:
	;;	if (y == m->nitems) _draw_menu_ret
	ld	a, d
	push	de
	push	hl
	push	bc
	ld	bc, #_off_menu_nitems
	call	_get_offset
	cp	e
	pop	bc
	pop	hl
	jr	z, _draw_menu_ret ; leave 2byte to pop while return
	pop	de
	;;       if (y == m->active) revers_arch(0);
	cp	e
	jr	nz, _draw_menu_next1
	xor	a
	push	hl
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	pop	hl
	;; B = x, C = x2, D = y, HL = menu
_draw_menu_next1:		; c4ae
	;;       gotoxy_arch(x, y + 1);
	push	hl
	push	de
	inc	d
	ld	e, b
	push	de
	call	_gotoxy_arch
	pop	de
	pop	de		; D = y
	pop	hl		; HL = menu, Stack; null
	;;       if(m->items[y].title[0] == '-')
	push	de
	push	hl		; e7ca
	push	bc		; Stack; x2x menu y
	
	ld	bc, #_off_menu_items
	ld	a, (bc)
	add	a, l
	ld	l, a
	ld	a, h
	adc	#0
	ld	h, a		; HL = m->items[0]

	ld	bc, #_size_menuitem
	ld	a, (bc)
	ld	c, a
	ld	b, #0		; BC = sizeof(struct menuitem)
	
	ld	a, d		; A = y
_draw_menu_loop2:
	or	a
	jr	z, _draw_menu_next2
	add	hl, bc
	dec	a
	jr	_draw_menu_loop2
_draw_menu_next2:
	ld	bc, #_off_menuitem_title
	call	_get_offset
	ld	a, (de)
	cp	a, #0x2d	; '-'
	jr	nz, _draw_menu_else
	;; 	chline_arch(CTK_CONF_MENUWIDTH);
	ld	a, #CTK_CONF_MENUWIDTH
	push	af
	inc	sp
	call	_chline_arch
	inc	sp
	jr	_draw_menu_next3
_draw_menu_else:
	;; 	cputs_arch(m->items[y].title);
	push	de
	call	_cputs_arch
	pop	de
_draw_menu_next3:
	;;       clearto_arch(x + CTK_CONF_MENUWIDTH);
	pop	bc		; B = x, C = x2, Stack; menu y
	push	bc
	ld	a, b
	add	a, #CTK_CONF_MENUWIDTH
	push	af
	inc	sp
	call	_clearto_arch
	inc	sp
	;;       revers_arch(1);
	ld	a, #1
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	pop	bc
	pop	hl
	pop	de
	inc	d
	jr	_draw_menu_loop1
_draw_menu_ret:			; C = x2
	ld	b, #0
	push	bc
	call	_gotoxy_arch
	pop	bc
_draw_menu_ret2:
	pop	af
	ret	

_ctk_draw_menus:
	;; ---------------------------------
	;; void ctk_draw_menus(struct ctk_menus *menus);
 	;; Stack; retl reth menusl menush
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
	;;   clip_arch(0, SCREEN_HEIGHT);
	ld	h, #SCREEN_HEIGHT
	ld	l, #0
	push	hl
	call	_clip_arch
	;;   gotoxy_arch(0, 0);
	ld	hl, #0
	ex	(sp), hl
	call	_gotoxy_arch
	;;   revers_arch(1);
	ld	h, #1
	ex	(sp), hl
	inc	sp
	call	_revers_arch
	inc	sp		; Stack; null

	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	h, d
	ld	l, e		; HL = menus
	ld	bc, #_off_menus_desktopmenu
	call	_get_offset	
	push	de		; Stack; menus->desktopmenu
	ld	bc, #_off_menus_open
	call	_get_offset
	push	de		; Stack; menus->open menus->desktopmenu
	ld	bc, #_off_menus_menus
	call	_get_offset
	ld	h, d
	ld	l, e		; HL = menu
_ctk_draw_menus_loop1:	
	ld	bc, #_off_menu_next
	call	_get_offset
	;; if (menu == NULL) _ctk_draw_menus_next1
	ld	a, d
	or	e
	jr	z, _ctk_draw_menus_next1
	;;     draw_menu_asm(m, menus->open);
	ld	h, d
	ld	l, e		; HL = menu->next
	pop	bc
	push	bc		; Stack; menus->open menus->desktopmenu
	push	hl		; Stack; menu menus->open menus->desktopmenu
	call	_draw_menu_asm
	pop	hl		; Stack; menus->open menus->desktopmenu
	jr	_ctk_draw_menus_loop1
_ctk_draw_menus_next1:
	pop	de		; menus->open
	pop	hl		; menus->desktopmenu
	push	de		; Stack; menus->open
	;; clearto_arch(SCREEN_WIDTH - strlen(menus->desktopmenu->title) - 1);
	ld	bc, #_off_menu_title
	call	_get_offset
	ld	b ,#0
_ctk_draw_menus_loop2:
	ld	a, (de)
	or	a
	jr	z, _ctk_draw_menus_next2
	inc	b
	inc	de
	jr	_ctk_draw_menus_loop2
_ctk_draw_menus_next2:
	ld	a, #SCREEN_WIDTH
	sub	b	; strlen(menus->desktopmenu->title)
	dec	a	; SCREEN_WIDTH - strlen(menus->desktopmenu->title) - 1
	push	hl
	push	af
	inc	sp
	call	_clearto_arch
	inc	sp
	pop	hl		; desktopmenu
	pop	bc		; open, Stack; null
	call	_draw_menu_asm
	xor	a
	push	af
	inc	sp
	call	_revers_arch
	inc	sp
	ret
.endif				; CTK_CONF_MENUS

	;; ---------------------------------
	;; unsigned char ctk_draw_width(void);
 	;; Stack; retl reth
	;; _______L____
	;; return width
	;; ---------------------------------
_ctk_draw_width:
	ld	l, #SCREEN_WIDTH
	ret

	;; ---------------------------------
	;; unsigned char ctk_draw_height(void);
 	;; Stack; retl reth
	;; _______L____
	;; return width
	;; ---------------------------------
_ctk_draw_height:
	ld	l, #SCREEN_HEIGHT
	ret

_ctk_conio_arch_asm_end::
