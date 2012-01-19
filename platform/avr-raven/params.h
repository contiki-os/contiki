#ifndef __PARAMS_H__
#define __PARAMS_H__
/* PARAMETER_STORAGE =
 * 0 Hard coded, minmal program and eeprom usage.
 * 1 Stored in fixed eeprom locations, rewritten from flash if corrupt.
 *   This allows parameter changes using a hardware programmer or custom application code.
 *   Corruption test is based on channel verify so get the channel before anything else!
 * 2 Obtained from eeprom using the general settings manager and read from program flash if not present.
 *   Useful for for testing builds without wearing out eeprom memory.
 * 3 Obtained from eeprom using the settings manager and rewritten from flash if not present.
 *   This ensures all parameters are present in upper eeprom flash.
 *
 * Note the parameters in this file can be changed without forcing a complete rebuild.
 */
//#define PARAMETER_STORAGE 0
//#define PARAMETER_STORAGE 1 //+476
//#define PARAMETER_STORAGE 2 //+2045
//#define PARAMETER_STORAGE 3 //+2144

//#define CONTIKI_CONF_SETTINGS_MANAGER 1
//#define CONTIKI_CONF_RANDOM_MAC 1        //adds 78 bytes

#if CONTIKI_CONF_SETTINGS_MANAGER
#if MCU_CONF_LOW_WEAR
#define PARAMETER_STORAGE 2
#else
#define PARAMETER_STORAGE 3
#endif
#else
#if MCU_CONF_LOW_WEAR
#define PARAMETER_STORAGE 0
#else
#define PARAMETER_STORAGE 1
#endif
#endif

/* Include settings.h, then dummy out the write routines */
#include "settings.h"
#if PARAMETER_STORAGE==2
#define settings_add(...) 0
#define settings_add_uint8(...) 0
#define settings_add_uint16(...) 0
#endif

#if AVR_WEBSERVER
/* Webserver builds can set some defaults in httpd-fsdata.c via makefsdata.h */
extern uint8_t eemem_mac_address[8];
extern uint8_t eemem_server_name[16];
extern uint8_t eemem_domain_name[30];
#endif

#ifdef SERVER_NAME
#define PARAMS_SERVERNAME SERVER_NAME
#else
#define PARAMS_SERVERNAME "Raven_webserver"
#endif
#ifdef DOMAIN_NAME
#define PARAMS_DOMAINNAME DOMAIN_NAME
#else
#define PARAMS_DOMAINNAME "localhost"
#endif
#ifdef NODE_ID
#define PARAMS_NODEID NODE_ID
#else
#define PARAMS_NODEID 0
#endif
#ifdef CHANNEL_802_15_4
#define PARAMS_CHANNEL CHANNEL_802_15_4
#else
#define PARAMS_CHANNEL 26
#endif
#ifdef IEEE802154_PANID
#define PARAMS_PANID IEEE802154_PANID
#else
#define PARAMS_PANID 0xABCD
#endif
#ifdef IEEE802154_PANADDR
#define PARAMS_PANADDR IEEE802154_PANADDR
#else
#define PARAMS_PANADDR 0
#endif
#ifdef RF230_MAX_TX_POWER
#define PARAMS_TXPOWER RF230_MAX_TX_POWER
#else
#define PARAMS_TXPOWER 0
#endif
#ifdef EUI64_ADDRESS
#define PARAMS_EUI64ADDR EUI64_ADDRESS
#else
/* This form of of EUI64 mac allows full 6LoWPAN header compression from mac address */
#if UIP_CONF_LL_802154
//#define PARAMS_EUI64ADDR {0x02, 0xNN, 0xNN, 0xNN, 0xNN, 0xNN, 0xNN, 0xNN}
#define PARAMS_EUI64ADDR {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#else
//#define PARAMS_EUI64ADDR {0x02, 0xNN, 0xNN, 0xff, 0xfe, 0xNN, 0xNN, 0xNN}
#define PARAMS_EUI64ADDR {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif
/* This form of of EUI64 mac allows 16 bit 6LoWPAN header compression on multihops */
//#define PARAMS_EUI64ADDR {0x02, 0x00, 0x00, 0xff, 0xfe, 0x00, 0xNN, 0xNN}
#endif

uint8_t params_get_eui64(uint8_t *eui64);
#if PARAMETER_STORAGE==0
/* Hard coded program flash parameters */
#define params_get_servername(...) 
#define params_get_nodeid(...) PARAMS_NODEID
#define params_get_channel(...) PARAMS_CHANNEL
#define params_get_panid(...) PARAMS_PANID
#define params_get_panaddr(...) PARAMS_PANADDR
#define params_get_txpower(...) PARAMS_TXPOWER
#else
/* Parameters stored in eeprom */
uint16_t params_get_nodeid(void);
uint8_t params_get_channel(void);
uint16_t params_get_panid(void);
uint16_t params_get_panaddr(void);
uint8_t params_get_txpower(void);
#endif

#endif /* __PARAMS_H__ */
