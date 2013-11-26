#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_
#include <stdint.h>
#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;

typedef unsigned short uip_stats_t;

#define UIP_CONF_UIP_IP4ADDR_T_WITH_U32 1

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000

#endif /* CONTIKI_CONF_H_ */
