; 2002-11-16, Ullrich von Bassewitz
; 2016-04-28, Greg King
;
; void logscr(const void *msg, unsigned len);

        .export         _logscr

        .import         BSOUT
        .import         popax
        .importzp       ptr1, ptr2

;--------------------------------------------------------------------------

.proc   _logscr
        eor     #$FF
        sta     ptr2
        txa
        eor     #$FF
        sta     ptr2+1          ; remember -count-1

        jsr     popax           ; get buf
        sta     ptr1
        stx     ptr1+1

L1:     inc     ptr2            ; count the char that will be printed
        bne     L2
        inc     ptr2+1
        beq     L9
L2:     ldy     #$00
        lda     (ptr1),y
        jsr     BSOUT
        inc     ptr1
        bne     L1
        inc     ptr1+1
        bne     L1              ; branch always

L9:     rts
.endproc
