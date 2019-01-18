
;---------------------------------------------------------------------
	;; The 1351 mouse code was taken from the CC65 libraries,
	;; written by Ullrich von Bassewitz 
;---------------------------------------------------------------------	

        .macpack        generic

	.import		_ctk_mouse_joyx, _ctk_mouse_joyy
	.import		_ctk_mouse_firebutton
	.export		_ctk_mouse_asm_irq
	.import		_ctk_mouse_joy, _ctk_mouse_1351
;---------------------------------------------------------------------
	
;---------------------------------------------------------------------
.code

;---------------------------------------------------------------------			
_ctk_mouse_asm_irq:
	lda #0
	sta _ctk_mouse_firebutton
	
	jsr _ctk_mouse_joy
	;; jsr_ctk_mouse_1351

 ; 	lda _ctk_mouse_joybutton
 ; 	ora _ctk_mouse_1351button
 ; 	sta _ctk_mouse_firebutton
	
	  lda _ctk_mouse_joyy
	  clc
	  adc #$32
	  sta $d001
	  sta $d003	

	  lda _ctk_mouse_joyx
	  clc
	  adc #$18
	  sta $d000
	  sta $d002	
	  lda #0
	  adc _ctk_mouse_joyx+1
	  and #1
	  beq :+
	  lda #3
:		
	  sta $d010                                                                                       
	  jmp $ea31
