	.import __curunit
       	.constructor    saveunit, 32
.code
saveunit:
     	lda	$ba
     	sta	__curunit
     	rts
