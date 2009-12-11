;;; 
;;; 
;;; ctk-conio_arch-asm.hS
;;; 
;;; Architecture depend ctk-conio implementation.
;;; This assembler source contributes saving speed and memory.
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: ctk-conio_arch-asm.cS,v 1.3 2009/12/11 15:00:38 matsutsuka Exp $
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
.if CTK_CONF_MENUS
	.globl	_ctk_draw_menus
.endif
	.globl	_ctk_draw_width
	.globl	_ctk_draw_height

	;; import symbols
	.globl	_wherex_arch
	.globl	_wherey_arch
	
	.globl	_cputc_arch_asm
	.globl	_cputs_arch_asm
	.globl	_cputsn_arch_asm
	.globl	_clip_arch_asm
	.globl	_clearto_arch_asm
	.globl	_revers_arch_asm
	.globl	_chline_arch_asm
	.globl	_cvline_arch_asm
	.globl	_gotoxy_arch_asm
		
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
.if CTK_CONF_HYPERLINK
	.globl	_off_widget_hyperlink_text
.endif
.if CTK_CONF_ICONS
	.globl	_off_widget_icon_title
	.globl	_off_widget_icon_textmap
.endif
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
.endif

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
	xor	a
	call	_revers_arch_asm
	ld	h, #SCREEN_HEIGHT
	ld	l, #0
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
	ex	de, hl
_ctk_draw_clear_asm:
	call	_clip_arch_asm
	ld	a, h
	sub	l			; A = clipy2 - clipy1
	ld	b, a			; height
	ld	c, #SCREEN_WIDTH	; width
	ld	d, l		; y
	ld	e, #0		; x
	call	_clearbox_arch_asm
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
	ex	de, hl
	call	_clip_arch_asm
	;;   clearbox_arch(window->x + 1, window->y + 2, window->w, window->h);
	ld	hl, #2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	h, d
	ld	l, e		; HL = window

	ld	bc, #_off_window_h
	call	_get_offset
	ld	d, e		; D = window->h
	ld	bc, #_off_window_w
	call	_get_offset	; E = window->w
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
	pop	bc		; B = h, C = w
	call	_clearbox_arch_asm
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
	ex	de, hl
	call	_clip_arch_asm
	
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
	;;  drawbox_asm(window->x, window->y + 1, window->w, window->h);
	push	af		; focus
	ld	h, d
	ld	l, e		; HL = window
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
	ld	d, a		; D = window->y + 1, E = window->x
	pop	bc		; B = h, C = w, HL = window
	call	_drawbox_asm
	inc	d		; D = y = window->y + 2
	inc	e		; E = x = window->x + 1
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
	ld	h, b
	ld	l, c	
	call	_clip_arch_asm
	push	de		; Stack; widget focus
	ex	de, hl		; HL = widget
	ld	bc, #_off_widget_window
	call	_get_offset
	ex	de, hl		; HL = window
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
	;; static void draw_widget_asm(struct ctk_widget *w,
	;; 			unsigned char x, unsigned char y,
	;; 			unsigned char focus)
 	;; Stack; retl reth
	;; in; A = focus, BC = yx, DE = widget
	;; ____________
	;; return void
	;; ---------------------------------
_draw_widget_asm:
	push	hl
	push	de
	push	bc
	push	af
	
	push	de		; Stack; w
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
	and	a, #_CTK_FOCUS_WIDGET
	call	_revers_arch_asm
	ld	h, b
	ld	l, c
	call	_gotoxy_arch_asm
	pop	hl		; Stack; null, HL = w
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
.if CTK_CONF_HYPERLINK
	cp	a, #_CTK_WIDGET_HYPERLINK
	jp	z, _draw_widget_hyperlink
.endif
	cp	a, #_CTK_WIDGET_TEXTENTRY
	jp	z, _draw_widget_textentry
.if CTK_CONF_ICONS
	cp	a, #_CTK_WIDGET_ICON
	jp	z, _draw_widget_icon
