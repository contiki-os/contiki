/*
 * \file
 *	This file contains a set of configuration for using SDCC as a compiler.
 *	This is based on the cc2430 file (which in turn is based on the z80 one)
 *
 * \author
 *	 Takahide Matsutsuka <markn@markn.org> (Original)
 *	 George Oikonomou - <oikonomou@users.sourceforge.net>
 *	   (updates for the cc2530 ports)
 */

#ifndef __8051_DEF_H__
#define __8051_DEF_H__

#include <stdint.h>

/* In watchdog mode, our WDT can't be stopped once started
 * Include watchdog_stop()'s declaration and then trash it */
#include "dev/watchdog.h"
#define watchdog_stop() watchdog_periodic()

/* This port no longer implements the legacy clock_delay. Hack its usages
 * outta the way till it gets phased out completely
 * NB: This also overwrites the prototype so delay_usec() is declared twice */
#define clock_delay(t) clock_delay_usec(t)

/*
 * lint - style defines to help syntax parsers with sdcc-specific 8051 code
 * They don't interfere with actual compilation
 */
#if !defined(__SDCC_mcs51) && !defined(SDCC_mcs51)
#define __data
#define __xdata
#define __code
#define __bit bool
#define __sfr volatile unsigned char
#define __sbit volatile bool
#define __critical
#define __at(x)
#define __using(x)
#define __interrupt(x)
#define __naked
#endif

#define CC_CONF_FUNCTION_POINTER_ARGS	1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS		1
#define CC_CONF_UNSIGNED_CHAR_BUGS	0
#define CC_CONF_REGISTER_ARGS		0
#define CC_CONF_FUNCTION_POINTER_KEYWORD __reentrant
#define CC_CONF_NON_BANKED_OPTIMIZATION 1

#if (defined(__SDCC_mcs51) || defined(SDCC_mcs51)) && CC_CONF_NON_BANKED_OPTIMIZATION
#define CC_NON_BANKED __nonbanked
#else
#define CC_NON_BANKED
#endif

/* Generic types. */
typedef unsigned short uip_stats_t;

/* Time type. */
typedef unsigned short clock_time_t;
#define MAX_TICKS (~((clock_time_t)0) / 2)
/* Defines tick counts for a second. */
#define CLOCK_CONF_SECOND   128

/* Compiler configurations */
#define CCIF
#define CLIF

/* Single asm instruction without messing up syntax highlighting */
#if defined(__SDCC_mcs51) || defined(SDCC_mcs51)
#define ASM(x) __asm \
  x \
  __endasm
#else
#define ASM(x)
#endif

/* Critical section management */
#define DISABLE_INTERRUPTS()  do {EA = 0;} while(0)
#define ENABLE_INTERRUPTS()   do {EA = 1;} while(0)

/* Macro for a soft reset. */
#define SOFT_RESET() do {((void (__code *) (void)) 0x0000) ();} while(0)

/* We don't provide architecture-specific checksum calculations */
#define UIP_ARCH_ADD32    0
#define UIP_ARCH_CHKSUM	  0

#define CC_CONF_ASSIGN_AGGREGATE(dest, src)	\
    memcpy(dest, src, sizeof(*dest))

#define uip_ipaddr_copy(dest, src)		\
    memcpy(dest, src, sizeof(*dest))

#endif /* __8051_DEF_H__ */
