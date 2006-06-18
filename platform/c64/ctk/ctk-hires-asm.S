
;---------------------------------------------------------------------       
	.importzp       ptr1, ptr2, ptr3, ptr4, sreg
	.importzp	sp

	.import		incsp2

	
	.import		_ctk_hires_cursx
	.import		_ctk_hires_cursy
	.import		_ctk_hires_color
	.import		_ctk_hires_underline
	.import		_ctk_hires_reversed

	.import		_ctk_hires_yscreenaddr
	.import		_ctk_hires_yhiresaddr

	.import		_ctk_hires_windowparams
	
	.import		_ctk_hires_bitmapptr

	.import		_ctk_hires_theme
			
	.export		_ctk_hires_cclear
	.export		_ctk_hires_chline
	.export		_ctk_hires_cputc
	.export		_ctk_hires_cputsn

	.export		_ctk_hires_draw_buttonleft
	.export		_ctk_hires_draw_buttonright
	
	.export		_ctk_hires_draw_windowborders

	.export		_ctk_hires_draw_bitmapline
;---------------------------------------------------------------------

ptr5 = sreg

	;; Offsets in the theme struct

	ulcornerpattern =	8
	titlebarpattern =	ulcornerpattern+8
	urcornerpattern =	titlebarpattern+8
	rborderpattern  =	urcornerpattern+8
	lrcornerpattern =	rborderpattern+8
	lowerborderpattern =	lrcornerpattern+8
	llcornerpattern =	lowerborderpattern+8
	lborderpattern =	llcornerpattern+8

	
	buttonleftpattern =	lborderpattern+8
	buttonrightpattern =	buttonleftpattern+8

				
.bss
tmp01:	.res 1
tmpl:	.res 1
tmpf:	.res 1
tmpc:	.res 1
tmpw:	.res 1	
tmpptr:	.res 2

tmpy:	.res 1
tmph:	.res 1

	;; Window border parameters	
w:	.res 1
wpw          = 0
h:	.res 1
wph          = 1	
clipy1:	.res 1
wpclipy1     = 2
clipy2:	.res 1
wpclipy2     = 3	
color1:	.res 1
wpcolor1     = 4	
color2:	.res 1
wpcolor2     = 5	
titlelen:	.res 1
wptitlelen   = 6
title:	.res 2
wptitle      = 7	
	
;---------------------------------------------------------------------
.data
;---------------------------------------------------------------------
.code
setupptrs:	
	;; Find hires address.	
	lda _ctk_hires_cursy
	asl
	tax
	lda _ctk_hires_yhiresaddr,x
	sta ptr2
	lda _ctk_hires_yhiresaddr+1,x
	sta ptr2+1

	;; Add X coordinate to the hires address. 
	lda #0
	sta ptr1+1
	lda _ctk_hires_cursx
	asl
	rol ptr1+1
	asl	
	rol ptr1+1
	asl
	rol ptr1+1
	clc
	adc ptr2
	sta ptr2
	lda ptr2+1
	adc ptr1+1
	sta ptr2+1


	;; Find screen address. 
	lda _ctk_hires_cursy
	asl
	tax	
	lda _ctk_hires_yscreenaddr,x
	clc
	adc _ctk_hires_cursx
	sta ptr1
	lda _ctk_hires_yscreenaddr+1,x
	adc #0
	sta ptr1+1

	;; Turn off interrupts, prepare $01 to store color data in RAM
	;; under I/O area.		
	sei
	lda $01
	sta tmp01
	and #$f8
	sta $01
	rts
;---------------------------------------------------------------------	
restore01:	
	lda tmp01
	sta $01
	cli
	rts		
;---------------------------------------------------------------------
_ctk_hires_cclear:
	sta tmpl
	cmp #0
	bne @norts
	rts
@norts:		
		
	jsr setupptrs		

	;; Store color value in color memory

	ldy tmpl
	dey
	lda _ctk_hires_color
@loop0:
	sta (ptr1),y
	dey
	bpl @loop0

	;; Store either $00 or $ff in hires memory, depending on the
	;; reversed flag		
	
	ldx tmpl
	lda _ctk_hires_reversed
	beq @noff
	lda #$ff
@noff:		
	sta tmpf	
@loop1:
	ldy #7
	lda tmpf
@loop2:
	sta (ptr2),y
	dey
	bpl @loop2

	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc @noinc
	inc ptr2+1
@noinc:		

	inc _ctk_hires_cursx

	dex
	bne @loop1

	jmp restore01