.endif
_draw_widget_ret:
	xor	a
	call	_revers_arch_asm

	pop	af
	pop	bc
	pop	de
	pop	hl
	ret
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_separator:
	ld	b, e
	call	_chline_arch_asm
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

	;; BC = ypos, xpos, H = h, L = w->w, DE = text
	push	hl		; Stack; hw
	ld	h, b
	ld	l, c
	call	_gotoxy_arch_asm
	pop	hl
	;;       cputsn_arch(text, w->w);
	push	hl
	push	bc
	push	de		; Stack; text yx hw
	ld	b, l
	call	_cputsn_arch_asm ; DE = text
	pop	de
	pop	bc
	pop	hl		; Stack; null
	;;       clearto_arch(xpos + w->w);
	ld	a, c
	add	a, l
	push	hl
	push	de
	push	bc
	call	_clearto_arch_asm
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
	call	_cputc_arch_asm
	pop	de
	pop	hl
	ld	a, e
	ld	bc, #_off_widget_button_text
	call	_get_offset
	ld	b, a
	call	_cputsn_arch_asm
	ld	a, #0x5d	; ']'
	call	_cputc_arch_asm
	jp	_draw_widget_ret
.if CTK_CONF_HYPERLINK
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_hyperlink:
	ld	a, e
	ld	bc, #_off_widget_hyperlink_text
	call	_get_offset
	ld	b, a
	call	_cputsn_arch_asm
	jp	_draw_widget_ret
.endif
	
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_textentry:		; 0xc5dc sp=0xf97e ;widget=e3be
	;; e3cd;textentry.xpos<-b224
	;; stable variables;
	;;   w->w, w->h, w->widget.textentry.{state,ypos,xpos), xscroll
	push	ix
	ld	ix, #-6
	add	ix, sp
	ld	sp, ix
	push	bc		; yx
	
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
	
	;;     xscroll = 0;
	;;     if(w->widget.textentry.xpos >= w->w - 1) {
	;;       xscroll = w->widget.textentry.xpos - w->w + 1;
	;;     }
	ld	a, e
	inc	a
	sub	0(ix)	; xscroll = w->widget.textentry.xpos - w->w + 1
	jr	nc, _draw_widget_textentry_next
	xor	a		;  if (xscroll < 0) xscroll = 0
_draw_widget_textentry_next:	; A = xscroll, Stack; yx - IX
	ld	5(ix), a	; xscroll

	ld	bc, #_off_widget_textentry_text
	call	_get_offset	; DE = text

	ld	l, #0		; loop counter
	pop	bc
	;; L = counter, IX = sp, DE = text, BC = yx, Stack; null
_draw_widget_textentry_loop1:	; 0xc629
	;;       gotoxy_arch(xpos, ypos);
	push	hl		; Stack; text
	ld	h, b
	ld	l, c
	call	_gotoxy_arch_asm

	;;       if(w->widget.textentry.state == CTK_TEXTENTRY_EDIT &&
	;; 	    w->widget.textentry.ypos == j)
	pop	hl		; Stack; null, BC = yx, DE = text, L = count
	push	bc		; Stack; yx
	ld	a, l
	cp	a, 1(ix)	; j - w->h
	jp	nc, _draw_widget_textentry_ret
	ld	a, 2(ix)	; state
	or	a		; state == EDIT?
	jr	nz, _draw_widget_textentry_else
	ld	a, 3(ix)	; ypos
	cp	a, l		; ypos == j?
	jr	nz, _draw_widget_textentry_else
	;; 	revers_arch(0);
	push	hl		; Stack count yx
	push	de		; Stack text count yx
	xor	a
	call	_revers_arch_asm
	;; 	cputc_arch('>');
	ld	a, #0x3e	; '>'
	call	_cputc_arch_asm
	
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
_draw_widget_textentry_loop2:	; 0xc39d Stack text count yx
	;; H = count2, DE = text[xscroll + i]
	ld	a, h
	cp	a, 0(ix)	; i - w->w
	jr	nc, _draw_widget_textentry_loop2exit
	;; 	  revers_arch(i == w->widget.textentry.xpos - xscroll);
	push	hl		; Stack count2 text count1 yx
	ld	a, 4(ix)	; w->w.widget.textentry.xpos
	sub	5(ix)		; xscroll
	sub	h		; textentry.xpos - xscroll- i
	ld	a, #00		; flags do not change
	jr	nz, _draw_widget_textentry_revers
	ld	a, #01
