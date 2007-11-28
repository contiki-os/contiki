;;; 
;;; 
;;; getkey.S
;;; 
;;; PC-6001 architecture-depend key fetch module
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 

	.globl _keyavail_arch
	.globl _getkey_arch
	.area _GSINIT

	.area _DATA	
	;; unsigned char key;
_key:	
	.ds 1
	
	.area	_GSINIT
	.area	_CODE

	;; ---------------------------------
	;; unsigned char keyavail_arch();
	;; AFBC__HL____
	;; a=getkey, l=getkey
	;; ---------------------------------
_keyavail_arch:	
	ld	hl, #_key
	ld	a, (hl)
	;; if (key != 0) return 1; else goto _getch;
	or	a
	jr	z, _getch
	ld	l, #0x01
	ret

_getch:
	push	hl
	;; A = (keybuf)
	call	#0x103a
	pop	hl
	;; if (keybuf != null) goto _getch_key
	jr	nz, _getch_key
	;; we do not have a keyin
	xor	a
	jr	_getch_got
_getch_key:
 	cp	a, #0x14
 	jr	z, _getch
	
_getch_got:	
	;; key = getch()
	ld	(hl), a
	ld	l, a
	ret

	
	;; ---------------------------------
	;; unsigned char getkey_arch();
	;; AFBC__HL____
	;; ---------------------------------
_getkey_arch:	
	;; if (key == 0) keyavail_arch();
	ld		hl, #_key
	ld		a, (hl)
	or		a
	call	z, _keyavail_arch
	;; if (key == 0) goto _havekey
	or		a
	jr		z, _havekey
	;; key = 0	
	ld		(hl), #0x00
_havekey:
	ld	l, a
	ret
