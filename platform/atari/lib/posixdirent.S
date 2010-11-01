; Native: Shawn Jefferson, December 2005
; POSIX: Stefan Haubenthal, April 2008

		.include  "atari.inc"
		.export	  _opendir, _readdir, _closedir
		.import	  findfreeiocb, clriocb
		.import	  __oserror, return0, __do_oserror
		.importzp ptr1, tmp1
.ifdef  DEFAULT_DEVICE
        	.import __defdev
.endif


.proc	_opendir
		sta	ptr1
		stx	ptr1+1
		jsr	findfreeiocb
		beq	@iocbok
		bne	cioerr
@iocbok:	stx	diriocb
		jsr	clriocb
		ldx	diriocb
		ldy	#0		; '.' -> "D:*.*"
		lda	(ptr1),y
		cmp	#'.'
		bne	@use_parm

; "." was given as parameter, use default device/dir

.ifdef DEFAULT_DEVICE
		; construct a "Dn:*.*" like string from the default drive
		lda	__defdev+1
		sta	dddefdev+1	; copy drive number (overwrite 2nd 'D')
		lda	#<dddefdev
		sta	ICBAL,x
		lda	#>dddefdev
		sta	ICBAH,x
		bne	@cont
.else
		lda	#<defdev
		sta	ICBAL,x
		lda	#>defdev
		sta	ICBAH,x
		bne	@cont
.endif

@use_parm:	lda	ptr1
		sta	ICBAL,x
		lda	ptr1+1
		sta	ICBAH,x

@cont:		lda	#OPEN
		sta	ICCOM,x
		lda	#OPNIN|DIRECT
		sta	ICAX1,x
		jsr	CIOV
		bmi	cioerr
		lda	#0
		sta	__oserror
		tax
		lda	diriocb
		rts
.endproc

cioerr:		sty	__oserror
		jmp	return0

.proc	_readdir
		tax
		lda	#GETREC
		sta	ICCOM,x
		lda	#<entry
		sta	ICBAL,x
		sta	ptr1
		lda	#>entry
		sta	ICBAH,x
		sta	ptr1+1
		lda	#DSCTSZ
		sta	ICBLL,x
		lda	#0
		sta	ICBLH,x
		jsr	CIOV
		bmi	cioerr
		ldy	#0		; FREE SECTORS
		lda	(ptr1),y
		cmp	#'0'
		bcs	cioerr
		dey
@next:		iny			; remove trailing spaces
		iny
		iny
		lda	(ptr1),y
		dey
		dey
		sta	(ptr1),y
		cpy	#9
		bcs	@break
		cmp	#' '
		bne	@next

@break:		lda	#'.'		; extension dot
		sta	(ptr1),y
		iny			; copy extension
		sty	tmp1
		ldy	#10
		lda	(ptr1),y
		cmp	#' '
		bne	@hasext

; no extension present: remove the trailing dot and be done
		ldy	tmp1
		dey
		bne	@done

@hasext:	jsr	copychar
		ldy	#11
		jsr	copychar
		ldy	#12
		jsr	copychar

@done:		lda	#0		; end of string
		sta	(ptr1),y
		lda	ptr1
		ldx	ptr1+1
		rts


copychar:	lda	(ptr1),y	; src=y dest=tmp1
 		ldy	tmp1
		cmp	#' '
		beq	@break
		sta	(ptr1),y
		iny
		sty	tmp1
@break:		rts
.endproc

.proc	_closedir
		tax
		lda	#CLOSE
		sta	ICCOM,x
		jsr	CIOV
		bmi	@cioerr
		ldx	#0
		stx	__oserror		; clear system specific error code
		txa
		rts
@cioerr:	jmp	__do_oserror
.endproc

		.data
.ifdef DEFAULT_DEVICE
dddefdev:	.byte	"D"
.endif
defdev:		.asciiz	"D:*.*"

		.bss
diriocb:	.res	1
entry:		.res	DSCTSZ
