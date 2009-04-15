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
	bl _rom_data_init+.-base
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

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80


	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

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

	.align	5
fiq:

	.align	5

.globl reset_cpu
reset_cpu:
	mov     pc, r0