;---------------------------------------------------------------------
_ctk_hires_chline:
	sta tmpl
	cmp #0
	bne @norts
	rts
@norts:	
			
	jsr setupptrs		

	;; Store color value in color memory

	ldy tmpl
	dey
	lda _ctk_hires_color
@loop0:
	sta (ptr1),y
	dey
	bpl @loop0

	;; Store horizontal line pattern in hires memory
	ldx tmpl
@loop1:
	ldy #0
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$ff
	sta (ptr2),y
	iny
	lda #$ff
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y


	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc @noinc
	inc ptr2+1
@noinc:	

	inc _ctk_hires_cursx

	dex
	bne @loop1

	jmp restore01
;---------------------------------------------------------------------	
_ctk_hires_cputc:
	sta tmpc

	jsr setupptrs

	ldy #0
	lda _ctk_hires_color
	sta (ptr1),y
	
	;; Turn on font ROM
	lda tmp01
	and #$f8
	ora #$01
	sta $01

	lda #0
	sta ptr1+1		  
	
	;; Convert from PETSCII to screen encoding
	lda tmpc
	eor #$e0
	clc
	adc #$20
	bpl @cont
	adc #$40
	bpl @cont
	eor #$a0
@cont:
	ldx _ctk_hires_reversed
	beq :+
	eor #$80
:		

	;; Convert to address in character ROM
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	sta ptr1
	lda #$d8
	adc ptr1+1
	sta ptr1+1

	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_hires_underline
	beq @nounderline
	eor #$ff
@nounderline:
	sta (ptr2),y
	dey
@loop:
	lda (ptr1),y
	sta (ptr2),y
	dey
	bpl @loop
	
	inc _ctk_hires_cursx	
	
	jmp restore01		
;---------------------------------------------------------------------
_ctk_hires_cputsn:
	sta tmpl
	cmp #0
	bne :+
	rts
:		
	
	jsr setupptrs

	ldy tmpl
	dey
	lda _ctk_hires_color
@loop0:
	sta (ptr1),y
	dey
	bpl @loop0
			
	;; Turn on font ROM
	lda tmp01
	and #$f8
	ora #$01
	sta $01

	
	;; Output each character from the string
	lda #0
	sta tmpc
	ldy #0
	lda (sp),y
	sta ptr3
	iny
	lda (sp),y
	sta ptr3+1	
	
@loop1:	
	
	lda #0
	sta ptr1+1		  
		
	ldy tmpc
	lda (ptr3),y
	beq @end
	
	;; Convert from PETSCII to screen encoding		
	eor #$e0
	clc
	adc #$20
	bpl @cont
	adc #$40
	bpl @cont
	eor #$a0
@cont:
	ldx _ctk_hires_reversed
	beq :+
	eor #$80
:		

	;; Convert to address in character ROM
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	sta ptr1
	lda #$d8
	adc ptr1+1
	sta ptr1+1

	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_hires_underline
	beq @nounderline
	eor #$ff
@nounderline:
	sta (ptr2),y
	dey
@loop:
	lda (ptr1),y
	sta (ptr2),y
	dey
	bpl @loop
	
	inc _ctk_hires_cursx

	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc :+
	inc ptr2+1
:
	inc tmpc
	lda tmpc
	cmp tmpl
	bne @loop1
	
@end:		
	jsr restore01
	
        jmp incsp2
	
;---------------------------------------------------------------------
_ctk_hires_draw_windowborders:

	jsr setupptrs

	lda _ctk_hires_windowparams + wpclipy1
	beq :+
	jmp @nofirstline
:		
	
	;; Draw first line of the border, including the window title

	;; Start with filling color memory
	ldy _ctk_hires_windowparams + wpw
	iny
	lda _ctk_hires_windowparams + wpcolor1
	sta (ptr1),y	
	dey
	dey
	dey
	sty tmpw	

	ldy _ctk_hires_windowparams + wptitlelen
	iny
	iny
	iny
:	
	sta (ptr1),y
	iny
	cpy tmpw
	bcc :-

	inc tmpw

	;; Set color for upper left corner
	ldy #0
	lda _ctk_hires_windowparams + wpcolor1
	sta (ptr1),y
		
	
	;; Turn on font ROM for drawing the title
	lda tmp01
	and #$f8
	ora #$01
	sta $01

	;; Use a temporary pointer (ptr3) rather than ptr2 so we can
	;; reuse it later.
	lda ptr2
	sta ptr3
	lda ptr2+1
	sta ptr3+1
		
	;; Draw upper left corner
	ldy #7