_draw_widget_textentry_revers:
	call	_revers_arch_asm
	;;  DE = text, Stack; count2 text count1 yx
	;; 	  	  cputc((c == 0) ? CH_SPACE : c);
	ld	a, (de)		; ch
	or	a
	jr	nz, _draw_widget_textentry_nospace
	ld	a, #CH_SPACE
_draw_widget_textentry_nospace:
	push	de
	call	_cputc_arch_asm
	pop	de
	pop	hl		; Stack text count1 yx, H = count2
	inc	h
	inc	de
	jr	_draw_widget_textentry_loop2
_draw_widget_textentry_loop2exit:	; Stack text count yx
	xor	a
	call	_revers_arch_asm
	ld	a, #0x3c	; '<'
	call	_cputc_arch_asm
	jr	_draw_widget_textentry_endif

_draw_widget_textentry_else: ; 0xc68a DE = text, L = count1, Stack yx
	;; 	cputc_arch(CH_VERTLINE);
	ld	a, #CH_VERTLINE
	push	hl
	push	de
	call	_cputc_arch_asm
	pop	de
	;; 	cputsn_arch(text, w->w);
	ld	b, 0(ix)	; w->w
	push	de
	call	_cputsn_arch_asm
	pop	de		; Stack count1 yx
	;; 	clearto_arch(xpos + w->w + 1);
	pop	hl
	pop	bc
	push	bc
	push	hl
	push	de		; Stack text count yx
	ld	a, 0(ix)	; w->w
	add	a, c
	inc	a		; xpos + w->w + 1
	call	_clearto_arch_asm
	;; 	cputc_arch(CH_VERTLINE);
	ld	a, #CH_VERTLINE
	call	_cputc_arch_asm
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
	;; BC = ypos, xpos, HL = w, E = w->w, Stack; null
_draw_widget_icon:
	push	bc		; Stack; yx
	ld	bc, #_off_widget_icon_title
	call	_get_offset
	push	de		; Stack; title yx
	ld	bc, #_off_widget_icon_textmap
	call	_get_offset
	pop	hl		; HL = title, DE = textmap
	pop	bc		; BC = yx

	;; BC = yx, DE = textmap, HL = title
	push	hl		; Stack; title
	ld	h, b
	ld	l, c		; HL = yx
	ld	a, e
	or	d
	jr	z, _draw_widget_icon_title
	
	;; 	push	hl
	;; 	call	_wherex_arch
	;; 	ld	c, l
	;; 	call	_wherey_arch
	;; 	ld	b, l
	;; 	pop	hl

	ld	a, #3		; loop counter
_draw_widget_icon_loop:		; HL = yx, DE = textmap, Stack; title
	call	_gotoxy_arch_asm
	
	push	af		; Stack; loop title
	push	hl		; DE = textmap, Stack; yx loop title

	ld	b, #3
	call	_cputsn_arch_asm	; DE = DE + 3
	
	pop	hl
	pop	af		; HL = yx, A = count, Stack; title
	
	inc	h		; y++
	dec	a
	jr	nz, _draw_widget_icon_loop
	;; Stack; title, HL = yx
_draw_widget_icon_title:
	pop	de		; Stack; null, HL = yx, DE = title
	push	de		; Stack; title, HL = yx, DE = title
	ld	b, h
	ld	c, l		; BC = yx
	ex	de, hl		; BC = yx, HL = title
	ld	d, #0
_draw_widget_icon_title_strlen:
	ld	a, (hl)
	or	a
	jr	z, _draw_widget_icon_title_setpos
	inc	hl
	inc	d
	jr	_draw_widget_icon_title_strlen
	;; D = len, BC = yx, Stack; title
_draw_widget_icon_title_setpos:
	ld	h, d		; H = len
	ld	a, d
	add	a, c		; x + len
	cp	a, #SCREEN_WIDTH ;  x + len - SCREEN_WIDTH
	jr	c, _draw_widget_icon_title_show
	ld	a, #SCREEN_WIDTH
	sub	h
	ld	c, a
	;; B = y, C = x; H = len, Stack; title
_draw_widget_icon_title_show:
	ld	a, h		; A = len
	ld	h, b
	ld	l, c
	call	_gotoxy_arch_asm
	pop	de		; DE = title
	ld	b, a		; B = len
	call	_cputsn_arch_asm
	jp	_draw_widget_ret

