#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::2 */
const uint8_t default_mac_address[8]  PROGMEM = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const uint8_t default_server_name[16] PROGMEM = "huginn";
const uint8_t default_domain_name[30] PROGMEM = "localhost";
uint8_t eemem_mac_address[8]  EEMEM = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
uint8_t eemem_server_name[16] EEMEM = "huginn";
uint8_t eemem_domain_name[30] EEMEM = "localhost";
