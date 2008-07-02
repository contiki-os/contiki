;;; 
;;; 
;;; ctt0.S 
;;; 
;;; \file
;;;     C runtime startup routine
;;; 
;;; \author
;;;     Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: crt0.S,v 1.3 2008/07/02 07:13:22 matsutsuka Exp $
;;; 
        .module crt0
       	.globl	_main

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area   _GSINIT
	.area   _GSFINAL
 	.area	_DATA
 	.area	_DATAFINAL
	.area   _BSS
	.area   _HEAP

        .area   _CODE
init::
        ;; Clear global variables
	ld	hl, #_datastart
 	ld	bc, #_dataend
_clear_loop:
	ld	a, h
	sub	b
	jr	nz, _clear_next
	ld	a, l
	sub	c
	jr	z, _clear_exit
_clear_next:
	ld	(hl), #0
	inc	hl
	jr	_clear_loop
_clear_exit:	
        ;; Initialize global variables
        call    gsinit
	jp	_main

        .area   _GSINIT
gsinit::

        .area   _GSFINAL
        ret
	.area	_DATA
_datastart::
	.area	_DATAFINAL
_dataend::
