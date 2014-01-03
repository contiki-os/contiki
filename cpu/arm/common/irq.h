

#ifndef _ARM_IRQ_H
#define _ARM_IRQ_H

#include "core_cmFunc.h"

// TODO : Disable it and use EFM32 INT_Disable() instead (adding lock counter)
#define irq_enable()   __enable_irq()
#define irq_disable()  __disable_irq()

static inline unsigned long irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, cpsr	@ irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}


static inline void irq_restore(unsigned long flags)
{
	asm volatile(
		"	msr	cpsr_c, %0	@ irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

#endif
