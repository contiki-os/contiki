#ifndef __CONTIKI_CONF_H__CDBB4VIH3I__
#define __CONTIKI_CONF_H__CDBB4VIH3I__

#include <stdint.h>
#include "platform-conf.h"
#include "lpc1768.h"

#define CCIF
#define CLIF

#define WITH_UIP 1
#define WITH_ASCII 1

#define CLOCK_CONF_SECOND 100

//UART used for sending printf messages
//Defined as 0 for UART0 and 1 for UART1
#define DEBUG_UART      0

/* Define the MAC address of the device */
/* 0C 1D 12 E0 1F 10*/
#define EMAC_ADDR0              0x10
#define EMAC_ADDR1              0x1F
#define EMAC_ADDR2              0xE0
#define EMAC_ADDR3              0x12
#define EMAC_ADDR4              0x1D
#define EMAC_ADDR5              0x0C

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#ifndef BV
#define BV(x) (1<<(x))
#endif

/* uIP configuration */
//Ethernet LLH(Link Level Header) size is 14 bytes
#define UIP_CONF_LLH_LEN         14
#define UIP_CONF_BROADCAST       1
#define UIP_CONF_LOGGING 1
#define UIP_CONF_BUFFER_SIZE 1024
#define UIP_CONF_TCP_FORWARD 1
#define UIP_CONF_ICMP6 1

/* Prefix for relocation sections in ELF files */
#define REL_SECT_PREFIX ".rel"

#define CC_BYTE_ALIGNED __attribute__ ((packed, aligned(1)))

#define USB_EP1_SIZE 64
#define USB_EP2_SIZE 64

#define RAND_MAX 0x7fff
#endif /* __CONTIKI_CONF_H__CDBB4VIH3I__ */
