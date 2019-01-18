;;; 
;;; 
;;; clock.S
;;; 
;;; retrieve tickcount from PC-6001's internal timer
;;; 
;;; @author Takahide Matsutsuka <markn@markn.org>
;;; 
;;; $Id: clock.S,v 1.1 2007/09/11 12:12:59 matsutsuka Exp $
;;; 
	.globl _clock_time

	.area _GSINIT
	;; ---------------------------------
	;; unsigned long clock_time();
	;; __BCDEHL____
	;; return long value as (DE, BC)
	;; ---------------------------------
	;; this function uses PC-6001's internal timer
	;; which counts up 2 per 1/512 sec.
	.area _CODE
_clock_time:
	ld	hl, #0xfa28
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	inc	hl
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ld	l, c
	ld	h, b
	
	ret
