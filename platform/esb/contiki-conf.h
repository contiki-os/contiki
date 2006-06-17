#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define IRQ_PORT1 0x01
#define IRQ_PORT2 0x02
#define IRQ_ADC   0x03

#define EEPROMFS_ADDR_NODECONF  0x0000
#define EEPROMFS_ADDR_CODERECV  0x0040
#define EEPROMFS_ADDR_CODEPROP  0x0040

#define EEPROMFS_ADDR_TESTDATA   0x0042
#define EEPROMFS_ADDR_NUMTESTDATA 0x0040

#define EEPROMFS_ADDR_SENSORLOG 0x0400
#define EEPROMFS_SIZE_SENSORLOG 0x0200

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CC_CONF_INLINE inline

#define CC_CONF_VA_ARGS                1


typedef unsigned short clock_time_t;
#define CLOCK_CONF_SECOND 8

void clock_delay(unsigned int us2);

void clock_wait(int ms10);

void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

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
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;


#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_BUFFER_SIZE     140
#define UIP_CONF_RECEIVE_WINDOW  140

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

/* LEDs ports. */
#define LEDS_PxDIR P2DIR
#define LEDS_PxOUT P2OUT
#define LEDS_CONF_RED    0x01
#define LEDS_CONF_GREEN  0x02
#define LEDS_CONF_YELLOW 0x04

#endif /* __CONTIKI_CONF_H__ */
