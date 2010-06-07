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
	
	.equ    IRQ_DISABLE,          0x80      /* when I bit is set, IRQ is disabled */
	.equ    FIQ_DISABLE,          0x40      /* when F bit is set, FIQ is disabled */
	
	.equ    USR_MODE,       0x10
	.equ    FIQ_MODE,       0x11
	.equ    IRQ_MODE,       0x12
	.equ    SVC_MODE,       0x13
	.equ    ABT_MODE,       0x17
	.equ    UND_MODE,       0x1B
	.equ    SYS_MODE,       0x1F

	.equ    usr_stack_size, 1024
	.equ    irq_stack_size,  256
	.equ    fiq_stack_size,  256
	.equ    und_stack_size,  256
	.equ    abt_stack_size,   16
	.equ    sup_stack_size,   16

	
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

#ifdef USE_ROM_VARS	
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
#endif /*USE_ROM_VARS*/

.code 32
.align
_begin:
	/* FIQ mode stack */
	msr	CPSR_c, #(FIQ_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__fiq_stack_top__	/* set the FIQ stack pointer */

	/* IRQ mode stack */
	msr	CPSR_c, #(IRQ_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__irq_stack_top__	/* set the IRQ stack pointer */

	/* Supervisor mode stack */
	msr	CPSR_c, #(SVC_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__svc_stack_top__	/* set the SVC stack pointer */

	/* Undefined mode stack */
	msr	CPSR_c, #(UND_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__und_stack_top__	/* set the UND stack pointer */

	/* Abort mode stack */
	msr	CPSR_c, #(ABT_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__abt_stack_top__	/* set the ABT stack pointer */

	/* System mode stack */
	msr	CPSR_c, #(SYS_MODE | IRQ_DISABLE | FIQ_DISABLE)
	ldr	sp, =__sys_stack_top__	/* set the SYS stack pointer */

#ifdef USE_ROM_VARS	
	ldr r12,=_rom_data_init
	mov lr,pc
	bx  r12
#endif	
	msr	CPSR_c, #(SYS_MODE)

	b main

//	ldr     r1,=_system_stack
// 	msr     cpsr_c,#(SVC_MODE | I_BIT | F_BIT)
//	add     r1,r1,#sup_stack_size
// 	mov     sp,r1

// 	msr     cpsr_c,#(IRQ_MODE | I_BIT | F_BIT)
// 	add     r1,r1,#irq_stack_size
// 	mov     sp,r1

// 	msr     cpsr_c,#(FIQ_MODE | I_BIT | F_BIT)
// 	add     r1,r1,#fiq_stack_size
// 	mov     sp,r1
	
// 	msr     cpsr_c,#(ABT_MODE | I_BIT | F_BIT)
// 	add     r1,r1,#abt_stack_size
// 	mov     sp,r1

// 	msr     cpsr_c,#(UND_MODE | I_BIT | F_BIT)
// 	add     r1,r1,#und_stack_size
// 	mov     sp,r1

// 	msr     cpsr_c,#(USR_MODE | I_BIT | F_BIT)

//	bl _rom_data_init+.-base
//	msr     cpsr_c,#(SVC_MODE) // turn on interrupts --- for debug only
//	msr     cpsr_c,#(USR_MODE) // turn on interrupts --- for debug only		
// 	add     r1,r1,#usr_stack_size
// 	mov     sp,r1
	
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

_system_stack:
	. = . + usr_stack_size + irq_stack_size + fiq_stack_size + und_stack_size + abt_stack_size + sup_stack_size

	
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
//irq:
//
//	.align	5
fiq:

	.align	5

.globl reset_cpu
reset_cpu:
	mov     pc, r0
