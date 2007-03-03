#ifndef __6502_CONF_H__
#define __6502_CONF_H__

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned long  u32_t;
typedef unsigned short uip_stats_t;

#define CC_CONF_REGISTER_ARGS 1
#define CC_CONF_FASTCALL      __fastcall__

#define CCIF
#define CLIF

#define UIP_ARCH_ADD32  1
#define UIP_ARCH_CHKSUM 1

#endif /* __6502_CONF_H__ */
