	;;
	;;
	;;  1541 CFS implementation
	;;
	;; Architecture:
	;; 
	;; Functionality split between C64 and 1541: 1541 performs
	;; directory lookup and finding files. 1541 communicates track/sector
	;; address of file, C64 requests data to be read from track/sector.
	;;
	;;  Example: reading directory
	;;
	;;  C64:	DIR
	;;  1541:	directory entries in CFS format
	;;
	;;  Example: opening file
	;;
	;;  C64:	FILE filename
	;;  1541:	track/sector
	;;
	;;  Example: reading file
	;;
	;;  C64:	track/sector, length
	;;  1541:	length, data, track/sector
	
	
	.export _c64_dio_asm_init
	.export _c64_dio_asm_read_block
	.export _c64_dio_asm_write_block	

	.export _c64_dio_asm_track, _c64_dio_asm_sector

	.export _c64_dio_asm_ptr
	
	.importzp ptr1, ptr2
		
	;; job code $80 read, $90 write
	

ciout    = $ffa8
listen   = $ffb1
second   = $ff93
unlsn    = $ffae

nbytes   = 34
esc      = $42

blockread = 1
blockwrite = 2

errok    = 0
errerr   = 1

.data
_c64_dio_asm_track: .byte 0
_c64_dio_asm_sector: .byte 0
_c64_dio_asm_ptr: .byte 0,0			

.code
;---------------------------------------
_cfs_1541_read:
	;; First send READ command to drive
	
	lda #READ_CMD
	jsr send

	;; If drive do not respond we must download drive code
	cmp #ERR_NOT_RESPONDING
	bne notload
	
	jsr download
	lda #read
	jsr send
notload:
	;; Send file descriptor and number of bytes to read
	lda _cfs_1541_fd
	jsr send
	lda _cfs_1541_bytes
	jsr send

	lda _cfs_1541_ptr
	sta ptr1
	lda _cfs_1541_ptr + 1
	sta ptr1 + 1

	;; Read bytes from drive

	ldy #0
:	
	jsr recv
	sta (ptr1),y
	iny	
	cpy _cfs_1541_bytes
	bne -

	;; Read the number of bytes that were actually read from the file
	jsr recv

	;; Return the number of bytes read
	
	rts
;---------------------------------------	
_c64_dio_asm_read_block:		

  	lda #BLOCKREAD_CMD
         jsr send

         lda _c64_dio_asm_track
         jsr send
	 lda _c64_dio_asm_sector
         jsr send


         jsr recv
         cmp #errok
         bne readerr

	 lda _c64_dio_asm_ptr
	 sta ptr1 
	 lda _c64_dio_asm_ptr+1
	 sta ptr1+1
        ldy #0
readl:	
         jsr recv
         sta (ptr1),y
         iny
         bne readl
         clc
	 lda #0
         tax	
         rts
readerr:	
	;          sta $07c0	
	 jsr recv
	; 	 sta $07c1
         ldx #0
         rts
;---------------------------------------
_c64_dio_asm_write_block:		

 	lda #blockwrite
         jsr send

         lda _c64_dio_asm_track
         jsr send
         lda _c64_dio_asm_sector
         jsr send


	 lda _c64_dio_asm_ptr
	 sta ptr1 
	 lda _c64_dio_asm_ptr+1
	 sta ptr1+1
        ldy #0
writel:
         lda (ptr1),y	
         jsr send
         iny
         bne writel

         jsr recv
         cmp #errok
         bne writeerr
         lda #0
	 tax
         rts

writeerr:	
	;          sta $07c0
	 jsr recv
	; 	 sta $07c1
         ldx #0
         rts
;---------------------------------------
_c64_dio_asm_init:		
         sta devnr
         lda #$00
         sta drvmem
         lda #$05
         sta drvmem+1

         lda #<drive
         sta ptr1
         lda #>drive
         sta ptr1+1

       ; lda devnr
       ; ldx #<icmd
       ; ldy #>icmd
       ; jsr drvcmd
       ; jsr unlsn

mwl:	
         lda devnr
         ldx #<mwcmd
         ldy #>mwcmd
         jsr drvcmd

         ldy #0
         lda (ptr1),y
         jsr ciout
         iny
         cpy #nbytes
         bne *-8

         jsr unlsn

         lda drvmem
         clc
         adc #nbytes
         sta drvmem
         lda drvmem+1
         adc #0
         sta drvmem+1

         lda ptr1
         clc
         adc #nbytes
         sta ptr1
         tax
         lda ptr1+1
         adc #0
         sta ptr1+1
         cpx #<driveend
         sbc #>driveend
         bcc mwl

         lda devnr
         ldx #<mecmd
         ldy #>mecmd
         jsr drvcmd
         jsr unlsn

         rts
;---------------------------------------
drvcmd:	
         stx ptr2
         sty ptr2+1
         jsr listen
         lda #$6f
         jsr second

         ldy #0
         lda (ptr2),y
         sta drvcmdcmp+1
         inc ptr2
         bne *+4
         inc ptr2+1

         ldy #0
         lda (ptr2),y
         jsr ciout
         iny
drvcmdcmp:	 cpy #0
         bne *-8
         rts
