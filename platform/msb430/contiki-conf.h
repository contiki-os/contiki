#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define IRQ_PORT1 0x01
#define IRQ_PORT2 0x02
#define IRQ_ADC   0x03

// MSB430 SD Card driver
#define SD_CACHE 1
#define SD_READ_ANY 1
#define SD_WRITE 1

// MSP430 Infomemory
#define INFOMEM_START				0x1000
#define INFOMEM_BLOCK_SIZE			   128
#define INFOMEM_NODE_ID				0x0000 /* - 0x0004 */

#define CFS_SD_CONF_OFFSET			0x0000

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CC_CONF_INLINE inline

#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

// Clock
typedef unsigned short clock_time_t;
#define CLOCK_CONF_SECOND 100
#define F_CPU 2457600uL					// CPU target speed in Hz

#define BAUD2UBR(baud) ((F_CPU/baud))

#include "ctk/ctk-vncarch.h"

#define LOG_CONF_ENABLED 0

/**
 * The 8-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * char" works for most compilers.
 */
typedef unsigned char u8_t;

/**
 * The 16-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * short" works for most compilers.
 */
typedef unsigned short u16_t;

/**
 * The 32-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * long" works for most compilers.
 */
typedef unsigned long u32_t;

/**
 * The 32-bit signed data type.
 *
 * This may have to be tweaked for your particular compiler. "signed
 * long" works for most compilers.
 */
typedef long s32_t;

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;

/**
 * ScatterWeb code compatibilty
 * @{
 */
typedef char			bool;
typedef unsigned char	UINT8;
typedef unsigned int	UINT16;
typedef unsigned long	UINT32;
typedef signed char		INT8;
typedef signed int		INT16;
typedef signed long		INT32;

#define	true	1
#define false 	0
/** @} */

#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_BUFFER_SIZE     140
/*#define UIP_CONF_RECEIVE_WINDOW  140*/

#define UIP_CONF_LLH_LEN         6

#define UIP_CONF_RESOLV_ENTRIES  1

#define UIP_CONF_PINGADDRCONF    1

#define UIP_CONF_LOGGING         0

#define UIP_CONF_UDP_CONNS       6

#define UIP_CONF_FWCACHE_SIZE    1

#define UIP_CONF_BROADCAST       1

#define LOADER_CONF_ARCH "loader/loader-arch.h"

#define ELFLOADER_CONF_DATAMEMORY_SIZE 100
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x1000

/* LEDs ports MSB430 */
#define LEDS_PxDIR P5DIR
#define LEDS_PxOUT P5OUT
#define LEDS_CONF_RED		0x80
#define LEDS_CONF_GREEN		0x00
#define LEDS_CONF_YELLOW	0x00

#define SD_LED_READ_ON		LEDS_PxOUT &=~LEDS_CONF_RED
#define SD_LED_READ_OFF		LEDS_PxOUT |= LEDS_CONF_RED
#define SD_LED_WRITE_ON		SD_LED_READ_ON
#define SD_LED_WRITE_OFF	SD_LED_READ_OFF

#endif /* __CONTIKI_CONF_H__ */
