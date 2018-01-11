;
; Helper functions for pfs_opendir() and pfs_readdir()
;
; Close a directory listing that had been openned by pfs_opendir().
;
; 2012-06-01, Ullrich von Bassewitz
; 2016-04-10, Greg King
;

        .importzp       sp, ptr1, tmp1
        .import         pushax, _pfs_read, _pfs_close
        .export         __pfs_dirread1, __pfs_dirread

        .export         _pfs_closedir

;---------------------------------------------------------------------------
; Data structure

.struct DIR
    fd          .word
.endstruct

;---------------------------------------------------------------------------
; /* Read one byte from the directory into the supplied buffer.
; ** Return true if the read was successful, and false otherwise.
; */
; unsigned char __fastcall__ _pfs_dirread1(struct cfs_dir *dir, void *buf);

__pfs_dirread1:
        jsr     pushax          ; Push buf
        lda     #1              ; Load count = 1

; Run directly into __dirread

;---------------------------------------------------------------------------
; /* Read characters from the directory into the supplied buffer.
; ** Return true if the read was successful, and false otherwise.
; */
; unsigned char __fastcall__ _pfs_dirread(struct cfs_dir *dir, void *buf,

__pfs_dirread:
        pha                     ; Save count

; Replace dir by ((DIR*)dir)->fd.

        ldy     #$02
        lda     (sp),y
        sta     ptr1
        iny
        lda     (sp),y
        sta     ptr1+1
        ldy     #DIR::fd+1
        lda     (ptr1),y
        pha
        dey
        lda     (ptr1),y
        ldy     #$02
        sta     (sp),y
        pla
        iny
        sta     (sp),y

; Get count, save it again, clear the high byte, and call pfs_read().
; By the previous actions, the stack frame is as pfs_read() needs it;
; and, pfs_read() also will drop it.

        pla
        pha
        ldx     #>$0000
        jsr     _pfs_read

; Check for errors.

        cpx     #>-1
        beq     L3

; pfs_read() was successful; check number of bytes read. We assume that
; pfs_read() will not return more than count; so, .X is zero if we come here.

        sta     tmp1            ; Save returned count
        pla                     ; Our count
        cmp     tmp1
        beq     L2              ; OK, return count

; Didn't read enough bytes. That is an error for us.

        bne     L1              ; Branch always

; pfs_read() returned an error.

L3:     pla                     ; Drop count
        inx                     ; .X = 0
L1:     txa                     ; Return zero
L2:     rts

;---------------------------------------------------------------------------
; void __fastcall__ pfs_closedir(struct cfs_dir *dirp);

.proc   _pfs_closedir
        sta     ptr1
        stx     ptr1+1

; Load ((DIR *)dirp)->fd.

        ldy     #DIR::fd+1
        lda     (ptr1),y
        tax
        dey
        lda     (ptr1),y

; Close the directory file.

        jmp     _pfs_close
.endproc
