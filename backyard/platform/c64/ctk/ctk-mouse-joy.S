
;---------------------------------------------------------------------

        .macpack        generic

	.import		_ctk_mouse_joyx, _ctk_mouse_joyy
	.import		_ctk_mouse_firebutton	
	.export		_ctk_mouse_joy	
;---------------------------------------------------------------------
.bss
lastjoydy:	.res 1
lastjoydx:	.res 1
joydycnt:	.res 1
joydxcnt:	.res 1
	
;---------------------------------------------------------------------
.code

;---------------------------------------------------------------------
_ctk_mouse_joy:
	  ldy $dc02
	  lda #0
          sta $dc02
          lda $dc00
          sty $dc02
	  ldy #0
	  ldx #0
	  lsr
	  bcs nodey
	  dey
nodey:
	  lsr
	  bcs noiny
	  iny
noiny:
	  lsr
	  bcs nodex
	  dex
nodex:
	  lsr
	  bcs noinx
	  inx
noinx:
	  and #1
	  eor #1
	  ora _ctk_mouse_firebutton
	  sta _ctk_mouse_firebutton

	  cpy lastjoydy
	  sty lastjoydy
	  bne noydy
	  tya
	  pha
	  inc joydycnt
	  lda joydycnt
          cmp #$1f
          bmi nostajoydycnt
          lda #$20
	  sta joydycnt
nostajoydycnt:
	  lsr
	  lsr
	  lsr
	  lsr
	  tay
	  pla
asldeyloop:
	  asl
	  dey
	  bpl asldeyloop
	  tay
	  jmp ydy
noydy:
	  lda #0
	  sta joydycnt
ydy:

	  tya
	  clc
	  adc _ctk_mouse_joyy

	  cmp #240
	  bcc :+
	  lda #0
:	
		
 	  cmp #200
 	  bcc :+
 	  lda #199		
:
	  sta _ctk_mouse_joyy
	
	  cpx lastjoydx
	  stx lastjoydx
	  bne noxdx
	  txa
	  pha
	  inc joydxcnt
	  lda joydxcnt
          cmp #$1f
          bmi nostajoydxcnt
          lda #$20
	  sta joydxcnt
nostajoydxcnt:
	  lsr
	  lsr
	  lsr
	  lsr
	  tax
	  pla
asldexloop:
	  asl
	  dex
	  bpl asldexloop
	  tax
	  jmp xdx
noxdx:
	  lda #0
	  sta joydxcnt
xdx:
	                                                                                       
	  txa
	  clc
	  adc _ctk_mouse_joyx
	  sta _ctk_mouse_joyx
	  php
	  lda #$ff
	  cpx #0
	  bmi nolda02
	  lda #0
nolda02:
	  plp
	  adc _ctk_mouse_joyx+1
	  sta _ctk_mouse_joyx+1
	  rts	                                                                                       