:	
	lda _ctk_hires_theme+ulcornerpattern,y
	sta (ptr3),y
	dey
	bpl :-

	;; Draw first empty char left of the title
	ldy #8
	lda #0
:		
; 	sta (ptr3),y		
	iny
	cpy #$10
	bne :-

	lda ptr3
	clc
	adc #$10
	sta ptr3
	bcc :+
	inc ptr3+1
:
	dec tmpw

	;;  Draw the title
	lda _ctk_hires_windowparams + wptitle
	sta ptr4
	lda _ctk_hires_windowparams + wptitle+1
	sta ptr4+1
	

	lda #0
	sta tmpc
@titleloop:
	lda #0
	sta ptr5+1

	ldy tmpc
	lda (ptr4),y

	
	;; Convert from PETSCII to screen encoding		
	eor #$e0
	clc
	adc #$20
	bpl @cont
	adc #$40
	bpl @cont
	eor #$a0
@cont:
	;; Convert to address in character ROM
	asl
	rol ptr5+1
	asl
	rol ptr5+1
	asl
	rol ptr5+1
	sta ptr5
	lda #$d8
	adc ptr5+1
	sta ptr5+1

	;; Print on hires screen, character by character, bottom-up.
	ldy #7
:
	lda (ptr5),y
; 	sta (ptr3),y		
	dey
	bpl :-
	
	lda ptr3
	clc
	adc #8
	sta ptr3
	bcc :+
	inc ptr3+1
:
	dec tmpw
	
	inc tmpc
	lda tmpc
	cmp _ctk_hires_windowparams + wptitlelen
	bne @titleloop

	;; Draw empty char after the title
	ldy #7
	lda #0
:		
; 	sta (ptr3),y
	dey	
	bpl :-

	lda ptr3
	clc
	adc #$8
	sta ptr3
	bcc :+
	inc ptr3+1
:
	dec tmpw

	;; Draw window title bar pattern
@titlebarloop:	
	ldy #7
:
	lda _ctk_hires_theme+titlebarpattern,y
	sta (ptr3),y
	dey
	bpl :-

	lda ptr3
	clc
	adc #$8
	sta ptr3
	bcc :+
	inc ptr3+1
:
	;; Loop until we are three characters away from right border
	;; to avoid flickering since we would be overwriting the close
	;; button.
	dec tmpw
	lda tmpw
	cmp #3
	bcs @titlebarloop

	lda ptr3
	clc
	adc #$18
	sta ptr3
	bcc :+
	inc ptr3+1
:		
	;; Draw upper right corner piece
	ldy #7
:
	lda _ctk_hires_theme+urcornerpattern,y
	sta (ptr3),y
	dey
	bpl :-


@nofirstline:


	ldx _ctk_hires_windowparams + wph
	inx
	stx tmph

	;; Draw left and right borders

	;; Fill color memory (pointer in ptr1, setup by setupptrs)

	;; First check how much we actually should draw by inspecting
	;; the clipy1 and clipy2 fields in the _ctk_hires_windowparams 
	;; structure		

	lda #1
	sta tmpy
	
	ldx _ctk_hires_windowparams + wpclipy1
@clipy1loop:
	cpx #0
	beq @clipy1loopend
	lda ptr1
	clc
	adc #$28
	sta ptr1
	bcc :+
	inc ptr1+1
:
	lda ptr3
	clc
	adc #$40
	sta ptr3
	lda ptr3+1
	adc #1
	sta ptr3+1

	inc tmpy
	
	dex
	jmp @clipy1loop
@clipy1loopend:	

	lda ptr1
	clc
	adc #$28
	sta ptr1
	bcc :+
	inc ptr1+1
:
		
	ldx tmpy
	ldy _ctk_hires_windowparams + wpw
	iny
	sty tmpw
@bordercloop:
	cpx _ctk_hires_windowparams + wpclipy2
	beq @bordercloopend
	
	ldy tmpw
	lda _ctk_hires_windowparams + wpcolor2
	sta (ptr1),y
	ldy #0
	lda _ctk_hires_windowparams + wpcolor1
	sta (ptr1),y

	lda ptr1
	clc
	adc #$28
	sta ptr1
	bcc :+
	inc ptr1+1
:
	
	inx
	cpx tmph
	bcc @bordercloop
	beq @bordercloop	
@bordercloopend:

	
	;; Fill color memory for lower border, if clipy2 allows
	cpx _ctk_hires_windowparams + wpclipy2
	beq @noclowerborder

	lda ptr1
	sec
	sbc #$28
	sta ptr1
	bcs :+
	dec ptr1+1