;---------------------------------------
devnr:	    .byte 8
mwcmd:	    .byte 6
	.byte $4d, $2d, $57
drvmem:	   .word $0500
         .byte nbytes
         .byte 0
mecmd:	    .byte 2
	 .byte $55, $33
         .byte 0
;---------------------------------------
send:

         sta ptr2
         ldx #7
sendl:	
         lsr ptr2


         lda $dd00
         and #$df
         ora #$10
         bcc *+4
         eor #$30
         sta $dd00

         lda #$c0
         bit $dd00
         bne *-3

         lda $dd00
         and #$cf
         sta $dd00

         lda $dd00
         and #$c0
         eor #$c0
         bne *-7

         dex
         bpl sendl

	 ldx $d020
	 lda #1
	 sta $d020
	 stx $d020
		
         rts
;---------------------------------------
recv:
	 ldx $d020
	 lda #0
	 sta $d020
	 stx $d020
	
         ldx #7
recvl:	
         lda $dd00
         and #$c0
         eor #$c0
         beq *-7
         asl a

         lda $dd00
         and #$df
         ora #$10
         bcs *+4
         eor #$30
         sta $dd00
         ror ptr2

         lda #$c0
         bit $dd00
         beq *-3

         lda $dd00
         and #$cf
         sta $dd00

         dex
         bpl recvl
         lda ptr2
         rts
;---------------------------------------

;---------------------------------------
;the code residing in the drive:
;---------------------------------------
dtmp     = $46
dtmp2    = $97
dbuf     = $0300
dbufcmd  = $00
dbuftrack = $06
dbufsect = $07

retries  = 10
bretries = 2
;---------------------------------------
drive  :		
	.org $0500



         cld
         tsx
         stx dstack
         sei
         jsr ledoff

drivel:	
         jsr dload
         jsr ledoff
         cli
         jmp drivel
;---------------------------------------
ledon:	
         lda $1c00
         ora #$08
         sta $1c00
         rts
ledoff:	
         lda $1c00
         and #$f7
         sta $1c00
         rts
;---------------------------------------
drecv:	
         ldx #7
drecvl:	
         lda $1800
         bmi atn
         and #5
         beq *-7
         lsr a
         lda #2
         bcc *+4
         lda #8
         sta $1800
         ror dtmp
         lda $1800
         bmi atn
         and #5
         eor #5
         beq *-9
         lda #0
         sta $1800
         dex
         bpl drecvl
         lda dtmp
         rts
;---------------------------------------
atn:	
         ldx dstack
         txs
         cli
         rts
;---------------------------------------
dsend:	
         sta dtmp
         ldx #7
dsendl:	
         lsr dtmp
         lda #2
         bcs *+4
         lda #8
         sta $1800

         lda $1800
         bmi atn
         and #5
         eor #5
         bne *-9

         sta $1800

         lda #5
         bit $1800
         bne *-3
         dex
         bpl dsendl
         rts
;---------------------------------------
dload:
         ldy #0

         jsr drecv
         sta dcmd
	
         jsr ledon
	
         jsr drecv
         sta dbuftrack

         jsr drecv
         sta dbufsect

	 lda dcmd
	 cmp #blockwrite
	 bne dblockread
dblockwrite:
         ldy #0
drecvpl:	
         jsr drecv
         sta dbuf,y	
         iny
         bne drecvpl
	
	 jmp dputsect

dblockread:		
         jsr dgetsect

dsendpage:	
         ldy #0
dsendpl:	
         lda dbuf,y
         jsr dsend
         iny
         bne dsendpl
         rts
;---------------------------------------
dgetsect:	
       ; stx dbuftrack
       ; sty dbufsect
         ldx #retries
dgetsectl:	
         lda #$80
         sta dbufcmd

         cli
:
         lda dbufcmd
         bmi :-
         sei
         cmp #1
         beq dgsnoerr

         cpx #0
         beq dgsserr
         dex
         cpx #bretries
         bcs dgetsectl
         pha
         lda #$c0
         sta dbufcmd
         cli
:		
         lda dbufcmd
         bmi :-
         pla
         cpx #0
         bne dgetsectl

dgsserr:	
         pha
         lda #errerr
         jsr dsend
         pla
         jsr dsend
         rts
dgsnoerr:	
         lda #errok
         jsr dsend
         rts
;---------------------------------------
dputsect:	
       ; stx dbuftrack
       ; sty dbufsect
         ldx #retries
dputsectl:	
         lda #$90
         sta dbufcmd

         cli
:
         lda dbufcmd
         bmi :-
         sei
         cmp #1
         beq dpsnoerr

         cpx #0
         beq dpsserr
         dex
         cpx #bretries
         bcs dputsectl
         pha
         lda #$c0
         sta dbufcmd
         cli
:		
         lda dbufcmd
         bmi :-
         pla
         cpx #0
         bne dputsectl

dpsserr:	
         pha
         lda #errerr
         jsr dsend
         pla
         jsr dsend
         rts
dpsnoerr:	
         lda #errok
         jsr dsend
         rts
;---------------------------------------
led:	      .byte 0,0
dstack:	   .byte 0
dcmd:	     .byte 0
.RELOC
driveend:	

