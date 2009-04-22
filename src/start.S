/*
 *  armboot - Startup Code for ARM720 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */
	
	.equ    I_BIT,          0x80      /* when I bit is set, IRQ is disabled */
	.equ    F_BIT,          0x40      /* when F bit is set, FIQ is disabled */
	
	.equ    USR_MODE,       0x10
	.equ    FIQ_MODE,       0x11
	.equ    IRQ_MODE,       0x12
	.equ    SVC_MODE,       0x13
	.equ    ABT_MODE,       0x17
	.equ    UND_MODE,       0x1B
	.equ    SYS_MODE,       0x1F

	.equ    usr_stack_size, 256*4
	.equ    irq_stack_size, 128*4
	.equ    fiq_stack_size, 128*4
	.equ    und_stack_size,  32*4
	.equ    abt_stack_size,  32*4
	.equ    sup_stack_size,  32*4
	
/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */	

.set base, .
.set _rom_data_init, 0x108d0
	

	.globl _start
_start:	b       _begin
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq
	
/* these vectors are used for rom patching */	
.org 0x20
.code 16
_RPTV_0_START:
	bx lr /* do nothing */

.org 0x60
_RPTV_1_START:
	bx lr /* do nothing */

.org 0xa0
_RPTV_2_START:
	bx lr /* do nothing */

.org 0xe0
_RPTV_3_START:
	bx lr /* do nothing */

.org 0x120
ROM_var_start: .word 0
.org 0x7ff
ROM_var_end:   .word 0
.code 32
.align
_begin:

 	ldr     r1,=_system_stack
 	msr     cpsr_c,#(SVC_MODE | I_BIT | F_BIT)
 	add     r1,r1,#sup_stack_size
 	mov     sp,r1

 	msr     cpsr_c,#(IRQ_MODE | I_BIT | F_BIT)
 	add     r1,r1,#irq_stack_size
 	mov     sp,r1

 	msr     cpsr_c,#(FIQ_MODE | I_BIT | F_BIT)
 	add     r1,r1,#fiq_stack_size
 	mov     sp,r1
	
 	msr     cpsr_c,#(ABT_MODE | I_BIT | F_BIT)
 	add     r1,r1,#abt_stack_size
 	mov     sp,r1

 	msr     cpsr_c,#(UND_MODE | I_BIT | F_BIT)
 	add     r1,r1,#und_stack_size
 	mov     sp,r1

// 	msr     cpsr_c,#(USR_MODE | I_BIT | F_BIT)
// 	add     r1,r1,#usr_stack_size
// 	mov     sp,r1


	bl _rom_data_init+.-base
	msr     cpsr_c,#(SVC_MODE) // turn on interrupts --- for debug only	
//	swi
	b main
	
_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq
	.balignl 16,0xdeadbeef	
	
/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

_system_stack:
	. = . + usr_stack_size + irq_stack_size + fiq_stack_size + und_stack_size + abt_stack_size + sup_stack_size

	
/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

_start_armboot: .word main


	
/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 *************************************************************************
 */

cpu_init_crit:
	# actually do nothing for now!
	mov	pc, lr


/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */


/*
 * exception handlers
 */
	.align  5
undefined_instruction:

	.align	5
software_interrupt:

	.align	5
prefetch_abort:
	nop
	.align	5
data_abort:

	.align	5
not_used:


	.align	5
irq:
	push	{lr}
	movs    lr,pc
	b isr
	pop     {lr}
	subs 	pc,r14,#4 // suggested irq return cmd
//	STMFD    sp!, {r0-r12,lr}
//	MOVNE 	 lr,pc
//	ldr 	r0, =isr
//	BX      r0
//	LDMFD    r13!, {r0-r12,r14}
//	MOVS     PC, R14
//	subs 	pc, r14, #4
fiq:

	.align	5

.globl reset_cpu
reset_cpu:
	mov     pc, r0