:
		
	ldy _ctk_hires_windowparams + wpw
	lda _ctk_hires_windowparams + wpcolor2		
@lbordercloop:	
	sta (ptr1),y
	dey
	bne @lbordercloop
	
@noclowerborder:	
		
	;; Draw right border
	ldx tmpy
@rborderloop:
	cpx _ctk_hires_windowparams + wpclipy2
	beq @rborderloopend

	lda ptr3
	clc
	adc #$40
	sta ptr3
	lda ptr3+1
	adc #1
	sta ptr3+1

	ldy #7
:
	lda _ctk_hires_theme+rborderpattern,y		
	sta (ptr3),y
	dey
	bpl :-

	inx
	cpx tmph
	bne @rborderloop	
@rborderloopend:
	
 	cpx _ctk_hires_windowparams + wpclipy2 
 	beq @norbordercorner
	
	lda ptr3
	clc
	adc #$40
	sta ptr3
	lda ptr3+1
	adc #1
	sta ptr3+1
	
	;; Draw lower right corner, if clipy2 allows it
	ldy #7
:		
	lda _ctk_hires_theme+lrcornerpattern,y
	sta (ptr3),y
	dey
	bpl :-

@norbordercorner:	

	stx tmpy
		
	;; Draw lower border
	ldx _ctk_hires_windowparams + wpw

@lowerborderloop:
	lda ptr3
	sec
	sbc #$8
	sta ptr3
	bcs :+
	dec ptr3+1
:	

	lda tmpy
	cmp _ctk_hires_windowparams + wpclipy2
	beq @nolowerborder
	
	ldy #7
:	
	lda _ctk_hires_theme+lowerborderpattern,y
	sta (ptr3),y
	dey
	bpl :-
@nolowerborder:	
		
	dex
	bne @lowerborderloop

	lda ptr3
	sec
	sbc #$8
	sta ptr3
	bcs :+
	dec ptr3+1
:	


	ldx tmpy
 	cpx tmph
 	bne @nolbordercorner
	
	;; Draw lower left corner, if clipy2 allows it
	ldy #7
:		
	lda _ctk_hires_theme+llcornerpattern,y
	sta (ptr3),y
	dey
	bpl :-

	lda ptr3
	sec
	sbc #$40
	sta ptr3
	lda ptr3+1
	sbc #1
	sta ptr3+1

@nolbordercorner:	
				
	;; Draw left border

	
		
@lborderloop:
	cpx #1
	beq @lborderloopend
	cpx _ctk_hires_windowparams + wpclipy1
	bcc @lborderloopend
	
	ldy #7
:
	lda _ctk_hires_theme+lborderpattern,y	
	sta (ptr3),y
	dey
	bpl :-

	lda ptr3
	sec
	sbc #$40
	sta ptr3
	lda ptr3+1
	sbc #1
	sta ptr3+1
	
	dex
	jmp @lborderloop
	
@lborderloopend:		

	
	jmp restore01
;---------------------------------------------------------------------	
_ctk_hires_draw_buttonleft:
	jsr setupptrs

	ldy #0
	lda _ctk_hires_color
	sta (ptr1),y

:		
	lda _ctk_hires_theme+buttonleftpattern,y
	sta (ptr2),y
	iny
	cpy #8
	bne :-
	
	jmp restore01
;---------------------------------------------------------------------	
_ctk_hires_draw_buttonright:
	jsr setupptrs
	ldy #0
	lda _ctk_hires_color
	sta (ptr1),y

:		
	lda _ctk_hires_theme+buttonrightpattern,y
	sta (ptr2),y
	iny
	cpy #8
	bne :-
	
	jmp restore01	
;---------------------------------------------------------------------
_ctk_hires_draw_bitmapline:
	sta tmpl
	cmp #0
	bne :+
	rts
:		

	jsr setupptrs
	
	ldy #0	
	lda _ctk_hires_color
:	
 	sta (ptr1),y
	iny
	cpy tmpl
	bne :-

	lda _ctk_hires_bitmapptr
	sta ptr1
	lda _ctk_hires_bitmapptr+1
	sta ptr1+1
	
	ldy #0
	ldx #0
:		
	lda (ptr1),y
 	sta (ptr2),y		
	iny
	inx
	cpx #8
	bne :-

	ldx #0
	
	dec tmpl
	bne :-
	
	jmp restore01	
;---------------------------------------------------------------------
	