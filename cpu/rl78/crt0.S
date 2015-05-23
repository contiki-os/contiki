/* Copyright (c) 2011 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met: 

     Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse 
     or promote products derived from this software without specific 
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */

#include "rl78-sys.h"

	.section ".vec","a"
	.short	_start

	.section ".ivec","a"
	.macro	_iv	x
	.weak	\x
	.short	\x
	.endm
#define IV(x) _iv _##x##_handler
#define IVx() .short 0

	/* To use a vector, simply define a global function named foo_handler()
	   for any IV(foo) listed below (i.e. tm05_handler) */

	.global	_interrupt_vector_table
_interrupt_vector_table:
	IV(wdti)
	IV(lvi)
	IV(p0)
	IV(p1)
	IV(p2)
	IV(p3)
	IV(p4)
	IV(p5)

	IV(st2)
	IV(sr2)
	IV(sre2)
	IV(dma0)
	IV(dma1)
	IV(st0)
	IV(sr0)
	IV(tm01h)

	IV(st1)
	IV(sr1)
	IV(sre1)
	IV(iica0)
	IV(tm00)
	IV(tm01)
	IV(tm02)
	IV(tm03)

	IV(ad)
	IV(rtc)
	IV(it)
	IV(kr)
	IV(st3)
	IV(sr3)
	IV(tm13)
	IV(tm04)

	IV(tm05)
	IV(tm06)
	IV(tm07)
	IV(p6)
	IV(p7)
	IV(p8)
	IV(p9)
	IV(p10)

	IV(p11)
	IV(tm10)
	IV(tm12)
	IV(sre3)
	IV(tm13h)
	IV(md)
	IV(iica1)
	IV(fl)

	IV(dma2)
	IV(dma3)
	IV(tm14)
	IV(tm15)
	IV(tm16)
	IV(tm17)
	IVx()
	IVx()

	IVx()
	IVx()
	IVx()
	IVx()
	IVx()
	IV(brk)
	/* Note: 126 vectors */

	.section ".csstart", "ax"
	.global __csstart
__csstart:	
	br	!!_start

	.weak	__rl78_option_byte_0
	.weak	__rl78_option_byte_1
	.weak	__rl78_option_byte_2
	.weak	__rl78_option_byte_3
__rl78_option_byte_0 = 0x6e
__rl78_option_byte_1 = 0xff
__rl78_option_byte_2 = 0xe8
__rl78_option_byte_3 = 0x85

	.section ".opt", "a"
	.byte	__rl78_option_byte_0
	.byte	__rl78_option_byte_1
	.byte	__rl78_option_byte_2
	.byte	__rl78_option_byte_3

	.text

	.global _start
	.type	_start, @function
_start:
	movw	sp, #__stack


;; block move to initialize .data

	;; we're copying from 00:[_romdatastart] to 0F:[_datastart]
	;; and our data is not in the mirrored area.
	mov	es, #0

	sel	rb0		; bank 0
	movw	hl, #__datastart
	movw	de, #__romdatastart
	sel	rb1		; bank 1
	movw	ax, #__romdatacopysize
	shrw	ax,1
1:
	cmpw	ax, #0
	bz	$1f
	decw	ax
	sel	rb0		; bank 0
	movw	ax, es:[de]
	movw	[hl], ax
	incw	de
	incw	de
	incw	hl
	incw	hl
	sel	rb1
	br	$1b
1:
	sel	rb0		; bank 0


;; block fill to .bss

	sel	rb0		; bank 0
	movw	hl, #__bssstart
	movw	ax, #0
	sel	rb1		; bank 1
	movw	ax, #__bsssize
	shrw	ax,1
1:
	cmpw	ax, #0
	bz	$1f
	decw	ax
	sel	rb0		; bank 0
	movw	[hl], ax
	incw	hl
	incw	hl
	sel	rb1
	br	$1b
1:
	sel	rb0		; bank 0



;	call	!!__rl78_init

#ifdef PROFILE_SUPPORT	/* Defined in gcrt0.S.  */
	movw	ax, # _start
	push	ax
	movw	ax, # _etext
	push	ax
	call	!!__monstartup
#endif

	movw	ax, #0
	push	ax	/* envp */
	push	ax	/* argv */
	push	ax	/* argc */
	call	!!_main
.LFE2:
	
        movw	ax, r8       ; Save return code.
	push	ax

#ifdef PROFILE_SUPPORT
	call	!!__mcleanup
#endif

	call	!!_exit

	.size	_start, . - _start

	.global	_rl78_run_preinit_array
	.type	_rl78_run_preinit_array,@function
_rl78_run_preinit_array:
	movw	hl, #__preinit_array_start
	movw	de, #__preinit_array_end
	movw	bc, #-2
	br	$_rl78_run_inilist

	.global	_rl78_run_init_array
	.type	_rl78_run_init_array,@function
_rl78_run_init_array:
	movw	hl, #__init_array_start
	movw	de, #__init_array_end
	movw	bc, #2
	br	$_rl78_run_inilist

	.global	_rl78_run_fini_array
	.type	_rl78_run_fini_array,@function
_rl78_run_fini_array:
	movw	hl, #__fini_array_start
	movw	de, #__fini_array_end
	movw	bc, #-2
	/* fall through */

	;; HL = start of list
	;; DE = end of list
	;; BC = step direction (+2 or -2)
_rl78_run_inilist:
next_inilist:
	movw	ax, hl
	cmpw	ax, de
	bz	$done_inilist
	movw	ax, [hl]
	cmpw	ax, #-1
	bz	$skip_inilist
	cmpw	ax, #0
	bz	$skip_inilist
	push	ax
	push	bc
	push	de
	push	hl
	call	ax
	pop	hl
	pop	de
	pop	bc
	pop	ax
skip_inilist:
	movw	ax, hl
	addw	ax, bc
	movw	hl, ax
	br	$next_inilist
done_inilist:
	ret

	.section	.init,"ax"

	.global __rl78_init
__rl78_init:

	.section	.fini,"ax"

	.global __rl78_fini
__rl78_fini:
	call	!!_rl78_run_fini_array

	.section .data
	.global ___dso_handle
	.weak   ___dso_handle
___dso_handle:
	.long	0

;;; Provide Dwarf unwinding information that will help GDB stop
;;; backtraces at the right place.  This is stolen from assembly
;;; code generated by GCC with -dA.
	.section	.debug_frame,"",@progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0	; Length of Common Information Entry
.LSCIE0:
	.4byte	0xffffffff	; CIE Identifier Tag
	.byte	0x1	; CIE Version
	.ascii "\0"	; CIE Augmentation
	.uleb128 0x1	; CIE Code Alignment Factor
	.sleb128 -1	; CIE Data Alignment Factor
	.byte	0xd	; CIE RA Column
	.byte	0xc	; DW_CFA_def_cfa
	.uleb128 0xc
	.uleb128 0x3
	.byte	0x8d	; DW_CFA_offset, column 0xd
	.uleb128 0x3
	.p2align 2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0	; FDE Length
.LASFDE0:
	.4byte	.Lframe0	; FDE CIE offset
	.4byte	_start		; FDE initial location
	.4byte	.LFE2 - _start	; FDE address range
	.byte   0xf     	; DW_CFA_def_cfa_expression
	.uleb128 1		; length of expression
	.byte 	0x30		; DW_OP_lit0
	.p2align 2
.LEFDE0:

	.text
