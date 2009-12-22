/*
 * \file
 *	This file contains a set of configuration for using SDCC as a compiler.
 *	Modified from z80 port for cc2430 port.
 *
 * \author
 *	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __8051_DEF_H__
#define __8051_DEF_H__

#define CC_CONF_FUNCTION_POINTER_ARGS	1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS		1
#define CC_CONF_UNSIGNED_CHAR_BUGS	0
#define CC_CONF_REGISTER_ARGS		0
#define CC_CONF_FUNCTION_POINTER_KEYWORD __reentrant

/* Generic types. */
typedef   signed char    int8_t;
typedef unsigned char   uint8_t;
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef unsigned char   u8_t;      /* 8 bit type */
typedef unsigned short u16_t;      /* 16 bit type */
typedef unsigned long  u32_t;      /* 32 bit type */
typedef   signed long  s32_t;      /* 32 bit type */
typedef unsigned short uip_stats_t;
typedef   signed long  int32_t;    /* 32 bit type */
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

/* Compiler configurations */
#define CCIF
#define CLIF
#define CC_CONF_CONST_FUNCTION_BUG

/* Critical section management */
#define DISABLE_INTERRUPTS()	EA = 0;
#define ENABLE_INTERRUPTS()		EA = 1;

#define ENTER_CRITICAL()	\
{	\
	__asm		\
	push	ACC	\
	push	IE	\
	__endasm;	\
}	\
	EA = 0;

#define EXIT_CRITICAL()	\
{	\
	__asm			\
	pop		ACC	\
	__endasm;		\
	ACC &= 0x80;		\
	IE |= ACC;		\
	__asm			\
	pop		ACC	\
	__endasm; 		\
}

/*
 * Enable architecture-depend checksum calculation
 * for uIP configuration.
 * @see uip_arch.h
 * @see uip_arch-asm.S
 */
/*
 * DO NOT USE UIP_ARCH flags!
 * uip_arch code was copied from z80 directory but NOT ported
 */

#define UIP_ARCH_ADD32		0
#define UIP_ARCH_CHKSUM	0
#define UIP_ARCH_IPCHKSUM

#define CC_CONF_ASSIGN_AGGREGATE(dest, src)	\
    memcpy(dest, src, sizeof(*dest))

#define uip_ipaddr_copy(dest, src)		\
    memcpy(dest, src, sizeof(*dest))

#endif /* __8051_DEF_H__ */