.endif
	
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
	push	hl		; Stack; menu open x
	call	_cputs_arch_asm
	;;   cputc_arch(CH_SPACE);
	ld	a, #CH_SPACE
	call	_cputc_arch_asm

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
	call	_revers_arch_asm
	pop	hl
	;; B = x, C = x2, D = y, HL = menu
_draw_menu_next1:		; c4ae
	;;       gotoxy_arch(x, y + 1);
	push	hl
	ld	h, d
	ld	l, b
	inc	h
	call	_gotoxy_arch_asm
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
	ld	b, #CTK_CONF_MENUWIDTH
	call	_chline_arch_asm
	jr	_draw_menu_next3
_draw_menu_else:
	;; 	cputs_arch(m->items[y].title);
	call	_cputs_arch_asm
_draw_menu_next3:
	;;       clearto_arch(x + CTK_CONF_MENUWIDTH);
	pop	bc		; B = x, C = x2, Stack; menu y
	push	bc
	ld	a, b
	add	a, #CTK_CONF_MENUWIDTH
	call	_clearto_arch_asm
	;;       revers_arch(1);
	ld	a, #1
	call	_revers_arch_asm
	pop	bc
	pop	hl
	pop	de
	inc	d
	jr	_draw_menu_loop1
_draw_menu_ret:			; C = x2
	ld	h, #0
	ld	l, c
	call	_gotoxy_arch_asm
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
	call	_clip_arch_asm
	;;   gotoxy_arch(0, 0);
	ld	h, #0
	call	_gotoxy_arch_asm
	;;   revers_arch(1);
	ld	a, #1
	call	_revers_arch_asm

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
	call	_clearto_arch_asm
	pop	hl		; desktopmenu
	pop	bc		; open, Stack; null
	call	_draw_menu_asm
	xor	a
	call	_revers_arch_asm
	ret
.endif

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

	
	;; ---------------------------------
	;; internal functions
	;; ---------------------------------
	
	;; ---------------------------------
	;; static void clearbox_arch_asm(unsigned char x, y, w, h)
	;; Stack; retl reth
	;; AFBCDEHL____
	;; return void
	;; ---------------------------------
	;; DE = yx, B = h, C = w
_clearbox_arch_asm:
	ld	a, e
	add	a, c
	ld	c, a		; to_x = x + w
	ld	h, d
	ld	l, e
_clearbox_arch_loop::
	call	_gotoxy_arch_asm	; (x, y)
	push	hl
	push	bc		; B = h, HL = yx
	ld	a, c
	call	_clearto_arch_asm
	pop	bc
	pop	hl
	inc	h		; y++
	djnz	_clearbox_arch_loop
	ret

	;; ---------------------------------
	;; void drawbox_asm(unsigned char x, y, w, h)
	;; Stack; retl reth
	;; B = h, C = w, D = y, E = x
	;; AF__________
	;; return void
	;; ---------------------------------
_drawbox_asm:
	push	bc
	push	de
	push	hl
	
	ld	h, d
	ld	l, e
	inc	h
	call	_gotoxy_arch_asm	; (x, y + 1)
	dec	h
	push	hl
	push	bc		; Stack; hw, yx, B = h
	call	_cvline_arch_asm
	ld	a, #CH_LLCORNER
	call	_cputc_arch_asm
	pop	bc
	push	bc		; Stack; hw yx
	ld	b, c		; w
	call	_chline_arch_asm
	ld	a, #CH_LRCORNER
	call	_cputc_arch_asm
	pop	bc
	pop	hl
	call	_gotoxy_arch_asm	; (x, y)
	push	hl
	push	bc		; Stack; hw yx
	ld	a, #CH_ULCORNER
	call	_cputc_arch_asm
	pop	bc
	push	bc		; Stack; hw yx
	ld	b, c		; B = w
	call	_chline_arch_asm
	ld	a, #CH_URCORNER
	call	_cputc_arch_asm
	pop	bc		; B = h
	pop	hl
	ld	a, l
	inc	a
	add	a, c
	ld	l, a		; L = x + 1 + w
	inc	h		; H = y + 1
	call	_gotoxy_arch_asm
	call	_cvline_arch_asm
	
	pop	hl
	pop	de
	pop	bc
	ret

_ctk_conio_arch_asm_end::
