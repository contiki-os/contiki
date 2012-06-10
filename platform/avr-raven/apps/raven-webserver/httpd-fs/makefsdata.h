#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::11:22ff:fe33:4455 */
const uint8_t default_mac_address[8]  PROGMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
const uint8_t default_server_name[16] PROGMEM = "Contiki-Raven";
const uint8_t default_domain_name[30] PROGMEM = "localhost";
uint8_t eemem_mac_address[8]  EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
uint8_t eemem_server_name[16] EEMEM = "Contiki-Raven";
uint8_t eemem_domain_name[30] EEMEM = "localhost";

